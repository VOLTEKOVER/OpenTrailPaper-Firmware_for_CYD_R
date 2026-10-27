#if defined(ARDUINO) && defined(BOARD_CYD)

#include "epd_compat.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_heap_caps.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "diag.h"

namespace {

constexpr int kNativeWidth = 240;
constexpr int kNativeHeight = 320;
constexpr size_t kFramebufferSize = (size_t)kNativeWidth * kNativeHeight / 2;

TFT_eSPI g_tft;
uint8_t* g_framebuffer = nullptr;

}  // namespace

bool epdc_begin() {
    if (g_framebuffer) return true;

    g_tft.begin();
    g_tft.setRotation(0);
    g_tft.fillScreen(TFT_WHITE);
    pinMode(BOARD_BL_EN, OUTPUT);
    digitalWrite(BOARD_BL_EN, HIGH);
    epd_set_rotation(DISPLAY_ROTATION);

    g_framebuffer = (uint8_t*)heap_caps_malloc(kFramebufferSize, MALLOC_CAP_8BIT);
    if (!g_framebuffer) {
        diag::log("CYD TFT: framebuffer allocation failed (%u bytes)",
                  (unsigned)kFramebufferSize);
        return false;
    }
    memset(g_framebuffer, 0xFF, kFramebufferSize);
    g_tft.setTextColor(TFT_BLACK, TFT_WHITE);
    g_tft.setTextDatum(MC_DATUM);
    g_tft.drawString("CYD TFT ready", g_tft.width() / 2, g_tft.height() / 2);
    diag::log("CYD TFT initialized: %dx%d", g_tft.width(), g_tft.height());
    return true;
}

uint8_t* epdc_framebuffer() { return g_framebuffer; }
int epdc_grey_levels() { return 16; }

void epdc_paint() {
    if (!g_framebuffer) return;

    constexpr int kRowsPerTransfer = 8;
    uint16_t rows[kNativeWidth * kRowsPerTransfer];
    for (int y = 0; y < kNativeHeight; y += kRowsPerTransfer) {
        for (int dy = 0; dy < kRowsPerTransfer; ++dy) {
            const int py = y + dy;
            for (int x = 0; x < kNativeWidth; ++x) {
                const uint8_t packed =
                    g_framebuffer[(size_t)py * (kNativeWidth / 2) + (x >> 1)];
                const uint8_t level = (x & 1) ? packed >> 4 : packed & 0x0F;
                const uint8_t grey = (uint8_t)(level * 17);
                rows[dy * kNativeWidth + x] = g_tft.color565(grey, grey, grey);
            }
        }
        g_tft.pushImage(0, y, kNativeWidth, kRowsPerTransfer, rows);
    }
}

void epdc_paint_wait() {}
void epdc_power_off_wait() {}
bool epdc_power_off_verify() { return true; }
void epdc_park_pins() {}

void epdc_clear(int passes) {
    (void)passes;
    if (!g_framebuffer) return;
    memset(g_framebuffer, 0xFF, kFramebufferSize);
    g_tft.fillScreen(TFT_WHITE);
}

void epdc_clear_dirty(int tolerance) {
    (void)tolerance;
}

#endif
