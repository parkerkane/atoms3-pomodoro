#include "pomodoro.h"

namespace display {

Arduino_DataBus* displayBus = new Arduino_ESP32SPI(
    33 /* DC */,
    15 /* CS */,
    17 /* SCK */,
    21 /* MOSI */,
    GFX_NOT_DEFINED /* MISO */);

Arduino_GFX* display = new Arduino_GC9107(
    displayBus,
    34 /* RST */,
    0 /* rotation */,
    true /* IPS */);

void setup()
{
    display->begin(27000000);
    display->setRotation(0);
}

void setBacklight(int val)
{
    static int lastVal = -1;

    if (val != lastVal) {
        //        printf("Set backlight: %i\r\n", val);
        analogWrite(DISPLAY_BL_PIN, val);
    }

    lastVal = val;
}

void drawFullClock()
{
    display->fillArc(64, 64, 48, 16, 0, 360, RED);
}

void clearTime()
{
    display->fillRect(64 - 6, 64 - 5, 14, 10, BG_COLOR);
}

void clearScreen()
{
    display->fillScreen(BG_COLOR);

    display->drawCircle(64, 63, 16, MAROON);
    display->drawCircle(64, 63, 48, MAROON);

#ifdef DEV
    display->setCursor(64 - 6, 0);
    display->setFont();
    display->print("Dev");
#endif

    drawFullClock();
}

void drawTime(unsigned long currentTimeMs)
{
    static unsigned long lastLeftTimeS = 0;

    unsigned long leftTimeS = (TIMER_LENGTH_MS - currentTimeMs) / mS_TO_S_FACTOR;

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

void drawClock(unsigned long currentTimeMs)
{
    float progressingArcPos = 360.0f - (float)((currentTimeMs * 360 / TIMER_LENGTH_MS) % 360);
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

void drawCycleIndicators(int cycleCount)
{
    display->fillCircle(8, 8, 16, cycleCount >= 1 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 8, 16, cycleCount >= 2 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(8, 128 - 8, 16, cycleCount >= 3 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 128 - 8, 16, cycleCount >= 4 ? REMINDER_DOT_COLOR : BG_COLOR);
}

void notifyTimed(unsigned long currentTimeMs)
{
    static bool blinkState = false;

    unsigned long blinkPos = (currentTimeMs * 4 / mS_TO_S_FACTOR) % (2 * 1); // Every 1s

    if (presence::getMovedState()) {
        setBacklight(TFT_LIGHT_HIGH);

        return;
    }

    if (blinkPos == 0 && !blinkState) {
        printf("Blink.\r\n");

        setBacklight(TFT_LIGHT_HIGH);

        blinkState = true;
    } else if (blinkPos != 0 && blinkState) {
        setBacklight(TFT_LIGHT_LOW);

        blinkState = false;
    }
}

void resetBacklight()
{
    setBacklight(TFT_LIGHT_LOW);
}

/* namespace display */
}
