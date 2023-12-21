#include "pomodoro.h"

Arduino_DataBus *displayBus = new Arduino_ESP32SPI(
        33 /* DC */,
        15 /* CS */,
        17 /* SCK */,
        21 /* MOSI */,
        GFX_NOT_DEFINED /* MISO */);

Arduino_GFX *display = new Arduino_GC9107(
        displayBus,
        34 /* RST */,
        0 /* rotation */,
        true /* IPS */);

void displaySetup()
{
    display->begin(27000000);
    display->setRotation(0);
}

void displaySetBacklight(int val)
{
    static int lastVal = -1;

    if (val != lastVal) {
        printf("Set backlight: %i\r\n", val);
        analogWrite(DISPLAY_BL_PIN, val);
    }

    lastVal = val;
}

void displayDrawFullClock()
{
    display->fillArc(64, 64, 48, 16, 0, 360, RED);
}

void displayClearTime()
{
    display->fillRect(64 - 6, 64 - 5, 14, 10, BG_COLOR);
}

void displayClearScreen()
{
    display->fillScreen(BG_COLOR);

    display->drawCircle(64, 63, 16, PINK);
    display->drawCircle(64, 63, 48, PINK);

    displayDrawFullClock();
}

void displayDrawTime(unsigned long currentTimeMs)
{
    static unsigned long lastLeftTimeS = 0;

    unsigned long leftTimeS = (TIMER_LENGHT_MS - currentTimeMs) / mS_TO_S_FACTOR;

    if (lastLeftTimeS != leftTimeS) {
        printf("Time left: %lu:%02lu\r\n", leftTimeS / 60, leftTimeS % 60);

        char stringBuffer[64];
        snprintf(stringBuffer, 64, "%02lu", (leftTimeS / 60) + 1);

        display->setCursor(64 - 6, 64 + 5);
        display->setFont(u8g2_font_7x14_mr);
        display->fillRect(64 - 6, 64 - 5, 14, 10, BG_COLOR);
        display->setTextColor(WHITE);
        display->println(stringBuffer);
    }
    lastLeftTimeS = leftTimeS;
}

void displayDrawClock(unsigned long currentTimeMs)
{
    float progressingArcPos = 360.0f - (float) ((currentTimeMs * 360 / TIMER_LENGHT_MS) % 360);
    float timeRotatingArcPos = ARC_CENTER_POS + ((currentTimeMs * 6 / mS_TO_S_FACTOR) % 360);

    display->fillArc(
            64, 64,
            48, 16,
            timeRotatingArcPos,
            progressingArcPos + timeRotatingArcPos,
            RED);

    display->fillArc(
            64, 64,
            48, 16,
            progressingArcPos + timeRotatingArcPos,
            360 + timeRotatingArcPos,
            RGB565(64, 0, 0));
}

void displayDrawCycleIndicators(int cycleCount)
{
    display->fillCircle(8, 8, 16, cycleCount >= 1 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 8, 16, cycleCount >= 2 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(8, 128 - 8, 16, cycleCount >= 3 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 128 - 8, 16, cycleCount >= 4 ? REMINDER_DOT_COLOR : BG_COLOR);
}

void displayNotifyTimed(unsigned long currentTimeMs)
{
    static bool blinkState = false;

    unsigned long blinkPos = (currentTimeMs * 10 / mS_TO_S_FACTOR) % (10 * 2); // Every 2s

    if (blinkPos == 0 && !blinkState) {
        printf("Blink.\r\n");

        displaySetBacklight(TFT_LIGHT_HIGH);

        blinkState = true;
    } else if (blinkPos != 0 && blinkState) {
        displaySetBacklight(TFT_LIGHT_LOW);

        blinkState = false;
    }
}

void displayResetBacklight()
{
    displaySetBacklight(TFT_LIGHT_LOW);
}