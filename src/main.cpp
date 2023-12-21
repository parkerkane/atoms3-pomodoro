#include "pomodoro.h"

/*************************************************************************************************/

enum State
{
    STATE_INIT,
    STATE_TIMER,
    STATE_TIMER_RUN,
    STATE_NOTIFY,
    STATE_NOTIFY_RUN,
    STATE_SLEEP,
    STATE_SLEEP_RUN,
};

enum ButtonState
{
    BUTTON_UP,
    BUTTON_UP_RUN,
    BUTTON_DOWN,
    BUTTON_DOWN_RUN,
    BUTTON_DOWN_LONG,
    BUTTON_DOWN_LONG_RUN,
};

#ifdef DEV
volatile State state = STATE_INIT;
#else
volatile State state = STATE_SLEEP;
#endif

volatile ButtonState buttonState;

/*************************************************************************************************/

volatile int cycleCount = 0;
volatile unsigned long startTimeMs = 0;

bool soundMuted = false;

unsigned long currentTimeMs;

/*************************************************************************************************/

void soundNotifyTimed()
{
    if (soundMuted) {
        return;
    }

    static unsigned long oldPos = ULONG_MAX;
    unsigned long soundPos = currentTimeMs / 60 / mS_TO_S_FACTOR; // Every 60s

    if (soundPos != oldPos) {
        printf("Buzz.\r\n");

        tone(BUZZ_PIN, 440, 5);
        delay(100);
        tone(BUZZ_PIN, 440, 5);

        oldPos = soundPos;
    }
}

void soundNotifyShort()
{
    tone(BUZZ_PIN, 440, 5);
}

void soundNotifyLong()
{
    tone(BUZZ_PIN, 440, 25);
    delay(100);
    tone(BUZZ_PIN, 220, 25);
}

void soundNotifyMute()
{
    tone(BUZZ_PIN, 1000, 50);
}

void soundNotifyShutdown()
{
    tone(BUZZ_PIN, 1000, 25);
    delay(125);
    tone(BUZZ_PIN, 500, 50);
    delay(250);
    tone(BUZZ_PIN, 250, 100);
    delay(250);
}

/*************************************************************************************************/

void updateCycleCount()
{
    cycleCount++;
    printf("Updated cycle count: %i\r\n", cycleCount);
}

void updateCurrentTime()
{
    currentTimeMs = millis() - startTimeMs;
}

/*************************************************************************************************/

void ARDUINO_ISR_ATTR

buttonClickDown()
{
    buttonState = BUTTON_DOWN;
}

/*************************************************************************************************/

void setup()
{
    printf("Hello world!\n\r");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(BUTTON_PIN, buttonClickDown, FALLING);

    ledcChangeFrequency(analogGetChannel(DISPLAY_BL_PIN), 500, 8);
    analogWrite(DISPLAY_BL_PIN, TFT_LIGHT_LOW);

    displaySetup();
    displayClearScreen();
    soundNotifyLong();
}

/*************************************************************************************************/

void loop_main()
{
    double dValue;

    updateCurrentTime();

    switch (state) {
        case STATE_INIT:
            startTimeMs = millis();

            // Reset Cycle count back to zero
            if (cycleCount >= 4) {
                cycleCount = 0;
            }

            soundMuted = false;

            state = STATE_TIMER;
            break;

        case STATE_TIMER:
            setCpuFrequencyMhz(40);
            displayClearScreen();
            displayResetBacklight();
            displayDrawCycleIndicators(cycleCount);

            soundNotifyShort();

            state = STATE_TIMER_RUN;

        case STATE_TIMER_RUN:
            if (currentTimeMs > TIMER_LENGTH_MS) {
                state = STATE_NOTIFY;
                break;
            }

            displayDrawTime(currentTimeMs);
            displayDrawClock(currentTimeMs);

            delay(100);
            break;

        case STATE_NOTIFY:
            setCpuFrequencyMhz(240);

            updateCycleCount();

            displayClearScreen();
            displayDrawFullClock();
            displayDrawCycleIndicators(cycleCount);
            displayClearTime();

            setCpuFrequencyMhz(10);

            state = STATE_NOTIFY_RUN;

        case STATE_NOTIFY_RUN:
            if (currentTimeMs - TIMER_LENGTH_MS > NOTIFY_TIMEOUT_MS) {
                state = STATE_SLEEP;
                break;
            }

            displayNotifyTimed(currentTimeMs);
            soundNotifyTimed();

            delay(25);
            break;

        case STATE_SLEEP:
            setCpuFrequencyMhz(240);

            cycleCount = 0;
            startTimeMs = millis();
            displayClearScreen();
            soundNotifyShutdown();

            setCpuFrequencyMhz(20);

            state = STATE_SLEEP_RUN;

        case STATE_SLEEP_RUN:
            dValue = pow((cos(currentTimeMs * 3.1415 / (15 * mS_TO_S_FACTOR)) + 1.0) / 2, 4.0);
            if (dValue < 0.7) {
                dValue = 0;
            }
            displaySetBacklight((int)(dValue * TFT_LIGHT_LOW));

            delay(50);

            break;

        default:
            state = STATE_INIT;
            break;
    }
}

/*************************************************************************************************/

void handleButtonClick()
{
    switch (state) {
        case STATE_TIMER:
        case STATE_TIMER_RUN:
            // Don't do anything
            break;

        case STATE_NOTIFY:
        case STATE_NOTIFY_RUN:
        case STATE_SLEEP:
        case STATE_SLEEP_RUN:
            state = STATE_INIT;
            break;

        default:
            state = STATE_INIT;
    }
}

void handleButtonLongClick()
{
    switch (state) {
        case STATE_NOTIFY:
        case STATE_NOTIFY_RUN:
            if (!soundMuted) {
                soundNotifyMute();
                soundMuted = true;
            } else {
                state = STATE_SLEEP;
            }
            break;

        default:
            state = STATE_SLEEP;
    }
}

void loop_button()
{
    static unsigned long buttonDownTime;

    switch (buttonState) {

        case BUTTON_UP:
            buttonDownTime = 0;
            buttonState = BUTTON_UP_RUN;

        case BUTTON_UP_RUN:
            break;

        case BUTTON_DOWN:
            buttonDownTime = millis();
            soundNotifyShort();

            buttonState = BUTTON_DOWN_RUN;

        case BUTTON_DOWN_RUN:
            if (digitalRead(BUTTON_PIN) == HIGH) {
                handleButtonClick();

                buttonState = BUTTON_UP;
            } else if (millis() - buttonDownTime > 1000) {
                buttonState = BUTTON_DOWN_LONG;
            }

            break;

        case BUTTON_DOWN_LONG:
            soundNotifyLong();
            buttonState = BUTTON_DOWN_LONG_RUN;

        case BUTTON_DOWN_LONG_RUN:
            if (digitalRead(BUTTON_PIN) == HIGH) {
                handleButtonLongClick();

                buttonState = BUTTON_UP;
            }

            break;

        default:
            buttonState = BUTTON_UP;
    }
}

/*************************************************************************************************/

void loop()
{
    loop_button();
    loop_main();
}