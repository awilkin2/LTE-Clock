# Overview
This project is a portable clock with LTE capability. Using this LTE capability, the device gathers and displays time, timezone, date, weather, and location on an e-paper display. Using a 600mAh LiPo battery, a battery life of several days is achieved.

## Hardware
The device uses a BRN404X Particle Boron LTE development board as the main driver and a V2 Waveshare 2.13in e-paper HAT as a display. It is powered with a 600mAh LiPo connected using a standard 2mm JST connector. The BRN404X and the e-paper display are connected via SPI with connections as shown in Table 1.

Table 1: Pin Connections
|BRN404X|Display|
|-|-|
|3.3V|VCC|
|GND|GND|
|D12|DIN|
|D13|CLK|
|D3|CS|
|D2|DC|
|D1|RST|
|D0|BUSY|

## Software

This project uses the Particle DeviceOS system firmware designed specifically for Particle devices. This firmware is similar in usage to the Arduino system firmware, with the addition of methods to control features such as cellular data. In addition to the Particle-included firmware, this project uses the Google Maps Device Locator libraray (https://github.com/particle-iot/google-maps-device-locator) designed for Particle devices.
All cellular connection in this project is routed to the Particle Cloud, where custom webhooks (shown in src/webhooks) request data from APIs. Location data is gathered from the Google Maps API, and weather and timezone data is gathered from weatherapi.com's API.

## Acknowledgements
Fonts were created using https://github.com/ayoy/fontedit