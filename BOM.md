# TheBrick — Bill of Materials
!!AI slop

## 1. Compute

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **ESP32-S3 N16R8 CAM dev board** (with OV2640 / OV5640) | 1 | The brain. QIO/OPI PSRAM enabled, `huge_app.csv` partition, USB-CDC on boot. Wi-Fi handles the XMPP client; the camera rides along on the same board. | ✅ |
| **OV2640 / OV5640 camera module** | 1 | Shipped with the S3-CAM board. Firmware uses RGB565 + QVGA. SCCB is on GPIO 4/5 — **a separate bus from the main I2C**. | ✅ |

## 2. Display & input

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **2.8" IPS ILI9341 + FT6336 capacitive touch, 320×240, SPI** | 1 | Driven by TFT_eSPI at 75 MHz SPI; touch via `htcw_ft6336` on I2C `0x38`. Display is portrait 240×320, rotation 0, `invertDisplay(true)`. | ✅ |
| **MCP23017 I2C 16-bit I/O expander module** | 1 | I2C `0x20`. Carries all buttons (INPUT_PULLUP), plus the amp shutdown and the LED gate. | ✅ |
| **4 × 4 × 1.5 mm SMD tactile switches** | 9 | Back, Home, Enter, Key1, Key2, Up, Down, Left, Right — all to MCP23017 pins, active-low. | ✅ |
| **3-pin SMD thumbwheel toggle switch** | 1 | Power / mode slider. Not read by firmware — hard power path. | 🔌 |

## 3. Power

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **TP5000 USB-C charge board**, 4.2 V / 3.6 V selectable, 1 A | 1 | Strapped for **4.2 V (Li-ion)** — the status bar computes percentage as `V / 4.2`. | 🔌 |
| **DC-DC buck-boost, 1.8–5.5 V in → 3.3 V out, 3 A**, low ripple | 1 | Main rail. Buck-boost matters: a 1S cell sags under the SIM800L's transmit bursts and a plain buck would brown out. [AliExpress](https://www.aliexpress.com/item/1005008428919779.html) | 🔌 |
| **INA219 current / power monitor breakout** | 1 | I2C `0x40`. Feeds the battery icon, the sysinfo app, and the `INA` / `CURR` commands on the USB-CDC channel. | ✅ |
| **IRLML6402 P-channel MOSFET** (SOT-23) | 1 | High-side switch for the LED, gated from MCP23017 pin 14. | ✅ |
| **USB Type-C breakout board** (female, with header) | 1 | Charge / data entry point. [AliExpress](https://www.aliexpress.com/item/1005009011843269.html) | 🔌 |
| **1S Li-ion / LiPo cell** | 1 | ⚠️ Not in the order list below — whatever cell was on hand. Size it to the 67 × 120 mm outline. | 🔌 |

## 4. Audio

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **MAX98357A I2S class-D amplifier, 3 W** | 1 | BCLK 42, LRCK 41, DIN 2 (`ESP32-audioI2S`). `SD` is held low at boot by MCP23017 pin 15 and raised only while the player app runs, so the amp is muted the rest of the time. | ✅ |
| **1511 cavity speaker, 8 Ω 1 W** | 1 | Shared output. Note the SIM800L also has its own speaker driver — on this build the call audio path and the music path both end at one speaker. | 🔌 |
| **Electret condenser mic capsule (2-pin)** | 1 | Wired to the SIM800L `MIC±` differential input for calls. Firmware never reads it; the modem does all the audio. | 🔌 |

## 5. Radio & time

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **SIM800L quad-band GSM/GPRS module** (micro-SIM core board) | 1 | UART1 at 9600 baud, **RX = GPIO 0, TX = GPIO 1**. Firmware sends `AT+CPIN`, `AT+CLVL`, polls `AT+CSQ` / `AT+CREG?` for the status bar, and runs calls + SMS. | ✅ |
| **DS3231SN precision RTC module** | 1 | I2C `0x68`. Auto-sets from build time on `lostPower()`. Also see `setTime.sh`. | ✅ |
| **GY-521 / MPU-6050 6-axis IMU** | 1 | ❓ **Bought but unused.** Nothing in the firmware opens it, and its default address `0x68` collides with the DS3231 anyway. Treat as a leftover. | ❓ |

## 6. Storage

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **eMMC / SD card** (SD_MMC, 1-bit) | 1 | CLK 39, CMD 38, D0 40. Holds music, photos, and the XMPP contact/chat cache. Exposed to a host over USB-MS. | ✅ |

## 7. Connectors & wiring

| Part | Qty | Notes | |
|---|:--:|---|:--:|
| **JST PH 2.0 mm, 2-pin, male + female housings** | ~4 | Battery and speaker pigtails. | 🔌 |
| **FFC / FPC flat ribbon cable, 0.5 mm & 1.0 mm pitch** | few | Camera and inter-board runs. Exact pin count depends on which modules you mount — check the footprints in the gerbers before ordering. | 🔌 |

---

## Pin map (from firmware, verbatim)

### I2C bus — `SDA = 45`, `SCL = 48`, 400 kHz, guarded by `i2cMutex`

| Address | Device |
|:--:|---|
| `0x20` | MCP23017 I/O expander |
| `0x38` | FT6336 touch controller |
| `0x40` | INA219 power monitor |
| `0x68` | DS3231 RTC |

### Direct ESP32-S3 GPIO

| Pins | Function |
|---|---|
| 21 MOSI, 47 SCLK, 3 CS, 14 DC, RST = board reset | ILI9341 display (SPI, 75 MHz) |
| 0 RX, 1 TX | SIM800L UART1 @ 9600 |
| 42 BCLK, 41 LRCK, 2 DIN | MAX98357A I2S |
| 39 CLK, 38 CMD, 40 D0 | eMMC / SD_MMC, 1-bit |
| 15 XCLK, 4 SDA, 5 SCL, 11/9/8/10/12/18/17/16 D0–D7, 6 VSYNC, 7 HREF, 13 PCLK | Camera (PWDN and RESET unused) |

### MCP23017 pins

| Pin | Function |
|:--:|---|
| 0 | Back |
| 1 | Home |
| 2 | Enter |
| 3 | Key1 |
| 4 | Key2 |
| 8 | Up |
| 9 | Left |
| 10 | Right |
| 11 | Down |
| 14 | LED gate (→ IRLML6402) |
| 15 | MAX98357A `SD` (amp enable) |
