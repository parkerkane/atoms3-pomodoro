#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>

#define uS_TO_S_FACTOR 1000000ULL
#define mS_TO_S_FACTOR 1000ULL

#define DISPLAY_BL_PIN 16
#define BUTTON_PIN 41
#define BUZZ_PIN 38

#define ARC_CENTER_POS (+270)

#define TFT_LOW 0xCC
#define TFT_HIGH 0xFF

#define BG_COLOR RGB565(32, 0, 0)
#define REMINDER_DOT_COLOR RGB565(255 / 2, 130 / 2, 198 / 2)

#define TIMER_LENGHT_MS (25 * 60 * mS_TO_S_FACTOR)
#define SHUTDOWN_TIME_MS (45 * 60 * mS_TO_S_FACTOR)

Arduino_DataBus* displayBus = new Arduino_ESP32SPI(33 /* DC */, 15 /* CS */, 17 /* SCK */, 21 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX* display = new Arduino_GC9107(displayBus, 34 /* RST */, 0 /* rotation */, true /* IPS */);

int cycleCount = 0;

bool blinkState = false;
bool soundState = false;
bool cycleState = false;

unsigned long startTimeMs = 0;
unsigned long lastLeftTimeS = 0;
unsigned long currentTimeMs;

void ARDUINO_ISR_ATTR buttonInt()
{
    // Reset Time
    startTimeMs = millis();

    // Reset Cycle count back to zero
    if (cycleCount >= 4) {
        cycleCount = 0;
    }
}

void clearScreen()
{
    display->fillScreen(BG_COLOR);
    display->setRotation(0);

    display->fillArc(64, 63, 48, 16, 0, 360, PINK);
}

void drawTime()
{
    unsigned long leftTimeS = (TIMER_LENGHT_MS - currentTimeMs) / mS_TO_S_FACTOR;

    if (lastLeftTimeS != leftTimeS) {
        printf("Minutes left: %u.%02u\r\n", leftTimeS / 60, leftTimeS % 60);

        char strbuf[64];
        snprintf(strbuf, 64, "%02u", (leftTimeS / 60) + 1);

        display->setCursor(64 - 6, 64 + 5);
        display->setFont(u8g2_font_7x14_mr);
        display->fillRect(64 - 6, 64 - 5, 14, 10, BG_COLOR);
        display->setTextColor(WHITE);
        display->println(strbuf);
    }
    lastLeftTimeS = leftTimeS;
}

void drawClock()
{
    cycleState = false;

    unsigned long arcPos = 360 - (int(float(currentTimeMs) * 360 / TIMER_LENGHT_MS) % 360);
    int arcStartPos = ARC_CENTER_POS + ((currentTimeMs * 6 / mS_TO_S_FACTOR) % 360);

    if (blinkState == true) {
        analogWrite(DISPLAY_BL_PIN, TFT_LOW);
        blinkState = false;
    }

    display->fillArc(64, 64, 48, 16, arcStartPos, arcPos + arcStartPos, RED);
    display->fillArc(64, 64, 48, 16, arcPos + arcStartPos, 360 + arcStartPos, RGB565(64, 0, 0));
}

void drawCycleIndicators()
{
    display->fillCircle(8, 8, 16, cycleCount >= 1 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 8, 16, cycleCount >= 2 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(8, 128 - 8, 16, cycleCount >= 3 ? REMINDER_DOT_COLOR : BG_COLOR);
    display->fillCircle(128 - 8, 128 - 8, 16, cycleCount >= 4 ? REMINDER_DOT_COLOR : BG_COLOR);
}

void drawFullClock()
{
    display->fillArc(64, 64, 48, 16, 0, 360, RED);

    display->fillRect(64 - 6, 64 - 5, 14, 10, BG_COLOR);
}

void displayNotify()
{
    int blinkPos = (currentTimeMs * 10 / mS_TO_S_FACTOR) % (10 * 2);

    if (blinkPos == 0 && blinkState == false) {
        printf("Blink.\r\n");

        analogWrite(DISPLAY_BL_PIN, TFT_HIGH);
        blinkState = true;
    } else if (blinkPos != 0 && blinkState == true) {
        analogWrite(DISPLAY_BL_PIN, TFT_LOW);
        blinkState = false;
    }
}

void soundNotify()
{
    int soundPos = (currentTimeMs * 10 / mS_TO_S_FACTOR) % (10 * 60);

    if (soundPos == 0 && soundState == false) {
        printf("Buzz.\r\n");

        tone(BUZZ_PIN, 440, 5);
        delay(100);
        tone(BUZZ_PIN, 440, 5);

        soundState = true;
    } else if (soundPos != 0 && soundState == true) {
        soundState = false;
    }
}

void shutdownDevice()
{
    analogWrite(DISPLAY_BL_PIN, 0);
    printf("Sleeping.\r\n");
    esp_deep_sleep_start();
}

void updateCycleCount()
{
    if (cycleState == false) {
        printf("Update resetCount\r\n");
        cycleState = true;
        cycleCount++;
    }
}

/*************************************************************************************************/

void setup()
{
    printf("Hello world!\n\r");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, buttonInt, RISING);

    ledcChangeFrequency(analogGetChannel(DISPLAY_BL_PIN), 500, 8);
    analogWrite(DISPLAY_BL_PIN, TFT_LOW);

    display->begin(27000000);
    clearScreen();

    startTimeMs = millis();
}


void loop()
{
    currentTimeMs = millis() - startTimeMs;

    if (currentTimeMs < TIMER_LENGHT_MS) {
        drawTime();
        drawClock();
        drawCycleIndicators();

        delay(100);
    } else if (currentTimeMs < SHUTDOWN_TIME_MS) {
        drawFullClock();
        drawCycleIndicators();

        updateCycleCount();
        displayNotify();
        soundNotify();

        delay(25);
    } else {
        shutdownDevice();
    }
}