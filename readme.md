# homie firmware for Sonoff mini

## introduction

- small firmware using the ESP8266 homie lib. 
- should be WD reset save by storing the relay state in rtc memory
- restore the last state after power off.
- no controller necessary
- use the `homie/sonoff-mini-01/relay/state/set` topic to switch the state

## how to flash without soldering

The Sonoff mini can be flashed directly over the air. Due to the 1MB flash memory of the ESP8265 limitation this is a 2 step process

If found a very straight forward way (simpler as in most youtube tutorials)

**No need to first update the sonoff firmware! Don't use the sonoff app!** 

- set the jumper to enable DIY mode
- Enable a hotspot with the SSID `sonoffDIY` and password `20170618sn` as mentioned in the doc. Best use the windows build in mobile hotspot if you have Wifi in your machine or to use your phone.
- Then use the DIY-Tool (v1.2.0) to "flash firmware". Use the `config.bin` firmware (in folder firmware) **NOT firmware.bin in this step!**.
- after reboot you should see the soft AP `Homie-xxxxx`. Connect to this one. A browser window will pop up but due to space restrictions no web interface is present.
- edit `config-example.json` enter your credentials and save it as `config.json`.
- upload your `config.json` with `curl.exe -X PUT http://192.168.123.1/config --header "Content-Type: application/json" -d "@config.json"` form the same folder where the JSON sits
- build your firmware (or use the pre build `firmware.bin`)
- flash the firmware over mqtt `python ota_updater.py -l IP_OF_YOUR_BROKER -t "homie/" -i "sonoff-mini-02" ../../.pio/build/sonoff_mini/firmware.bin` from ./scripts/ota_updater. Change `IP_OF_YOUR_BROKER` to the ip of your broker. (Detailed info in the readme.md there)
- enjoy!


<a href="https://homieiot.github.io/">
  <img src="https://homieiot.github.io/img/works-with-homie.png" alt="works with MQTT Homie">
</a>