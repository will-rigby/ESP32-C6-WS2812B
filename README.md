ESP32-C6 WS2812b Example
========================

I was working on a display project with the ESP32-C6 WROOM module to drive 4x 256 WS2812b LED display panels, they would poll the NOAA API for getting the aurora K-index, as well as letting me know when it is bin collection and recycling collection days etc. and displaying symbols on the display panels. The project was working fine on the ESP8266 dev board but when I started implementing it on the ESP32-C6 modules I'd bought a whole bunch of for some Zigbee projects, the Adafruit NeoPixel library and FastLED library aren't yet compatable with this microcontroller. I took a look at the code used by the ESP32-C6 development board to drive the single onboard WS2812b status LED and adapted it to handle more LEDs.

My application runs a HTTP server for setting LEDs and polls some APIs for setting others, if you try to run more than about 30 NeoPixels (I'm running 1024), you'll start trigger a watchdog reset, so this application runs the LED handler task on the second core. In this example code I've shown how a global variable can be used to have the LED task decided whether to display or not display data.
