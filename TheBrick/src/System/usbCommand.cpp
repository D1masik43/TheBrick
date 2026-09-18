#include "System/usbCommand.h"
#include "System/systemDrivers.h"
#include <Wire.h>

UsbCommand &UsbCommand::Get()
{
    static UsbCommand instance;
    return instance;
}

// Every I2C user on the brick shares one bus, so the mutex is not optional:
// the UI task polls the RTC and the touch controller from its own loop.
bool UsbCommand::Ina(float &vbus, float &shunt, float &current, float &power)
{
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    Adafruit_INA219 &ina = SystemDrivers::Get().GetINA219();
    vbus    = ina.getBusVoltage_V();
    shunt   = ina.getShuntVoltage_mV();
    current = ina.getCurrent_mA();
    power   = ina.getPower_mW();

    xSemaphoreGive(i2cMutex);
    return true;
}

void UsbCommand::ReplyIna()
{
    float v, s, c, p;
    if (!Ina(v, s, c, p))
    {
        Serial.println("ERR=i2c busy");
        return;
    }
    Serial.printf("VBUS=%.3f SHUNT=%.3f CURR=%.1f PWR=%.1f\n", v, s, c, p);
}

// One value per line: this is the telemetry format lab 5 parses.
void UsbCommand::ReplyCurrent()
{
    float v, s, c, p;
    if (!Ina(v, s, c, p))
    {
        Serial.println("ERR=i2c busy");
        return;
    }
    Serial.printf("CURR=%.1f\n", c);
}

void UsbCommand::ReplyScan()
{
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) != pdTRUE)
    {
        Serial.println("ERR=i2c busy");
        return;
    }

    // A device is present when it acknowledges its address byte; no data
    // bytes are exchanged, which is exactly the scan transaction of lab 4.
    Serial.print("I2C=");
    bool first = true;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.printf("%s0x%02x", first ? "" : ",", addr);
            first = false;
        }
    }
    Serial.println(first ? "none" : "");

    xSemaphoreGive(i2cMutex);
}

// Kept from the previous loop() handler: "26:03:22:23:01:00" or
// "2026/03/22 23:01:00" - any single-character separator works.
bool UsbCommand::TrySetRtc(const char *text)
{
    int y, m, d, hh, mm, ss;
    if (sscanf(text, "%d %*c %d %*c %d %*c %d %*c %d %*c %d",
               &y, &m, &d, &hh, &mm, &ss) != 6)
        return false;

    if (y < 100)
        y += 2000;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        SystemDrivers::Get().GetRTC().adjust(DateTime(y, m, d, hh, mm, ss));
        xSemaphoreGive(i2cMutex);
        Serial.println("RTC Updated!");
    }
    else
    {
        Serial.println("ERR=i2c busy");
    }
    return true;
}

void UsbCommand::Handle(char *text)
{
    while (*text == ' ')
        text++;
    if (*text == '\0')
        return;

    if (!strcasecmp(text, "*IDN?") || !strcasecmp(text, "ID?"))
    {
        Serial.println("ID=TheBrick ESP32-S3 USB-CDC, INA219 telemetry");
        return;
    }

    if (!strcasecmp(text, "INA?"))
    {
        ReplyIna();
        return;
    }

    if (!strcasecmp(text, "CURR?"))
    {
        ReplyCurrent();
        return;
    }

    if (!strcasecmp(text, "SCAN?"))
    {
        ReplyScan();
        return;
    }

    if (!strncasecmp(text, "POLL", 4) && (text[4] == ' ' || text[4] == '\0'))
    {
        const char *arg = text + 4;
        while (*arg == ' ')
            arg++;

        if (*arg == '\0')
        {
            Serial.printf("POLL=%lu\n", (unsigned long)pollMs);
            return;
        }
        if (!strcasecmp(arg, "OFF") || !strcmp(arg, "0"))
        {
            pollMs = 0;
            Serial.println("POLL=OFF");
            return;
        }

        long ms = strtol(arg, nullptr, 10);
        if (ms < 50 || ms > 60000)
        {
            Serial.println("ERR=poll interval out of range (50..60000)");
            return;
        }
        pollMs   = (uint32_t)ms;
        lastPoll = millis();
        Serial.printf("POLL=%lu\n", (unsigned long)pollMs);
        return;
    }

    if (!strncasecmp(text, "ECHO", 4) && (text[4] == ' ' || text[4] == '\0'))
    {
        const char *arg = text + 4;
        while (*arg == ' ')
            arg++;
        Serial.printf("ACK:%s\n", arg);
        return;
    }

    if (!strcasecmp(text, "HELP") || !strcmp(text, "?"))
    {
        Serial.println("CMD=*IDN? INA? CURR? SCAN? POLL <ms>|OFF ECHO <text> HELP");
        return;
    }

    if (TrySetRtc(text))
        return;

    Serial.println("ERR=unknown command");
}

void UsbCommand::Loop()
{
    while (Serial.available() > 0)
    {
        char c = (char)Serial.read();

        if (c == '\r')
            continue;

        if (c == '\n')
        {
            if (overflow)
            {
                Serial.println("ERR=line too long");
                overflow = false;
            }
            else
            {
                line[len] = '\0';
                Handle(line);
            }
            len = 0;
            continue;
        }

        if (len + 1 < LINE_CAP)
            line[len++] = c;
        else
            overflow = true;   // swallow the tail, report once at the newline
    }

    if (pollMs > 0 && millis() - lastPoll >= pollMs)
    {
        lastPoll = millis();
        ReplyCurrent();
    }
}
