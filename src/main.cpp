#include "pomodoro.h"

/*************************************************************************************************/

enum State {
    STATE_INIT,
    STATE_TIMER,
    STATE_TIMER_RUN,
    STATE_NOTIFY,
    STATE_NOTIFY_RUN,
    STATE_SLEEP,
    STATE_SLEEP_RUN,
    STATE_SETUP,
    STATE_BUTTON,
};

volatile State state;

/*************************************************************************************************/

volatile int cycleCount = 0;
volatile time_t startTimeMs = 0;

bool soundState = false;

time_t currentTimeMs;

/*************************************************************************************************/

int getCycleCount()
{
    return cycleCount;
}

time_t getCurrentTime()
{
    return currentTimeMs;
}

/*************************************************************************************************/

void soundNotifyTimed()
{
    int soundPos = (currentTimeMs * 10 / mS_TO_S_FACTOR) % (10 * 60); // Every 60s

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

void soundNotifyLong()
{
    tone(BUZZ_PIN, 440, 25);
    delay(100);
    tone(BUZZ_PIN, 220, 25);
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

volatile time_t buttonDownTimeMs = 0;
volatile bool buttonDownState = false;

void ARDUINO_ISR_ATTR buttonClickDown()
{
    buttonDownTimeMs = millis();
    buttonDownState = true;
}

void ARDUINO_ISR_ATTR buttonClickUp()
{
    buttonDownState = false;

    state = STATE_BUTTON;
}

/*************************************************************************************************/

void setup()
{
    // setCpuFrequencyMhz(40);
    printf("Hello world!\n\r");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(BUTTON_PIN, buttonClickDown, FALLING);
    attachInterrupt(BUTTON_PIN, buttonClickUp, RISING);

    ledcChangeFrequency(analogGetChannel(DISPLAY_BL_PIN), 500, 8);
    analogWrite(DISPLAY_BL_PIN, TFT_LIGHT_LOW);

    displaySetup();
    displayClearScreen();
    soundNotifyLong();
}

/*************************************************************************************************/

void handleStateSleepRun()
{
    int lightPos = ((sin(currentTimeMs * 3.1415 / (5 * mS_TO_S_FACTOR)) + 1.0) / 2) * 0xCC;

    displaySetBacklight(lightPos);
}

void handleStateSleepInit()
{
    cycleCount = 0;
    displayClearScreen();
}

void handleStateNotifyRun()
{
    displayNotifyTimed(currentTimeMs);
    soundNotifyTimed();
}

void handleStateNotifyInit()
{
    updateCycleCount();

    displayClearScreen();
    displayDrawFullClock();
    displayDrawCycleIndicators(cycleCount);
    displayClearTime();
}

void handleStateTimerRun()
{
    displayDrawTime(currentTimeMs);
    displayDrawClock(currentTimeMs);
}

void handleStateTimerInit()
{
    displayClearScreen();
    displayResetBacklight();
    displayDrawCycleIndicators(cycleCount);
}

void handleStateInit()
{
    startTimeMs = millis();

    // Reset Cycle count back to zero
    if (cycleCount >= 4) {
        cycleCount = 0;
    }
}

/*************************************************************************************************/

void loop()
{
    updateCurrentTime();

    switch (state) {
    case STATE_INIT:
        handleStateInit();

        state = STATE_TIMER;
        break;

    case STATE_TIMER:
        handleStateTimerInit();

        state = STATE_TIMER_RUN;

    case STATE_TIMER_RUN:
        if (currentTimeMs > TIMER_LENGHT_MS) {
            state = STATE_NOTIFY;
            break;
        }

        handleStateTimerRun();

        delay(1000);
        break;

    case STATE_NOTIFY:
        handleStateNotifyInit();

        state = STATE_NOTIFY_RUN;

    case STATE_NOTIFY_RUN:
        if (currentTimeMs > SHUTDOWN_TIME_MS) {
            state = STATE_SLEEP;
            break;
        }

        handleStateNotifyRun();

        delay(25);
        break;

    case STATE_SLEEP:
        handleStateSleepInit();

        state = STATE_SLEEP_RUN;

    case STATE_SLEEP_RUN:

        handleStateSleepRun();

        delay(50);

        break;

    case STATE_SETUP:
        state = STATE_INIT;
        break;

    case STATE_BUTTON:
        state = STATE_INIT;
        break;

    default:
        state = STATE_INIT;
        break;
    }
}
