#pragma once
#include <TFT_eSPI.h>
#include <pgmspace.h>
#include "fonts/cyrillic_font.h"

static inline int32_t vlwRead32(const uint8_t* p) {
    return ((int32_t)pgm_read_byte(p) << 24) |
           ((int32_t)pgm_read_byte(p+1) << 16) |
           ((int32_t)pgm_read_byte(p+2) << 8) |
           pgm_read_byte(p+3);
}

static inline bool hasCyrillic(const char* s) {
    while (*s) {
        uint8_t c = (uint8_t)*s;
        if (c >= 0xD0 && c <= 0xD3) return true;
        s++;
    }
    return false;
}

static inline uint32_t decodeUTF8(const char*& s) {
    uint8_t c = (uint8_t)*s;
    if (c < 0x80) { s++; return c; }
    if ((c & 0xE0) == 0xC0) {
        uint32_t cp = (c & 0x1F) << 6;
        cp |= ((uint8_t)s[1] & 0x3F);
        s += 2; return cp;
    }
    if ((c & 0xF0) == 0xE0) {
        uint32_t cp = (c & 0x0F) << 12;
        cp |= ((uint8_t)s[1] & 0x3F) << 6;
        cp |= ((uint8_t)s[2] & 0x3F);
        s += 3; return cp;
    }
    s++; return '?';
}

// Draw string using VLW font from PROGMEM for ALL characters.
// Uses the font's own glyphs for both ASCII and Cyrillic.
static int drawStringVLW(TFT_eSprite& spr, const char* text, int x, int y, uint16_t color) {
    int gCount = vlwRead32(cyrillic_font);
    int ascent = vlwRead32(cyrillic_font + 16);
    const uint8_t* metrics = cyrillic_font + 24;

    // Pre-compute bitmap offsets for each glyph
    // Bitmap data starts after all metrics (gCount * 28 bytes)
    const uint8_t* bmpBase = metrics + gCount * 28;

    int curX = x;
    const char* p = text;

    while (*p) {
        uint32_t cp = decodeUTF8(p);

        int bmpOff = 0;
        bool found = false;
        for (int i = 0; i < gCount; i++) {
            const uint8_t* gm = metrics + i * 28;
            int32_t gUni = vlwRead32(gm);
            int32_t gH   = vlwRead32(gm + 4);
            int32_t gW   = vlwRead32(gm + 8);
            int32_t gAdv = vlwRead32(gm + 12);
            int32_t gdY  = vlwRead32(gm + 16);
            int32_t gdX  = vlwRead32(gm + 20);

            if ((uint32_t)gUni == cp) {
                const uint8_t* bmp = bmpBase + bmpOff;
                for (int py = 0; py < gH; py++) {
                    for (int px = 0; px < gW; px++) {
                        uint8_t alpha = pgm_read_byte(bmp + py * gW + px);
                        if (alpha > 96)
                            spr.drawPixel(curX + gdX + px, y + gdY + py, color);
                    }
                }
                curX += gAdv;
                found = true;
                break;
            }
            bmpOff += gW * gH;
        }
        if (!found) {
            spr.drawChar('?', curX, y);
            curX += 6;
        }
    }
    return curX - x;
}

// Width of string rendered with VLW font
static int textWidthVLW(const char* text) {
    int gCount = vlwRead32(cyrillic_font);
    const uint8_t* metrics = cyrillic_font + 24;
    int w = 0;
    const char* p = text;
    while (*p) {
        uint32_t cp = decodeUTF8(p);
        bool found = false;
        for (int i = 0; i < gCount; i++) {
            const uint8_t* gm = metrics + i * 28;
            if ((uint32_t)vlwRead32(gm) == cp) {
                w += vlwRead32(gm + 12);
                found = true;
                break;
            }
        }
        if (!found) w += 6;
    }
    return w;
}

// Smart draw: uses VLW font only if string has Cyrillic, otherwise stock drawString
static void drawStringAuto(TFT_eSprite& spr, const char* text, int x, int y, uint16_t color) {
    if (hasCyrillic(text)) {
        drawStringVLW(spr, text, x, y, color);
    } else {
        spr.setTextColor(color);
        spr.drawString(text, x, y);
    }
}
