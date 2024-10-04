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
};

#ifdef DEV
#define DEFAULT_STATE STATE_INIT
#else
#define DEFAULT_STATE STATE_SLEEP
#endif

enum ButtonState {
    BUTTON_UP,
    BUTTON_UP_RUN,
    BUTTON_DOWN,
    BUTTON_DOWN_RUN,
    BUTTON_DOWN_LONG,
    BUTTON_DOWN_LONG_RUN,
};

enum PresenceState {
    PRESENCE_SETUP,
    PRESENCE_UPDATE,
    PRESENCE_SLEEP,
};

volatile State state = DEFAULT_STATE;
volatile ButtonState buttonState;
volatile PresenceState presenceState;


/*************************************************************************************************/

volatile int cycleCount = 0;
volatile unsigned long startTimeMs = 0;

unsigned long currentTimeMs;

bool soundMuted = false;

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

    // ble::setup();

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(BUTTON_PIN, buttonClickDown, FALLING);

    ledcChangeFrequency(analogGetChannel(DISPLAY_BL_PIN), 500, 8);
    analogWrite(DISPLAY_BL_PIN, TFT_LIGHT_LOW);

    presence::setup();

    display::setup();
    display::clearScreen();
    sound::notifyLong();
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
        display::clearScreen();
        display::resetBacklight();
        display::drawCycleIndicators(cycleCount);

        sound::notifyShort();

        state = STATE_TIMER_RUN;

    case STATE_TIMER_RUN:
        if (currentTimeMs > TIMER_LENGTH_MS) {
            state = STATE_NOTIFY;
            break;
        }

        display::drawTime(currentTimeMs);
        display::drawClock(currentTimeMs);

        delay(100);
        break;

    case STATE_NOTIFY:
        updateCycleCount();

        display::clearScreen();
        display::drawFullClock();
        display::drawCycleIndicators(cycleCount);
        display::clearTime();
        sound::resetNotifyState();
        presence::clearMovedState();

        state = STATE_NOTIFY_RUN;

    case STATE_NOTIFY_RUN:
        if (currentTimeMs - TIMER_LENGTH_MS > NOTIFY_TIMEOUT_MS) {
            state = STATE_SLEEP;
            break;
        }

        display::notifyTimed(currentTimeMs);

        if (!soundMuted) {
            sound::notifyTimed(currentTimeMs);
        }

        delay(25);
        break;

    case STATE_SLEEP:
        cycleCount = 0;
        startTimeMs = millis();
        display::clearScreen();
        sound::notifyShutdown();

        // ble::clearHearthbeatState();
        presence::clearState();

        state = STATE_SLEEP_RUN;

        break;

    case STATE_SLEEP_RUN:
        // if (ble::getHerthbeatState() || presence::getState()) {
        if (presence::getState()) {
            state = STATE_INIT;
        }

        dValue = cos(currentTimeMs * 3.1415 / (SLEEP_CYCLE_LENGTH_S * mS_TO_S_FACTOR));

        if (dValue < 0.7) {
            dValue = 0;
        }
        display::setBacklight((int)(dValue * TFT_LIGHT_LOW));

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

        if (!presence::getMovedState()) {
            printf("User not moved away!\n\r");
            sound::notifyDenied();
            break;
        }

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
            sound::notifyMute();
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
        sound::notifyShort();

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
        sound::notifyLong();
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


void loop_presence()
{
    static unsigned long time;

    static bool lastMoved = false;

    switch (presenceState) {
        case PRESENCE_SETUP:
            presence::clearMovedState();
            presenceState = PRESENCE_SLEEP;
            break;

        case PRESENCE_UPDATE:
            presence::update();

            time = millis();
            presenceState = PRESENCE_SLEEP;

            if (lastMoved == false && presence::getMovedState()) {
                printf("User moved\n\r");
                sound::notifyLong();
                sound::notifyLong();
                sound::notifyLong();
                sound::notifyDenied();

            }

            lastMoved = presence::getMovedState();

            break;

        case PRESENCE_SLEEP:
            if (millis() - time > 5000) {
                presenceState = PRESENCE_UPDATE;
            }
            break;
    }
}

/*************************************************************************************************/

void loop()
{
    loop_presence();
    loop_button();
    loop_main();
}