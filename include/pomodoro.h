#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <U8g2lib.h>

#define uS_TO_S_FACTOR 1000000ULL
#define mS_TO_S_FACTOR 1000ULL

#define DISPLAY_BL_PIN 16
#define BUTTON_PIN 41
#define BUZZ_PIN 38

#define ARC_CENTER_POS (+270)

#define TFT_LIGHT_LOW 0xCC
#define TFT_LIGHT_HIGH 0xFF

#define BG_COLOR RGB565(32, 0, 0)
#define REMINDER_DOT_COLOR RGB565(255 / 2, 130 / 2, 198 / 2)

#ifdef DEV

#define TIMER_LENGTH_MS (5 * 1 * mS_TO_S_FACTOR)
#define NOTIFY_TIMEOUT_MS (15 * 60 * mS_TO_S_FACTOR)

#define SOUND_NOTIFY_INTERVAL_S 10
#define SLEEP_CYCLE_LENGTH_S 5

#else

#define TIMER_LENGTH_MS (25 * 60 * mS_TO_S_FACTOR)
#define NOTIFY_TIMEOUT_MS (60 * 60 * mS_TO_S_FACTOR)

#define SOUND_NOTIFY_INTERVAL_S 60
#define SLEEP_CYCLE_LENGTH_S 15

#endif
// #define SHUTDOWN_TIME_MS (45 * 60 * mS_TO_S_FACTOR)

/*************************************************************************************************/

namespace display {

void setup();
void setBacklight(int val);
void drawFullClock();
void clearTime();
void clearScreen();
void drawTime(unsigned long currentTimeMs);
void drawClock(unsigned long currentTimeMs);
void drawCycleIndicators(int cycleCount);
void notifyTimed(unsigned long currentTimeMs);
void resetBacklight();

}

/*************************************************************************************************/

namespace ble {

void setup();
void clearHearthbeatState();
bool getHerthbeatState();
unsigned long getHerthbeatTime();

}
/*************************************************************************************************/

namespace sound {

void notifyTimed(unsigned long currentTimeMs);
void notifyShort();
void notifyLong();
void notifyMute();
void notifyShutdown();

void resetNotifyState();
void increaseNotifyState();

}

/*************************************************************************************************/
