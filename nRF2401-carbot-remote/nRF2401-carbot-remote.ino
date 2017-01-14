/*
 * Demonstartion remote controlled car control
 *
 * This was written as a simple speed control for the WCRS carbot, with software
 * acceleration limits.  The hardware platform for the demonstrator is not that
 * robust.  Jumping from zero to full speed could break the wheels off of the
 * chassis.
 *
 * The car this was written for is 2 powered wheels plus a third idler for
 * balance, and drives with differential steering.
 *
 * Updated: Jan 2017 by H Phil Duby
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
// The (maximum) size of a single speed change step: Should be left at 1
#define MAX_STEP_DELTA 1

// minimum time (milliseconds) to wait beteen motor speed setting changes:
// smaller gives higher acceleration
#define ACCELERATION_WAIT 10
#define DECELERATION_WAIT 5
// MAX_STEP_DELTA and «xx»CELERATION_WAIT combine to give maximum acceleration.
// Higher delta and/or smaller wait give higher (maximum) acceleration.

// NOTE: where loop timing constraints can be met, a MAX_STEP_DELTA of 1 gives
// the smoothest movement.  Adjust the «xx»CELERATION_WAIT values for tuning
// acceleration.

// (Hard) limit (for motor controller) speed settings
#define MAX_MOTOR_SPEED 255
#define MIN_MOTOR_SPEED -255

// The number of motors being controlled
const unsigned int MOTOR_COUNT = 2;
// names for motor speed array indexes
#define LEFT 0
#define RIGHT 1

// Our carbot (RF24) address (radio phone number)
// The remote control must send to this address
const byte myAddress[] = "rCBot";

// the structure of (but not the actual storage for) the information sent by
// the remote control unit.  Currently only the left and right motor speeds.
// NOTE: this structure must have the same fields as used in the controller.
// The names could be different, but the sizes better match.
struct speedSettings {
  int requestedSpeed [ MOTOR_COUNT ];
};
// The combined size of the block of data to receive: needed by the radio code
const unsigned int packetSize = sizeof( speedSettings );

// struct motorState {
//   Adafruit_DCMotor * motor
//   unsigned long lastChangeTime;
//   int currentSpeed;
//   bool isAccelerating;
// }

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

/****************** User Config ***************************/

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// NOTE: does NOT need to be the same as the remote control
RF24 radio ( 7, 8 );

// Specify the motor driver shield ports the motors are connected to
Adafruit_DCMotor *leftMotor = AFMS.getMotor( LEFT_MOTOR_PORT );
Adafruit_DCMotor *rightMotor = AFMS.getMotor( RIGHT_MOTOR_PORT );
/**********************************************************/

// Here is where the data for the carbot is actually stored, after the struct
// was defined above
speedSettings target;

/* ****** Storage for data that changes while the sketch is running ****** */

int curSpeed [ MOTOR_COUNT ]; // The current motor speed settings
unsigned long lastChangeTime; // The most recent time the settings were updated

// TRUE when accelerating; FALSE when decelerating or holding constant speed
bool accelerationMode;

unsigned long lastCommandTime;
// Time (milliseconds) to continue without new instructions before stopping
#define MAX_COMMAND_GAP 250

void setup() {
  Serial.begin ( 115200 ); // Debug and monitoring
  Serial.print (F( "\nWCRS RF24 remote control carbot using Adafruit Motorshield" ));

  // Prepare the motor controller and individual motors for use
  activateMotorControl ();
  activateMotor ( leftMotor );
  activateMotor ( rightMotor );

  initRemoteControl (); // Setup communications with remote

  curSpeed [ LEFT ] = 0; // Start up with motors stopped.
  curSpeed [ RIGHT ] = 0;
  lastChangeTime = millis();
  accelerationMode = true;
  lastCommandTime = lastChangeTime;
}

void loop() {
  // unsigned long nowTime;
  getTargets (); // update control settings, whenever new ones are available
  // nowTime = millis ();
  nextSpeed ( millis ());
}// ./void loop()


void nextSpeed( unsigned long nowTime )
{
  // unsigned long nowTime;
  unsigned long nextChangeTime;
  bool isAccel; // (will be) true when increasing speed

  // NOTE: put logic for [ac|dc]celeration timing here, so can change modes
  // quicker: make more resposinve to changing control inputs, without (much)
  // affecting configured overall acceleration performance.

  // Assumption is there should never have one wheel accerating while another
  // is decelerating.  *COULD* have one stopped and the other doing anything.
  isAccel =
    abs ( target.requestedSpeed [ LEFT  ]) > abs ( curSpeed [ LEFT ]) ||
    abs ( target.requestedSpeed [ RIGHT ]) > abs ( curSpeed [ RIGHT ]);

  if ( isAccel == accelerationMode ) { // same as previous
    // different timeout when accelerating and decelerating
    nextChangeTime = lastChangeTime +
      isAccel ? ACCELERATION_WAIT : DECELERATION_WAIT;
    if ( nowTime < nextChangeTime ) {
      // Not time to change the speed settings yet, keeping acceleration down
      return;
    }
  }// ./if ( isAccel == accelerationMode )

  // Either reached time to change speed, or acceleration direction changed

  accelerationMode = isAccel; // Keep track when [ac|dc]celerating
  curSpeed[ LEFT ]  = calcMotorSpeed ( curSpeed[ LEFT ],  target.requestedSpeed [ LEFT ]);
  curSpeed[ RIGHT ] = calcMotorSpeed ( curSpeed[ RIGHT ], target.requestedSpeed [ RIGHT ]);
  for ( int i = 0; i < MOTOR_COUNT; i++ ) {
    curSpeed[ i ] = calcMotorSpeed ( curSpeed[ i ], target.requestedSpeed [ i ]);
  }

  setMotorSpeed ( leftMotor, curSpeed [ LEFT ]);
  setMotorSpeed ( rightMotor, curSpeed [ RIGHT ]);
}// ./void nextSpeed()


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
  int calcSpeed;

  if ( targ < cur ) {
    // speed is changing more negative
    if ( cur - MAX_STEP_DELTA <= targ ) { // delta not greater than max step
      calcSpeed = targ; // speed is (now) at the target
    } else {
      calcSpeed = cur - MAX_STEP_DELTA; // limit change to max step
    }
  } else { // targ >= cur
    // speed is changing more positive
    if ( cur + MAX_STEP_DELTA >= targ ) { // delta not greater than max step
      calcSpeed = targ; // speed is (now) at the target
    } else {
      calcSpeed = cur + MAX_STEP_DELTA; // limit change to max step
    }
  }

  return calcSpeed;
}// ./int calcMotorSpeed()


/**
 * Set the new speed for a motor
 *
 * @param aMotor motor to change speed setting on
 * @param speedSetting new motor speed (-255 to 255)
 */
void setMotorSpeed( Adafruit_DCMotor * aMotor, int speedSetting )
{
  // Adjust (too) low setting to be actual zero
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
  unsigned long targetTime = millis ();
  if ( radio.available ()) { // another control packet has arrived
    radio.read( &target, packetSize );
//    Serial.print (F( "raw: Left=" ));
//    Serial.print ( target.requestedSpeed [ LEFT ]);
//    Serial.print (F( ", Right=" ));
//    Serial.print ( target.requestedSpeed [ RIGHT ]);
//    Serial.print (F( " @ " ));
//    Serial.println ( millis());

    // Limit the target speed settings to the valid motor range
    target.requestedSpeed [ LEFT ] = clipSpeed ( target.requestedSpeed [ LEFT ]);
    target.requestedSpeed [ RIGHT ] = clipSpeed ( target.requestedSpeed [ RIGHT]);
    lastCommandTime = targetTime; // Just saw a new command
  } else { // NOT ( radio.available ())
    // no new command this time
    if ( lastCommandTime + MAX_COMMAND_GAP < targetTime ) {
      // timeout: no command received for awhile: stop
      target.requestedSpeed [ LEFT ] = 0;
      target.requestedSpeed [ RIGHT ] = 0;
    }
  }
}// ./void getTargets()


/**
 * Keep motor speeds in the valid controller range
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
  // set address size (len (myAddress))

  // This is a remote control (ground) vehicle, which should never be out of the
  // users sight.  Reduce the radio power level, since only short range will
  // ever be needed.
  radio.setPALevel ( RF24_PA_LOW ); // RF24_PA_MAX is default

  // Currently one way communications, so only need a reading pipe
  radio.openReadingPipe ( 1, myAddress ); // First reading pipe

  // Start the radio listening for data
  radio.startListening ();
}// ./void initRemoteControl ()
