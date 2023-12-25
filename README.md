Pomodoro Timer
--------------

This is simple Pomodoro timer which uses [M5Stack ATOM S3](https://shop.m5stack.com/products/atoms3-dev-kit-w-0-85-inch-screen) dev board. There is
also support for use buzzer to make small sound notify.


Modes
-----

Pomodoro timer has different modes.

1. `Sleep`
    - **Short click**: Goto `Timer` mode
2. `Timer`
    - **Long click**: Goto `Sleep` mode
3. `Notify`
    - **Short click**: Goto `Timer` Mode
    - **Long click**:
        1) First time: Mutes click sound
        2) Second time: Goto `Sleep` mode


Desktop integration
-------------------

There is simple python script which monitors mouse movement. 

If mouse is moved script send message via BLE to timer to start Timer 
if it is in `Sleep` mode