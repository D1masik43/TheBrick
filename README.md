# TheBrick

A phone. A real one, but built from scratch — custom PCB, ESP32-S3 brain, and firmware written from the ground up because off-the-shelf phones don't let you solder a fix at 1am when the touch controller decides to stop responding.

No Android tax. No update nagging. Just a stack of ICs on a board I laid out myself, running a UI I wrote myself, doing exactly what I want it to do.

## Why

Because I wanted a phone that's mine in the way a laptop you build yourself is yours. Every screen, every driver, every weird workaround for a strapping pin that doesn't like booting without a modem attached — all of it traceable back to a decision I made. When something breaks, I don't file a ticket, I pull out the schematic.

## What's inside

- **ESP32-S3** (16MB flash, PSRAM, running plain Arduino framework via PlatformIO)
- **2.4" ILI9341 TFT** over SPI, driven with TFT_eSPI
- **FT6336 capacitive touch** for the UI
- **MCP23017** I/O expander handling the physical button cluster (D-pad + action buttons)
- **SIM800L** for actual cellular — calls and SMS over UART
- **Camera module** for photos, JPEG decode/display on-device
- **eMMC storage** for files, chat logs, and captured photos
- **INA219** power monitor + **DS3231 RTC** so the thing knows its battery and the time even after a reset
- **I2S audio** out, because a phone should be able to make noise

## The software side

Everything above lives behind a tiny in-house app framework — static apps for system-level screens (menu, quick settings, wifi) and non-static apps for anything that needs its own lifecycle (phone, SMS, camera, files, XMPP). Swipe down from the top for quick settings, tap into the app menu for everything else. It's not trying to be iOS. It's trying to be legible — I want to be able to open any source file six months from now and remember exactly why it does what it does.

## A tour, in pictures

### Quick Settings
Swipe-down panel — brightness, volume, WiFi/Bluetooth/DND toggles, flashlight. The stuff you touch ten times a day shouldn't need three taps to reach.

<img width="3000" height="4000" alt="20260819_175910" src="https://github.com/user-attachments/assets/498fbeb6-73f9-4d3e-a2e1-feedbebb3544" />
<img width="3000" height="4000" alt="20260819_180255" src="https://github.com/user-attachments/assets/81e1553a-8bdf-40b6-86f1-61168110262f" />


### App Menu
The home base. Scrollable grid of everything installed — phone, SMS, camera, files, XMPP, wifi settings, system info, and whatever I've bolted on since.

<img width="1920" height="2560" alt="photo_2026-08-19_18-10-59" src="https://github.com/user-attachments/assets/b62a5dcb-d8c0-45bf-b027-b719a0dc71d0" />

### Camera
Point, shoot, decode the JPEG right there on the TFT. Photos land straight on the eMMC.

<img width="3000" height="4000" alt="20260819_175910" src="https://github.com/user-attachments/assets/1200e542-cd51-4213-9159-9ce2a5f71444" />


### Dial
The part that makes it an actual phone. SIM800L under the hood, dialing and call handling over UART like it's 2003 and that's a compliment.

<img width="1920" height="2560" alt="image" src="https://github.com/user-attachments/assets/a9a0eefd-862d-4161-a640-6e1757086ca4" />

### XMPP
A real chat client, TLS and all (mbedTLS handling the handshake), with contacts and chat history persisted to the SD card so a reboot doesn't wipe your conversations.

<img width="3000" height="4000" alt="20260819_180338" src="https://github.com/user-attachments/assets/ee8e8009-3162-4bc2-aa3e-a70254fb6fc9" />


### Files
Browse what's actually sitting on the SD, currently Pictures only are displayed.

<img width="3000" height="4000" alt="20260819_180504" src="https://github.com/user-attachments/assets/447c9338-6ddb-4bc1-a6e5-97c2adf291d6" />
<img width="3000" height="4000" alt="20260819_180516" src="https://github.com/user-attachments/assets/a1538ed1-787f-4d0c-817c-051861f48da3" />

## Status

Actively getting built and debugged in parallel with the hardware — there's a hand-soldered prototype that works, and a newer PCB revision that's still shaking out a couple of I2C gremlins. Firmware moves fast because the hardware has to keep up with it. PCB is in final stage of development.
