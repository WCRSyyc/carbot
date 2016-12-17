/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

#define MAX_MOTOR_SPEED 255
#define MIN_MOTOR_SPEED -255
#define MAX_MOTOR_STEP 10
#define MIN_SPEED 10
// names for motor indexes
#define LEFT 0
#define RIGHT 1

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *myMotorL = AFMS.getMotor(1);
// You can also make another motor on port M2 or M3 or M4
Adafruit_DCMotor *myMotorR = AFMS.getMotor(3);


/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

struct txMessStru {
  int buffType;
   int x,y,z;
};
// x and y can be in the range -255 to 255
// z is zero or one

txMessStru myMessage;
const unsigned int MOTOR_COUNT = 2;
int curSpeed[MOTOR_COUNT];
int targSpeed[MOTOR_COUNT];

byte addresses[][6] = {"1Node","2Node"};

void setup() {
  Serial.begin(115200);
  Serial.print(F("WCRS carbot; RF245 radioNumber is "));
  Serial.print(radioNumber);
  Serial.println(" using Adafruit Motorshield");

  AFMS.begin();  // create with the default frequency 1.6KHz

  myMotorL->setSpeed(curSpeed[LEFT]);
  myMotorR->setSpeed(curSpeed[RIGHT]);
  myMotorL->run(RELEASE);
  myMotorR->run(RELEASE);

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
//  if(radioNumber){
//    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
//  }else{
//    radio.openWritingPipe(addresses[0]);
//    radio.openReadingPipe(1,addresses[1]);
//  }

  // Start the radio listening for data
  radio.startListening();
}

void loop() {
  while (radio.available()) {                     // While there is data ready
    //Serial.println("available");
    radio.read( &myMessage, sizeof(txMessStru) );  // Get the payload one char only

    Serial.print("raw: ");
    Serial.print(myMessage.x);
    Serial.print(", ");
    Serial.print(myMessage.y);
    Serial.print(", ");
    Serial.println(myMessage.z);

    targSpeed[LEFT] = myMessage.y;
    targSpeed[RIGHT] = myMessage.y;

    calcTurnSpeed();

    curSpeed[ LEFT ]  = calcMotorSpeed ( curSpeed[ LEFT ],  targSpeed[ LEFT] );
    curSpeed[ RIGHT ] = calcMotorSpeed ( curSpeed[ RIGHT ], targSpeed[ RIGHT ]);

    Serial.print("new: L=");
    Serial.print(curSpeed[ LEFT ]);
    Serial.print(", R=");
    Serial.println(curSpeed[ RIGHT ]);

    Serial.print("LEFT ");
    setMotorSpeed(myMotorL, curSpeed[LEFT]);
    Serial.print("RIGHT ");
    setMotorSpeed(myMotorR, curSpeed[RIGHT]);
  } // end of while radio avail
}    //loop     

void calcTurnSpeed()
{
  // adjust targSpeed left and right from myMessage.x
}

// use current and target speeds to get a new speed value to set
int calcMotorSpeed( int cur, int targ )
{
  int newSpeed;
  Serial.print("calc from cur= ");
  Serial.print(cur);
  Serial.print(", targ=");
  Serial.println(targ);
  if ( targ < cur ) {
    Serial.println("targ < cur");
    if ( cur - MAX_MOTOR_STEP < targ ) {
      newSpeed = targ;
    } else {
      newSpeed = cur - MAX_MOTOR_STEP;
    }
  } else { // targ >= cur
    Serial.println("targ > cur");
    if ( cur + MAX_MOTOR_STEP > targ ) {
      newSpeed = targ;
    } else {
      newSpeed = cur - MAX_MOTOR_STEP;
    }
  }
  if ( newSpeed < MIN_MOTOR_SPEED) {
    Serial.println("min limit");
    newSpeed = MIN_MOTOR_SPEED;
  }
  if ( newSpeed > MAX_MOTOR_SPEED) {
    Serial.println("max limit");
    newSpeed = MAX_MOTOR_SPEED;
  }
  Serial.print("new =");
  Serial.println(newSpeed);
  
  return newSpeed;
}// ./int calcMotorSpeed()

void setMotorSpeed( Adafruit_DCMotor * aMotor, int speedSetting  )
{
  if ( abs ( speedSetting ) < MIN_SPEED ) {
    aMotor->setSpeed( 0 ); 
    aMotor->run(RELEASE);
    Serial.println("STOP");
    return;
  }

  if ( speedSetting < 0) {   //negative speed == reverse
    aMotor->run( BACKWARD );
    aMotor->setSpeed( abs( speedSetting )); 
    Serial.print("Back ");
    Serial.println( speedSetting );
  } else {
    aMotor->run(FORWARD);
    aMotor->setSpeed(speedSetting); 
    Serial.print("Forward ");
    Serial.println( speedSetting );
   }
}  

