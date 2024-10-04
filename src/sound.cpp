#include "pomodoro.h"

namespace sound {

int notifyState = 1;

void notifyTimed(unsigned long currentTimeMs)
{
    static unsigned long oldPos = ULONG_MAX;
    unsigned long soundPos = currentTimeMs / SOUND_NOTIFY_INTERVAL_S / mS_TO_S_FACTOR; // Every 60s

    if (soundPos != oldPos) {
        printf("Buzz.\r\n");

        unsigned int hbTime = millis() - ble::getHerthbeatTime();
        unsigned int prTime = millis() - presence::getPresenceTime();

        unsigned int minTime = min(hbTime, prTime);

        if (minTime > (10) * mS_TO_S_FACTOR) {
            resetNotifyState();
        } else {
            increaseNotifyState();
        }

        int aggressiveNotificationCount = min(512, (int)pow(1.5f, (float)max(0, notifyState - 1)));

        if (aggressiveNotificationCount <= 1) {
            tone(BUZZ_PIN, 440, 5);
            delay(100);
            tone(BUZZ_PIN, 440, 5);
            delay(100);

        } else {
            for (int i = 0; i < aggressiveNotificationCount; i++) {
                tone(BUZZ_PIN, 440, 15);
                tone(BUZZ_PIN, 880, 25);
                delay(50);
                tone(BUZZ_PIN, 440, 15);
                delay(50);
            }
        }

        // tone(BUZZ_PIN, 440, 5 * (1+notifyState));

        oldPos = soundPos;
    }
}

void notifyShort()
{
    tone(BUZZ_PIN, 440, 5);
}

void notifyLong()
{
    tone(BUZZ_PIN, 440, 25);
    delay(100);
    tone(BUZZ_PIN, 220, 25);
}

void notifyMute()
{
    tone(BUZZ_PIN, 1000, 50);
}

void notifyShutdown()
{
    tone(BUZZ_PIN, 1000, 25);
    delay(125);
    tone(BUZZ_PIN, 500, 50);
    delay(250);
    tone(BUZZ_PIN, 250, 100);
    delay(250);
}

void notifyDenied() 
{
    tone(BUZZ_PIN, 220, 200);
    delay(250);
    tone(BUZZ_PIN, 110, 500);
    delay(500);
}

void resetNotifyState()
{
    notifyState = 1;
}

void increaseNotifyState()
{
    notifyState++;
}

}
