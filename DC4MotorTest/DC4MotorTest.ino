/*
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

Sequence through the motors, ramping speed from zero to forward maximum, then to
backward maximum, and back to zero.  Use to verify correct connections, and to
make sure the programmed forward direction is really the wheel foward direction.

For use with the Adafruit Motor Shield v2
---->	http://www.adafruit.com/products/1438
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Time delay for speed changes (10 milliseconds)
#define DELTA_SPEED_DELAY 10
// Time delay between motor tests (1 second)
#define MOTOR_CHANGE_DELAY 1000
// Maximum motor speed setting
#define MAXIMUM_SPEED 255

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4 us use for each motor
Adafruit_DCMotor *myMotor1 = AFMS.getMotor ( 1 );
Adafruit_DCMotor *myMotor2 = AFMS.getMotor ( 2 );
Adafruit_DCMotor *myMotor3 = AFMS.getMotor ( 3 );
Adafruit_DCMotor *myMotor4 = AFMS.getMotor ( 4 );

void setup() {
  Serial.begin ( 115200 );
  Serial.println ( F( "Adafruit Motorshield v2 - DC Motor test!" ));

  AFMS.begin();  // create with the default frequency 1.6KHz
  // AFMS.begin ( 1000 );  // OR with a different frequency, say 1KHz

  // Set the speed of all motor to 0 to start
  myMotor1 -> setSpeed ( 0 );
  myMotor2 -> setSpeed ( 0 );
  myMotor3 -> setSpeed ( 0 );
  myMotor4 -> setSpeed ( 0 );
  myMotor1 -> run ( FORWARD );
  myMotor2 -> run ( FORWARD );
  myMotor3 -> run ( FORWARD );
  myMotor4 -> run ( FORWARD );
  // turn off motor
  myMotor1 -> run ( RELEASE );
  myMotor2 -> run ( RELEASE );
  myMotor3 -> run ( RELEASE );
  myMotor4 -> run ( RELEASE );
}

void loop() {
  // Comment out all but the motors to be tested
  Serial.print ( F( "motor 1: " ));
  rampMotor ( myMotor1 );
//  Serial.print ( F( "motor 2: " ));
//  rampMotor ( myMotor2 );
  Serial.print ( F( "motor 3: " ));
  rampMotor ( myMotor3 );
//  Serial.print ( F( "motor 4: " ));
//  rampMotor ( myMotor4 );
  delay ( MOTOR_CHANGE_DELAY );
}

/**
 * Cycle motor speed from zero to + max, to - max, to zero.
 *
 * @param aMotor pointer to an Adafruit_DCMotor to cycle through speeds
 */
void rampMotor( Adafruit_DCMotor * aMotor )
{
  uint8_t i;
  Serial.print ( F( "tick " )); // report start of forward speed cycle

  aMotor -> run ( FORWARD );
  for ( i = 0; i < MAXIMUM_SPEED; i++ ) {
    aMotor -> setSpeed ( i );
    delay ( DELTA_SPEED_DELAY ); // acceleration limiter time delay
  }
  // Limit check must be "!=0" not "<0" because i is unsigned and is never < 0
  for ( i = MAXIMUM_SPEED; i != 0; i-- ) {
    aMotor -> setSpeed ( i );
    delay ( DELTA_SPEED_DELAY );
  }

  Serial.print ( F( "tock " )); // report start of backward speed cycle

  aMotor->run ( BACKWARD );
  for ( i = 0; i < MAXIMUM_SPEED; i++ ) {
    aMotor -> setSpeed ( i );
    delay ( DELTA_SPEED_DELAY );
  }
  for ( i = MAXIMUM_SPEED; i != 0; i-- ) {
    aMotor -> setSpeed ( i );
    delay ( DELTA_SPEED_DELAY );
  }

  Serial.println ( F( "tech" )); // report end of motor speed cycle
  aMotor -> run ( RELEASE );
}
