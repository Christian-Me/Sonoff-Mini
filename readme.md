# homie firmware for Sonoff mini

## introduction

- small firmware using the ESP8266 homie lib. 
- should be WD reset save by storing the relay state in rtc memory
- restore the last state after power off via broker.
- no controller necessary
- use the `homie/sonoff-mini-01/relay/state/set` topic to switch the state remotely
- relay is toggled by the devices pushbutton and by an external switch connected to S1 and S2 on the terminals. 

## how to flash without soldering

The Sonoff mini can be flashed directly over the air. Due to the 1MB flash memory of the ESP8265 limitation this has to be done in a 2 step process

If found a very straight forward way (simpler as in most youtube tutorials). No manual resets or power cycles necessary.

**No need to first update the sonoff firmware! Don't use the sonoff app!**

Follow these steps carefully. You have only one shot. Even if there are many steps I can be done in less than two minutes.

- **Disconnect from mains!** Open the case and set the jumper to enable DIY mode, close the cover and reconnect power.
- Enable a hotspot with the SSID `sonoffDIY` and password `20170618sn` as mentioned in the doc. Best use the windows build in mobile hotspot if you have Wifi in your machine. *(You should be able to use your phone or reconfigure your AP/router but then you have to make sure that your computer and the sonoff is in the same subnet)* 
- Then use the DIY-Tool (v1.2.0) to "flash firmware". Use the `config.bin` firmware (in folder firmware) **DO NOT use firmware.bin in this step!**.
- after automatic reboot you should see the soft AP `Homie-xxxxx`. Connect to this one. A browser window will pop up but due to space restrictions no web interface is present.
- edit `config-example.json` enter your credentials and save it as `config.json`. Check that the baseId (followed by a slash) and unique deviceId is as you like it.
- upload your `config.json` with `curl.exe -X PUT http://192.168.123.1/config --header "Content-Type: application/json" -d "@config.json"` form the same folder where the JSON sits. The Device will reboot and you will be able to see it showing up on your mqtt broker.
- build your firmware (or use the pre build `firmware.bin`)
- flash the firmware over mqtt `python ota_updater.py -l IP_OF_YOUR_BROKER -t "homie/" -i "YOUR_DEVICE_ID" ../../.pio/build/sonoff_mini/firmware.bin` from ./scripts/ota_updater. Change `IP_OF_YOUR_BROKER` to the ip of your broker and `YOUR_DEVICE_ID` as defined in the config.json. [Detailed info how to install and use theota_updater](http://github.com/homieiot/homie-esp8266/tree/develop/scripts/ota_updater)
- you can now remove the jumper as it is not needed anymore. **But please disconnect from mains power first!**
- enjoy!

From now on you can update your devices OTA via mqtt. I use a sonoff s20 for testing an debugging as this one has the same GPIO pins (except the switch input) and nice jumper terminals. You can write a script to update all you devices in one go.

----

<a href="https://homieiot.github.io/">
  <img src="https://homieiot.github.io/img/works-with-homie.png" alt="works with MQTT Homie">
</a>

more information how to use the Homie for ESP8266 library [can by found here](homieiot.github.io/homie-esp8266/docs/stable/)

A big *Thank You* goes to the folks on [Homie/ESP8266 on Gitter](https://gitter.im/homie-iot/ESP8266#) for their support.