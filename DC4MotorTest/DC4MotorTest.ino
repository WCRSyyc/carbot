/*
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2
---->	http://www.adafruit.com/products/1438
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *myMotor1 = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(2);
Adafruit_DCMotor *myMotor3 = AFMS.getMotor(3);
Adafruit_DCMotor *myMotor4 = AFMS.getMotor(4);
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  myMotor1->setSpeed(0);
  myMotor2->setSpeed(0);
  myMotor3->setSpeed(0);
  myMotor4->setSpeed(0);
  myMotor1->run(FORWARD);
  myMotor2->run(FORWARD);
  myMotor3->run(FORWARD);
  myMotor4->run(FORWARD);
  // turn on motor
  myMotor1->run(RELEASE);
  myMotor2->run(RELEASE);
  myMotor3->run(RELEASE);
  myMotor4->run(RELEASE);
}

void loop() {
  Serial.print(F( "motor 1: " ));
  rampMotor ( myMotor1 );
//  Serial.print(F( "motor 2: " ));
//  rampMotor ( myMotor2 );
  Serial.print(F( "motor 3: " ));
  rampMotor ( myMotor3 );
//  Serial.print(F( "motor 4: " ));
//  rampMotor ( myMotor4 );
  delay(1000);
}

void rampMotor( Adafruit_DCMotor * aMotor )
{
  uint8_t i;
  Serial.print("tick");

  aMotor->run(FORWARD);
  for (i=0; i<255; i++) {
    aMotor->setSpeed(i);
    delay(10);
  }
  for (i=255; i!=0; i--) {
    aMotor->setSpeed(i);
    delay(10);
  }

  Serial.print("tock");

  aMotor->run(BACKWARD);
  for (i=0; i<255; i++) {
    aMotor->setSpeed(i);
    delay(10);
  }
  for (i=255; i!=0; i--) {
    aMotor->setSpeed(i);
    delay(10);
  }

  Serial.println("tech");
  aMotor->run(RELEASE);
}
