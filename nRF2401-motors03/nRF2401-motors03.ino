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

// motor controller port numbers where motors are physically attached
#define LEFT_MOTOR_PORT 1
#define RIGHT_MOTOR_PORT 3

#define ZERO_DEAD_BAND 10
#define MAX_STEP_DELTA 1
// minimum time (milliseconds) to wait beteen motor speed setting changes:
// smaller gives higher acceleration
#define CHANGE_WAIT 10
// MAX_STEP_DELTA and CHANGE_WAIT combine to give maximum acceleration.  Higher
// delta and/or smaller wait give higher (maximum) acceleration

#define MAX_MOTOR_SPEED 255
#define MIN_MOTOR_SPEED -255
// names for motor indexes
#define LEFT 0
#define RIGHT 1

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *myMotorL = AFMS.getMotor( LEFT_MOTOR_PORT );
Adafruit_DCMotor *myMotorR = AFMS.getMotor( RIGHT_MOTOR_PORT );


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

txMessStru joyStickData;
const unsigned int MOTOR_COUNT = 2;
int curSpeed[ MOTOR_COUNT ];
int targSpeed[ MOTOR_COUNT ];
unsigned long nextChangeTime;


byte addresses[][6] = {"1Node","2Node"};

void setup() {
  Serial.begin( 115200 );
  Serial.print(F( "\nWCRS carbot; RF245 radioNumber is " ));
  Serial.print( radioNumber );
  Serial.println(F( " using Adafruit Motorshield" ));

  AFMS.begin();  // create with the default frequency 1.6KHz

  myMotorL->setSpeed( curSpeed[ LEFT ]);
  myMotorR->setSpeed( curSpeed[ RIGHT ]);
  myMotorL->run( RELEASE );
  myMotorR->run( RELEASE );

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel( RF24_PA_LOW );

  // Currently one way communications, so only need a reading pipe
  radio.openReadingPipe( 1, addresses[ 0 ]);
  // Open a writing and reading pipe on each radio, with opposite addresses
//  if(radioNumber){
//    radio.openWritingPipe(addresses[1]);
//    radio.openReadingPipe(1,addresses[0]);
//  }else{
//    radio.openWritingPipe(addresses[0]);
//    radio.openReadingPipe(1,addresses[1]);
//  }

  // Start the radio listening for data
  radio.startListening();
  nextChangeTime = millis();
}

void loop() {
  unsigned long nowTime;
  getPacket(); // update control settings, whenerver new ones are available
  nowTime = millis();
  if ( nowTime < nextChangeTime ) {
    // Not time to change the settings yet
    return;
  }

  nextChangeTime = nowTime + CHANGE_WAIT;
//  Serial.print(F( "Change @ " ));
//  Serial.println( nowTime );
//  return;

  // The (non-turnning) target speed for both motors is the latest y joystick value
  targSpeed[ LEFT ] = joyStickData.y;
  targSpeed[ RIGHT ] = joyStickData.y;

  adjustForTurn(); // Adjust target speeds if turnning

//  Serial.print(F( "LEFT " ));
  curSpeed[ LEFT ]  = calcMotorSpeed ( curSpeed[ LEFT ],  targSpeed[ LEFT] );
//  Serial.print(F( "RIGHT " ));
  curSpeed[ RIGHT ] = calcMotorSpeed ( curSpeed[ RIGHT ], targSpeed[ RIGHT ]);

//  Serial.print(F( "new: L=" ));
//  Serial.print(curSpeed[ LEFT ]);
//  Serial.print(F( ", R=" ));
//  Serial.println(curSpeed[ RIGHT ]);

//  Serial.print(F( "LEFT " ));
  setMotorSpeed(myMotorL, curSpeed[LEFT]);
//  Serial.print(F( "RIGHT " ));
  setMotorSpeed(myMotorR, curSpeed[RIGHT]);
}// ./void loop()

/**
 * Whenever a new joystick values are available, update the latest settings
 */
void getPacket()
{
  if ( radio.available()) { // if another packet has arrived
    radio.read( &joyStickData, sizeof( txMessStru ));

//    Serial.print(F( "raw: X=" ));
//    Serial.print( joyStickData.x );
//    Serial.print(F( ", Y=" ));
//    Serial.print( joyStickData.y );
//    Serial.print(F( ", Z=" ));
//    Serial.print( joyStickData.z );
//    Serial.print(F( " @ " ));
//    Serial.println( millis());

    // Adjust (too) low readings to actual zero
    if ( abs( joyStickData.x ) < ZERO_DEAD_BAND ) {
      joyStickData.x = 0;
    }
    if ( abs( joyStickData.y ) < ZERO_DEAD_BAND ) {
      joyStickData.y = 0;
    }
  }
}// ./void getPacket()


/**
 * Adjust the individual target motor speeds to provide delta for turnning
 */
void adjustForTurn()
{
  // Negative values of x turn (more) left (slower left, faster right)
  targSpeed[ LEFT ] += joyStickData.x;
  targSpeed[ RIGHT ] -= joyStickData.x;
}// ./void adjustForTurn()


/**
 * Calculated a new motor speed setting closer to the target, with the change
 * by the maximum allowed acceleration (MAX_STEP_DELTA)
 *
 * @param cur the current motor speed setting
 * @param targ the target motor speed setting
 */
int calcMotorSpeed( int cur, int targ )
{
  int newSpeed;
//  Serial.print(F( "calc from cur= " ));
//  Serial.print( cur );
//  Serial.print(F( ", targ=" ));
//  Serial.print( targ );
//  Serial.print(F( ": " ));

  if ( targ < cur ) { // more negative speed
//    Serial.print(F( "targ < cur; " ));
    if ( cur - MAX_STEP_DELTA < targ ) { // delta less than max
      newSpeed = targ;
    } else {
      newSpeed = cur - MAX_STEP_DELTA; // limit change to max step
    }
  } else { // targ >= cur
//    Serial.print(F( "targ => cur; " ));
    if ( cur + MAX_STEP_DELTA > targ ) { // delta less than max
      newSpeed = targ;
    } else {
      newSpeed = cur + MAX_STEP_DELTA; // limit change to max step
    }
  }

  if ( newSpeed < MIN_MOTOR_SPEED) {
//    Serial.print(F( "min limit; " ));
    newSpeed = MIN_MOTOR_SPEED;
  }
  if ( newSpeed > MAX_MOTOR_SPEED) {
//    Serial.print(F( "max limit; " ));
    newSpeed = MAX_MOTOR_SPEED;
  }
//  Serial.print(F( "new =" ));
//  Serial.println( newSpeed );

  return newSpeed;
}// ./int calcMotorSpeed()


/**
 * Set the new speed for a motor
 *
 * @param aMotor motor to change speed setting on
 * @param speedSetting new motor speed (-255 to 255)
 */
void setMotorSpeed( Adafruit_DCMotor * aMotor, int speedSetting )
{
  if ( abs ( speedSetting ) < ZERO_DEAD_BAND ) {
    // In the speed dead band: stop, or stay stopped
    aMotor->setSpeed( 0 );
    aMotor->run( RELEASE );
//    Serial.println("STOP");
    return;
  }

  if ( speedSetting < 0 ) { // turn wheel backwards
    aMotor->run( BACKWARD );
    aMotor->setSpeed( -speedSetting );
//    Serial.print(F( "Back " ));
//    Serial.println( -speedSetting );
  } else { // turn wheel forward
    aMotor->run( FORWARD );
    aMotor->setSpeed( speedSetting );
//    Serial.print(F( "Forward " ));
//    Serial.println( speedSetting );
  }
}// ./void setMotorSpeed()
