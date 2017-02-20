# carbot
develop software for simple remote controlled then autonomous robot car

## Remote Control
Folders nRF2401-carbot-joystick and nRF2401-carbot-remote contain sketches to run (respectively) a joystick controller transmitter unit, and a 2 wheeled carbot receiver.  The joystick sketch translates X and Y position values into left and right motor speed (target) settings.  Negative speed values are used for reverse direction.  The speed values are sent through the nRF2401 radios to the car module.  There, when received, the target speed value are updated.  Wheter new values have been received or not, the sketch sketch 'steps' from the current (actual) speed settings toward the target.  This, combined with the acceleration and deceleration (change) wait time values limit how fast the speed can change.  This is used both the give a bit more realistic acceleration and braking, and to reduce the stress on the physical car unit.  Our initial prototype was built with materials that do not have a lot of structural strength.  Repeated too rapid change in speed risks tearing the motors from the chassis.  The remote sketch also includes a timeout, so that if the radio signal is lost, the car will stop.

### Older Remote Control
Folders nRF2401-Joy2 and nRF2401-motors03 contain sketches to run (respectively) a joystick controller transmitter unit, and a 2 wheeled carbot receiver.  In this version pair, the transmitter code sends the raw joystick X and Y position values, as well as the integrated push button switch state to the recieving carbot unit.  The motor controlling sketch interprets and translates those values into forward and back speed settings for the left and right motors.

## joystickTest
This sketch is a standalone program to display the changing values as the joystick on the shield is manipulated.  An easy way to verify that the shield is wired correctly, and what the orrientation is.  Figure out which direction is forward and back, versus left and right.  Verify that the readings change from 0 to full range for each axis.  If the joystick includes a switch on the button, verify what the pressed and released state values are.

## motor and speed control testing
DC4MotorTest is a simple motor speed cycling test sketch (to be) used to verify that the motors are connected to the correct motor shield ports, and that the forward speed settings correspond to the wheel forward direction.
