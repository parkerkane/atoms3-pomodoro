#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <U8g2lib.h>

#define uS_TO_S_FACTOR 1000000ULL
#define mS_TO_S_FACTOR 1000ULL

#define DISPLAY_BL_PIN 16
#define BUTTON_PIN 41
#define BUZZ_PIN 38

#define ARC_CENTER_POS (+270)

#define TFT_LIGHT_LOW 0xDD
#define TFT_LIGHT_HIGH 0xFF

#define BG_COLOR RGB565(32, 0, 0)
#define REMINDER_DOT_COLOR RGB565(255 / 2, 130 / 2, 198 / 2)

#define TIMER_LENGHT_MS (25 * 60 * mS_TO_S_FACTOR)
#define NOTIFY_TIMEOUT_MS (30 * 60 * mS_TO_S_FACTOR)
// #define SHUTDOWN_TIME_MS (45 * 60 * mS_TO_S_FACTOR)

long getCurrentTime();

void displaySetup();

void displayClearScreen();

void displaySetBacklight(int val);
void displayDrawFullClock();
void displayClearTime();
void displayClearScreen();
void displayDrawTime(unsigned long currentTimeMs);
void displayDrawClock(unsigned long currentTimeMs);
void displayDrawCycleIndicators(int cycleCount);
void displayNotifyTimed(unsigned long currentTimeMs);
void displayResetBacklight();
