# Sonoff-Alexa
Alternate firmware for the ITEAD-Sonoff based on the Arduino port of ESP8266 source for a ITEAD-Sonoff to connect directly with Alexa from Amazon. This source is a replacement for the ESP8266 based Sonoff devices.
Use it as a starting block for customizing your Sonoff.

## What's a "Sonoff"?
Sonoff is just a small ESP8266 based module, that can toggle mains power on his output. Everything is included in a nice plastic package.
See more here [Sonoff manufacturer website](https://www.itead.cc/sonoff-wifi-wireless-switch.html)

## What can your code do for me?
The goal of this is to give you a kickstart. If you use it as a starter package it should be easy for advanced making your Sonoff behave the way you want. Adding, removing features should now be trivial.

Features include (~~strikethrough~~ = not implemented yet)
- wifi credentials configuration/onboarding using [WiFiManager](https://github.com/tzapu/WiFiManager)
- web configuration portal to setup servers, ports, hostname and invokationname
- OTA over the air firmware update
- ~~Blynk integration~~
- turn off/on, relay from onboard button, by short button press
- medium button press (> four sec.)- reset
- long button press (> eight sec.)- reset the WiFi settings and restart
- it's only possible to use the timer function at the webfrontend
- Integration of a new function called toggle.
  This function is required for a remote control, see project Dashbutton. See more here [Sonoff-Alexa](https://github.com/thdillinger/Sonoff-Dashbutton)

## Getting started
First of all you will need to solder a 4 or 5 pin header on your Sonoff so you can flash the new firmware.

You will need to download any libraries included, they should all have URLs in the source code mentioned. If not you can find them in the Arduino Library Manager.

After you have the header, the libraries installed and the serial to usb dongle is ready to use, power up the module while pressing the onboard button. This should put it into programming mode.

Flash the firmware, the module should reset afterwards and the green LED should blink.
Slow blink = connecting
Fast blink = configuration portal started

If it has never been configured before, connect to the Access Point of the module to created and configure it. If you don't get a configuration popup while connecting, open URL 192.168.4.1 in your browser.
After it's configured and connected, the green LED and the relay should stay off (this is the default).

Now OTA should also be enabled and you can do future updates over the air.

Further information can be found at my website [Sonoff Hack für Alexa von Amazon](http://dillinger-engineering.de/sonoff-switch-mit-webfrontend-und-alexa-amazon/2017/08/).
