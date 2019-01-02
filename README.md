# carbot
Develop software for simple remote controlled then autonomous robot car

This has been switched to a wrapper repository.  The actual sketches are each in their own repository.  This includes them as submodules, and adds extra information about using them.

## Remote Control
Folders nRF2401-carbot-joystick and nRF2401-carbot-remote contain sketches to run (respectively) a joystick controller transmitter unit, and a 2 wheeled carbot receiver.

### nRF2401-carbot-joystick
The joystick sketch translates X and Y joystick position values into left and right [differential steering](https://github.com/WCRSyyc/carbot/wiki/differential-steering) motor speed (target) settings.  Negative speed values are used for reverse direction.  The speed values are sent through the nRF2401 radios to the car module.

This sketch is intended to be very simple and still flexible.  The top of the sketch (just after the library include statements) has a set of 5 lines that control the hardware configuration.  These lines specify the Arduino pin numbers that are wired to the joystick sense lines.  The X and Y joystick inputs need to be connected to [analog input pins](https://github.com/WCRSyyc/carbot/wiki/arduino-pin-numbers).  Any buttons on the joystick module only need to use a digital pin, but an analog pin can also be used as a digital input.  Our base sketch and prototype joystick shield use analog pins 0, 1, and 2.  A0 and A1 are the joystick X (left - right) and Y (forward - back) inputs.  A2 is connected to the single joystick button.  When analog pins are being treated as additional digital pins, they are numbered from 14, so A2 is digital pin 16.  The current sketch does not do anything with the button state, but it is being read and stored for use in later versions.  Changing the joystick pin values changes where the sketch gets the readings from.

The type of joystick used for this project is a pair of [potentiometers](https://github.com/WCRSyyc/ardx/wiki/potentiometer) wired to form 2 independent [voltage dividers](https://github.com/WCRSyyc/ardx/wiki/voltage-divider).  Depending which end of each potentiometer is connected to ground and 5 volts, the direction the knob is move to get increasing values (voltage readings) will be reversed.  The axis inversion constants compensate for this.  If higher analog values are read when pulling back on the knob, change yInversion from 1 to -1.  If higher values are read when pushing the knob to the left, change xInversion from 1 to -1.

To make the remote unit easier to steer, the maximum speed difference between the left and right motors has been deliberately limited.  The MAX_TURN_DELTA value controls the sensitivity.  To allow faster turns, increase the multiplier (initially 63).  The best value here is dependant on both personal choice, and on the physical capabilities of the remote unit.  If the motors are slow, more sensitive (higher value) steering works fine.  But when they are fast, the extra sensitivity can make steering (a lot) more challenging.

### nRF2401-carbot-remote
This sketch receives the motor speed settings and uses them to update the target speed values.  Whether new values have been received or not, the sketch sketch 'steps' from the current (actual) speed settings toward the target.  This, combined with the acceleration and deceleration (change) wait time values, limit how fast the speed can change.  This is used both to give a bit more realistic acceleration and braking, and to reduce the stress on the physical car unit.  Our initial prototype was built with materials that do not have a lot of structural strength.  Repeated, too rapid change in speed risks tearing the motors from the chassis.  The remote sketch also includes a timeout, so that if the radio signal is lost, the car will stop.

The sketch implements all of the physics to manage the acceleration properties of the remote unit.  It is intended for a 2 wheeled robot with [differential steering](https://github.com/WCRSyyc/carbot/wiki/differential-steering).  The inputs are the desired left and right motor speeds.  The sketch adjust the actual speed values towards those values, deliberately limited by the simulation mass and motor power.

The current acceleration logic is very simplified.  It is using a fixed (maximum) change in the motor power settings per step.  DC motors have maximum torque at zero speed.  To implement constant acceleration, different stepping logic is needed, with the smallest (or slowest changing) steps near zero speed, and larger steps as the speed increases.

Next step beyond that would be to define an engine `power curve`, then simulate that based on the actual motor, wheel, and chasis characteristics.

### Older Remote Control
Folders nRF2401-Joy2 and nRF2401-motors03 contain sketches to run (respectively) a joystick controller transmitter unit, and a 2 wheeled carbot receiver.  In this version pair, the transmitter code sends the raw joystick X and Y position values, as well as the integrated push button switch state to the receiving carbot unit.  The motor controlling sketch interprets and translates those values into forward and back speed settings for the left and right motors.

## Joystick shield testing
joystickTest is a standalone sketch to display the changing values as the joystick on the shield is manipulated.  An easy way to verify that the shield is wired correctly, and what the orientation is.  Figure out which direction is forward and back, versus left and right.  Verify that the readings change from 0 to full range for each axis.  If the joystick includes a switch on the button, verify what the pressed and released state values are.

## motor and speed control testing
DC4MotorTest is a simple motor speed cycling test sketch (to be) used to verify that the motors are connected to the correct motor shield ports, and that the forward speed settings correspond to the wheel forward direction.
