#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

namespace {

constexpr int kTouchCs = 33;
constexpr int kTouchIrq = 36;
constexpr int kTouchSck = 25;
constexpr int kTouchMiso = 39;
constexpr int kTouchMosi = 32;
constexpr int kBacklight = 21;

TFT_eSPI tft;
XPT2046_Touchscreen touch(kTouchCs, kTouchIrq);
uint32_t lastTouchLog = 0;
uint32_t lastHeartbeat = 0;

}  // namespace

void setup() {
    Serial.begin(115200);
    pinMode(kBacklight, OUTPUT);
    digitalWrite(kBacklight, HIGH);

    tft.init();
    tft.setRotation(0);
    const int width = tft.width();
    const int height = tft.height();

    setup_t setup;
    tft.getSetup(setup);
    Serial.printf("TFT: driver=0x%04x native=%ux%u logical=%dx%d SPI=%d MOSI=%d MISO=%d SCK=%d CS=%d DC=%d RST=%d\n",
                  setup.tft_driver, setup.tft_width, setup.tft_height,
                  width, height,
                  setup.port, setup.pin_tft_mosi, setup.pin_tft_miso,
                  setup.pin_tft_clk, setup.pin_tft_cs, setup.pin_tft_dc,
                  setup.pin_tft_rst);

    constexpr uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE};
    tft.fillScreen(TFT_BLACK);
    for (int i = 0; i < 4; ++i) {
        const int x0 = width * i / 4;
        const int x1 = width * (i + 1) / 4;
        tft.fillRect(x0, 0, x1 - x0, height, colors[i]);
        tft.setTextColor(i == 3 ? TFT_BLACK : TFT_WHITE, colors[i]);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(String(i + 1), (x0 + x1) / 2, height / 2, 4);
    }

    SPI.begin(kTouchSck, kTouchMiso, kTouchMosi);
    touch.begin();
    touch.setRotation(0);
    Serial.println("CYD portrait probe ready; tap near the four screen corners.");
    Serial.println("Record raw X/Y from Serial Monitor for TL, TR, BL, BR.");
    lastHeartbeat = millis();
}

void loop() {
    if (touch.touched() && millis() - lastTouchLog >= 250) {
        lastTouchLog = millis();
        const TS_Point point = touch.getPoint();
        Serial.printf("XPT2046 raw: x=%d y=%d pressure=%d\n",
                      point.x, point.y, point.z);
    }

    if (millis() - lastHeartbeat >= 2000) {
        lastHeartbeat = millis();
        Serial.println("CYD probe running");
    }
}
