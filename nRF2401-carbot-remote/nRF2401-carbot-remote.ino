/*
 * Remote controlled demonstration car
 *
 * This takes speed setting commands from RF2401 packets to set the target speed
 * for left and right motors.  The actual motor speeds are adjusted towards the
 * target values over time, with the rate determined by the maximum acceleration
 * configuration settings.
 *
 * The car this was written for is 2 powered wheels plus a third idler for
 * balance, and drives with differential steering.
 *
 * Arduino UNO
 * Adafruit Motor Shield V2
 * RF2401 transceiver (through custom shield (protoshield))
 */

#include <SPI.h>
#include "RF24.h"

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// motor controller port numbers where motors are physically attached
#define LEFT_MOTOR_PORT 1
#define RIGHT_MOTOR_PORT 3

// The range around zero that is treated as actual zero speed setting
#define ZERO_DEAD_BAND 10
#define MAX_STEP_DELTA 1
// minimum time (milliseconds) to wait beteen motor speed setting changes:
// smaller gives higher acceleration
#define CHANGE_WAIT 10
// MAX_STEP_DELTA and CHANGE_WAIT combine to give maximum acceleration.  Higher
// delta and/or smaller wait give higher (maximum) acceleration

// NOTE: where loop timing constraints can be met, a MAX_STEP_DELTA of 1 gives
// the smoothest movement.  Adjust the CHANGE_WAIT for tuning acceleration.

// (Hard) limit (for motor controller) speed settings
#define MAX_MOTOR_SPEED 255
#define MIN_MOTOR_SPEED -255

// names for motor indexes
#define LEFT 0
#define RIGHT 1

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

/****************** User Config ***************************/

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio ( 7, 8 );

// Specify the motorshield ports the motors are connected to
Adafruit_DCMotor *myMotorL = AFMS.getMotor( LEFT_MOTOR_PORT );
Adafruit_DCMotor *myMotorR = AFMS.getMotor( RIGHT_MOTOR_PORT );
/**********************************************************/

struct speedSettings {
  int leftSpeed, rightSpeed;
};
const unsigned int packetSize = sizeof( speedSettings );
// speed range -255 to 255 (0-255 forward and backward)

speedSettings target;
const unsigned int MOTOR_COUNT = 2;
int curSpeed [ MOTOR_COUNT ];
unsigned long nextChangeTime;

byte address[] = "rCBot"; // remote control car bot

void setup() {
  Serial.begin ( 115200 ); // Debug and monitoring
  Serial.print (F( "\nWCRS RF24 remote control carbot using Adafruit Motorshield" ));

  activateMotorControl ();
  activateMotor ( myMotorL );
  activateMotor ( myMotorR );

  initRemoteControl ();

  curSpeed [ LEFT ] = 0; // Start up with motors stopped.
  curSpeed [ RIGHT ] = 0;
  nextChangeTime = millis();
}

void loop() {
  unsigned long nowTime;
  getTargets(); // update control settings, whenever new ones are available
  nowTime = millis();
  if ( nowTime < nextChangeTime ) {
    // Not time to change the settings yet
    return;
  }

  nextChangeTime = nowTime + CHANGE_WAIT;

  curSpeed[ LEFT ]  = calcMotorSpeed ( curSpeed[ LEFT ],  target.leftSpeed );
  curSpeed[ RIGHT ] = calcMotorSpeed ( curSpeed[ RIGHT ], target.rightSpeed );

//  Serial.print (F( "new: L=" ));
//  Serial.print ( curSpeed[ LEFT ]);
//  Serial.print (F( ", R=" ));
//  Serial.println ( curSpeed[ RIGHT ]);

  setMotorSpeed ( myMotorL, curSpeed [ LEFT ]);
  setMotorSpeed ( myMotorR, curSpeed [ RIGHT ]);
}// ./void loop()


/**
 * Calculate a new motor speed setting closer to the target, with the (single
 * step) change limited by the maximum allowed acceleration (MAX_STEP_DELTA)
 *
 * IDEA use the actual time since last speed change to better calculate the new speed
 * IDEA support higher deceleration than acceleration
 *
 * @param cur the current motor speed setting
 * @param targ the target motor speed setting
 * @return new motor speed setting
 */
int calcMotorSpeed( int cur, int targ )
{
  int newSpeed;

  if ( targ < cur ) {
    // speed is changing more negative
    if ( cur - MAX_STEP_DELTA <= targ ) { // delta not greater than max step
      newSpeed = targ;
    } else {
      newSpeed = cur - MAX_STEP_DELTA; // limit change to max step
    }
  } else { // targ >= cur
    // speed is changing more positive
    if ( cur + MAX_STEP_DELTA >= targ ) { // delta not greater than max step
      newSpeed = targ;
    } else {
      newSpeed = cur + MAX_STEP_DELTA; // limit change to max step
    }
  }

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
  // Adjust (too) low setting to actual zero
  if ( abs ( speedSetting ) < ZERO_DEAD_BAND ) {
    // In the speed dead band: stop, or stay stopped
    aMotor->setSpeed( 0 );
    aMotor->run( RELEASE );
    return;
  }

  if ( speedSetting < 0 ) { // turn wheel backwards
    aMotor->run( BACKWARD );
    aMotor->setSpeed( -speedSetting );
  } else { // turn wheel forward
    aMotor->run( FORWARD );
    aMotor->setSpeed( speedSetting );
  }
}// ./void setMotorSpeed()


/**
 * Whenever new control settings are available, update the target values
 */
void getTargets()
{
  if ( radio.available ()) { // another control packet has arrived
    radio.read( &target, packetSize );
//    Serial.print (F( "raw: Left=" ));
//    Serial.print ( target.leftSpeed );
//    Serial.print (F( ", Right=" ));
//    Serial.print ( target.rightSpeed );
//    Serial.print (F( " @ " ));
//    Serial.println ( millis());

    // Limit the target speed settings to the valid motor range
    target.leftSpeed = clipSpeed ( target.leftSpeed );
    target.rightSpeed = clipSpeed ( target.rightSpeed );
  }
}// ./void getTargets()


/**
 * Keep motor speeds in the valid range
 *
 * @param requestSpeed the requested speed setting
 * @return speed setting clipped to the MIN/MAX MOTOR_SPEED range
 */
int clipSpeed ( int requestSpeed )
{
  int targetSpeed = requestSpeed;
  if ( requestSpeed < MIN_MOTOR_SPEED ) {
    targetSpeed = MIN_MOTOR_SPEED;
  }
  if ( requestSpeed > MAX_MOTOR_SPEED ) {
    targetSpeed = MAX_MOTOR_SPEED;
  }

  return targetSpeed;
}
/**
 * initialize the adafruit motor shield V2
 */
void activateMotorControl ()
{
  // TODO create motor shield profile structure, and use instead of defaults
  AFMS.begin();  // create with the default frequency 1.6KHz
}// ./void activateMotorControl ()


/**
 * Prepare a motor on an Adafruit DC Motor shield for initial use
 *
 * @param aMotor the Adafruit_DCMotor to initialize
 */
void activateMotor ( Adafruit_DCMotor * aMotor )
{
  aMotor -> run ( FORWARD );
  aMotor -> setSpeed ( 0 );
  aMotor -> run ( RELEASE ); // Coast?
}// ./void activateMotor ()


/**
 * Setup remote control communications
 */
  void initRemoteControl ()
{
  radio.begin ();

  // TODO create a radio profile structure
  // radio channel
  // disable auto ack
  // set payload size (packetSize == sizeof (speedSettings))
  // set address size (len (address))

  // This is a remote control (ground) vehicle, which should never be out of the
  // users sight.  Reduce the radio power level, since only short range will
  // ever be needed.
  radio.setPALevel ( RF24_PA_LOW ); // RF24_PA_MAX is default

  // Currently one way communications, so only need a reading pipe
  radio.openReadingPipe ( 1, address ); // First reading pipe

  // Start the radio listening for data
  radio.startListening ();
}// ./void initRemoteControl ()
