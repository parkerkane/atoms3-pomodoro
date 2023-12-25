#include "pomodoro.h"

namespace sound {

void notifyTimed(unsigned long currentTimeMs)
{
    static unsigned long oldPos = ULONG_MAX;
    unsigned long soundPos = currentTimeMs / SOUND_NOTIFY_INTERVAL_S / mS_TO_S_FACTOR; // Every 60s

    if (soundPos != oldPos) {
        printf("Buzz.\r\n");

        tone(BUZZ_PIN, 440, 5);
        delay(100);
        tone(BUZZ_PIN, 440, 5);

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

}
