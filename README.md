# Sonoff-Alexa
Alternate firmware for the ITEAD-Sonoff based on the Arduino port of ESP8266ESP8266 source for a ITEAD-Sonoff to connect directly with Alexa from Amazon. This source is a replacement for the ESP8266 based Sonoff devices.
Use it as a starting block for customizing your Sonoff.

## What's a "Sonoff"?
Sonoff is just a small ESP8266 based module, that can toggle mains power on it's output. It has everything included in a nice plastic package.
See more here [Sonoff manufacturer website](https://www.itead.cc/sonoff-wifi-wireless-switch.html)

## What can your code do for me?
The goal of this is to give you a kickstart. Using it as a starter package you should be fairly advanced on the way of making your Sonoff behave the way you want. Adding, removing features should now be trivial.

Features include (~~strikethrough~~ = not implemented yet)
- wifi credentials configuration/onboarding using [WiFiManager](https://github.com/tzapu/WiFiManager)
- web configuration portal to setup tokes, servers, ports, hostnam and invokatio name
- OTA over the air firmware update
- ~~Blynk integration~~
- turn off and on relay from onboard button, by short button press.
- medium button press (> four sec.)- reset
- long button press (> eight sec.)- reset the WiFi settings and restart
- turn off/on and timer set from a webfrontend 
- Integration of a new function in the web frontend called toggle.
  This function is required for a remote control, see Project Dashbutton. See more here [Sonoff-Alexa](https://github.com/thdillinger/Sonoff-Dashbutton)

## Getting started
First of all you will need to solder a 4 or 5 pin header on your Sonoff so you can flash the new firmware.

You will need to download any libraries included, they should all have URLs in the source code mentioned, or you can find them in the Arduino Library Manager.

After you have the header, the libraries installed and a serial to usb dongle ready, power up the module while pressing the onboard button. This should put it into programming mode.

Flash the firmware, the module should reset afterwards and the green LED should be blinking.
Slow blink = connecting
Fast blink = configuration portal started

Being your first run, connect to the Access Point the module created and configure it. If you don t get a configuration popup when connecting, open 192.168.4.1 in your browser.

After it's configured and connected, the green LED should stay off, and the relay should be disabled (this is the default).

OTA should also be enabled now and you can do future updates over the air.

Further information can be found at our website [Sonoff Hack für Alexa von Amazon](http://dillinger-engineering.de/sonoff-switch-mit-webfrontend-und-alexa-amazon/2017/08/).
