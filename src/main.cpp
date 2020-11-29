#include <Homie.h>
#include <rtcmem.h>

const int PIN_RELAY = 12;
const int PIN_LED = 13;
const int PIN_BUTTON = 0;
const int PIN_SWITCH = 4;
const int PIN_JUMPER = 16;

#define FIRMWARE_NAME "sonoff-mini"
#define FIRMWARE_VERSION "0.1.3"

#define DEBOUNCE_TIME 15
#define LED_PERM_VALUE 1000   // standby dimmed value
#define LED_DIM_STEP 500      // micros pro step > 0-1000 * 50us total 50ms

unsigned long lastMillis = 0;
bool buttonPressed=false;
int buttonState = 0;
int switchState = 0;
bool relayState = false;
int ledValue = 0;
unsigned long ledNextUpdate = 0;
uint8_t rtcResult = 0;
bool rtcResultPublished = false;

HomieNode relayNode("relay", "Relay", "relay");
HomieNode inputsNode("inputs", "Physical Inputs", "GPIO inputs");

rtcmem rtcMem;
rtcBufferType rtcBuffer;

void triggerLED(int value = 0){
  ledValue = value;
  ledNextUpdate = micros()+LED_DIM_STEP;
  analogWrite(PIN_LED,ledValue);
}

void updateLED() {
  if (ledValue<LED_PERM_VALUE) {
    if (micros()>ledNextUpdate) {
      ++ledValue;
      ledNextUpdate = micros()+LED_DIM_STEP;
      analogWrite(PIN_LED,ledValue);  
    }
  }
}

void homieParameterSet(const __FlashStringHelper *nodeId, const __FlashStringHelper *parameterId, bool state) {
  String topic = "";
  topic = Homie.getConfiguration().mqtt.baseTopic;
  topic+=Homie.getConfiguration().deviceId;
  topic+="/";
  topic+=nodeId;
  topic+="/";
  topic+=parameterId;
  topic+="/set";
  Homie.getMqttClient().publish(topic.c_str(),2,true,(state ? "true" : "false"));
}

bool resetHandler(const HomieRange& range, const String& value) {
  ESP.reset();
  return true;
}

bool relayHandler(const HomieRange& range, const String& value) {
  triggerLED();
  if (value != "true" && value != "false") return false;

  relayState = (value == "true");
  digitalWrite(PIN_RELAY, relayState ? HIGH : LOW);
  relayNode.setProperty("state").send(value);

  // store relay state to RTC to be restored after reboot
  rtcBuffer.bytes.b[0]=relayState ? LOW : HIGH; // inverted to default to off
  rtcMem.write32(1, rtcBuffer.dw);
  Serial.println(rtcBuffer.bytes.b[0],HEX);
  rtcBuffer.dw = (uint32_t) rtcMem.read32(1);
  Serial.println(rtcBuffer.bytes.b[0],HEX);
  return true;
}

void loopHandler() {

    if (!rtcResultPublished) {
      String topic = Homie.getConfiguration().mqtt.baseTopic;
      topic+=Homie.getConfiguration().deviceId;
      topic+="/$stats/rtcResult";
      String result = "";
      result += rtcResult;
      Homie.getMqttClient().publish(topic.c_str(),0,false,result.c_str());
      rtcResultPublished=true;
    }

    updateLED();
    int gpioValue = 0;

    if ((millis()-lastMillis) > DEBOUNCE_TIME) {
      // handle trigger of pushbutton
      buttonState = !digitalRead(PIN_BUTTON);
      if ((buttonState == HIGH) != buttonPressed) {
        triggerLED();
        if (buttonState == HIGH && !buttonPressed) {
          digitalWrite(PIN_RELAY, relayState ? LOW : HIGH); // don`t wait for the /set message to ping back (low latency) 
          inputsNode.setProperty("button").send("true");
          buttonPressed = true;

          // sent the /set parmater to trigger the relay the correct way
          homieParameterSet(F("relay"),F("state"),!relayState);
          lastMillis = millis();
        } else if (buttonState == LOW && buttonPressed) {
          inputsNode.setProperty("button").send("false");
          buttonPressed = false;
        }
      }
      // handle on/off switch
      gpioValue = digitalRead(PIN_SWITCH);
      if (switchState != gpioValue) {
          switchState = gpioValue;
          digitalWrite(PIN_RELAY, relayState ? LOW : HIGH); // don`t wait for the /set message to ping back (low latency) 
          inputsNode.setProperty("switch").send(gpioValue == HIGH ? "true" : "false");
          // sent the /set parmater to trigger the relay the correct way
          homieParameterSet(F("relay"),F("state"),!relayState);
          lastMillis = millis();
      }
    }
}

void setup() {
  // before doing anything else try to restore relay state from RTC
  pinMode(PIN_RELAY, OUTPUT);
  rtcBuffer.dw = (uint32_t) rtcMem.read32(1);
  rtcResult = rtcBuffer.bytes.b[0];
  if (rtcBuffer.bytes.b[0]<2) {
    digitalWrite(PIN_RELAY, 1-rtcBuffer.bytes.b[0]); // inverted to default to off
  }

  Serial.begin(115200);
  Serial << endl << endl;
  Serial.println(rtcResult,HEX);

  Homie_setFirmware(FIRMWARE_NAME, FIRMWARE_VERSION);
  Homie.setLoopFunction(loopHandler);
  Homie.setLedPin(PIN_LED, LOW).setResetTrigger(PIN_BUTTON, LOW, 5000);

  relayNode.advertise("state").setName("relay state").setDatatype("boolean").settable(relayHandler);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  inputsNode.advertise("button").setName("button state")
                                .setDatatype("boolean")
                                .setRetained(false);
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  inputsNode.advertise("switch").setName("switch state")
                                .setDatatype("boolean")
                                .setRetained(true);

  Homie.setup();
}

void loop() {
  Homie.loop();
}