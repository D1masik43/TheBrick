#pragma once
#include <Arduino.h>

/* Line oriented command channel on the native USB CDC port.
 *
 * Host sends "<COMMAND>\n", the brick answers with one "<KEY>=<VALUE>...\n"
 * line. Nothing is printed unless it was asked for, so a host program can
 * treat the port as a request/response channel instead of a log.
 *
 *   *IDN?        -> ID=TheBrick ...
 *   INA?         -> VBUS=.. SHUNT=.. CURR=.. PWR=..
 *   CURR?        -> CURR=..
 *   SCAN?        -> I2C=0x3c,0x40,..
 *   POLL <ms>    -> POLL=<ms>, then a CURR= line every <ms> until POLL OFF
 *   POLL OFF     -> POLL=OFF
 *   ECHO <text>  -> ACK:<text>
 *   HELP         -> one line per command
 *   <date>       -> RTC Updated!      (kept from the old loop() handler)
 *
 * Reading is non-blocking: characters are collected as they arrive, so a slow
 * or half-typed line never stalls the UI loop.
 */
class UsbCommand
{
  public:
    static UsbCommand &Get();

    UsbCommand(const UsbCommand &) = delete;
    UsbCommand &operator=(const UsbCommand &) = delete;

    void Loop();          // drain the port, emit the periodic CURR= line

  private:
    UsbCommand() = default;

    void Handle(char *line);
    bool Ina(float &vbus, float &shunt, float &current, float &power);
    void ReplyIna();
    void ReplyCurrent();
    void ReplyScan();
    bool TrySetRtc(const char *line);

    static const size_t LINE_CAP = 96;

    char     line[LINE_CAP];
    size_t   len        = 0;
    bool     overflow   = false;   // drop the rest of an over-long line
    uint32_t pollMs     = 0;       // 0 = periodic telemetry off
    uint32_t lastPoll   = 0;
};
