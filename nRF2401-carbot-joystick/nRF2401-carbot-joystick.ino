/*
 * Demonstration Joystick controller for carbot
 *
 * This was written as a remote control for the WCRS carbot demonstration
 * chassis.  See nRF2401-carbot-remote.
 *
 * This contains the logic to turn the joystick x, y values into left and right
 * motor speed values, for differential steering.
 *
 * Updated: Mar 11 2017 by H Phil Duby
 *
 * Arduino UNO
 * RF2401 transceiver (through custom shield (protoshield))
 * Analog joystick (through custom shield (protoshield))
 */

#include <SPI.h>
#include "RF24.h"
// Only include some counting and reporting when need to debug something that is
// going wrong.  Uncomment the following line to turn on the extra code.
//#define DEBUG

// Analog input pins that the joystick potentiometers are connected to, plus
// flags to reverse the meaning of left, right and forward, back for the
// joystick.  The values in the following block should be all that needs to be
// changed to handle different joystick wiring. the axis pins need to be analog.
const int joystickXAxisPin = A0;  // joystick x (left and right)
const int joystickYAxisPin = A1;  // joystick y (forward and back)
const int joystickButtonPin = 16; // Digital pin matching A2; button on joystick
const int xInversion = 1; // 1 == normal; -1 = reverse left and right
const int yInversion = 1; // 1 == normal; -1 = reverse forward and back

// the structure of (but not the actual storage for) the information to send to
// the carbot unit.  Currently only the left and right motor speeds.
// NOTE: this structure must be the same as what the code running on the carbot
// expects.
struct speedSettings {
  int leftSpeed, rightSpeed;
};
// The combined size of the block of data to send: needed by the radio code
const unsigned int packetSize = sizeof( speedSettings );

// Constants for translating the raw analog measurments of the joystick into
// individual motor speed settings
const int MIN_RAW_ADC = 0;
const int MAX_RAW_ADC = 1023; // 10 bits; maximum value from analogRead
const int MAX_SPEED_SETTING = yInversion * 255; // Forward; maximum motor speed setting
const int MIN_SPEED_SETTING = -MAX_SPEED_SETTING; // Backward
const int MAX_TURN_DELTA = xInversion * 63; // maximum motor speed [in/de]crease while turning
const int MIN_TURN_DELTA = -MAX_TURN_DELTA; // Right

// (RF24) address of the remote carbot
const byte botAddress[] = "rCBot";

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio ( 7, 8 );

const unsigned long PACKET_DELAY = 50; // milliseconds (0.05 seconds)
// Lower values give faster response
// IDEA use variable delay: send packet when values change
// - would need to include a heart beat when the speed settings are not zero,
//   since the motor speed goes to zero if no signal has been received for awhile.

// Here is where the data for the carbot is actually stored, after the struct
// was defined above
speedSettings command;

int sensorValueX = 0;        // value read from the joystick pot
int sensorValueY = 0;        // value read from the joystick pot
int sensorButton = 0;        // value read from the joystick button
#ifdef DEBUG
int lpCnt = 0;
const int DISP_COUNT = 10;
#endif

void setup() {
#ifdef DEBUG
  Serial.begin ( 115200 );
  Serial.println (F( "Carbot Joystick starting up" ));
#endif

  pinMode ( joystickButtonPin, INPUT_PULLUP);

  initRemoteControl ();
}

void loop()
{
  getSensorData (); // get sensorValue X, Y, and Z
  xyToDifferential(); // calculate left and right motor speeds
  if ( !radio.write ( &command, packetSize )) { // Send to the carbot unit
    Serial.println (F( "tx error" )); // no ACK?
  }
#ifdef DEBUG
  lpCnt++;
  if ( lpCnt >= DISP_COUNT ) {
    lpCnt = 0;
    Serial.print (F( "Left=" ));
    Serial.print ( command.leftSpeed );
    Serial.print (F( ", Right=" ));
    Serial.println ( command.rightSpeed );
  }
#endif

  delay ( PACKET_DELAY ); // Wait a bit to not flood the radio channel with more
                          // information that the carbot can really handle
}// ./void loop()


/**
 * get information about the joystick x, y, and button positions
 *
 * This code is based on what the actual physical joystick is capable of
 * reporting.  In this case, the X and Y position, and whether the button is
 * pressed.
 *
 * @output global sensorValueX, sensorValueX, sensorButton
 */
void getSensorData()
{
  sensorValueX = analogRead ( joystickXAxisPin );
  sensorValueY = analogRead ( joystickYAxisPin );
  sensorButton = digitalRead ( joystickButtonPin );
} // ./getSensorData()

/**
 * Convert joystick x and y values into motor speed settings for differential
 * steering
 *
 * This is the code to change to adjust sensitivity of the joystick controls.
 * The initial map function just does a linear conversion of joystick position
 * motor speed setting.
 *
 * @output command structure for the carbot unit
 */
void xyToDifferential()
{
  int baseSpeed;
  int turnDelta;

  // The base forward/back speed setting
  baseSpeed = map ( sensorValueY, MIN_RAW_ADC, MAX_RAW_ADC,
    MIN_SPEED_SETTING, MAX_SPEED_SETTING );
  // The differential steering speed adjustment amount
  turnDelta = map ( sensorValueX, MIN_RAW_ADC, MAX_RAW_ADC,
    MIN_TURN_DELTA, MAX_TURN_DELTA );

  // Individual motor speed settings
  command.leftSpeed = baseSpeed + turnDelta;
  command.rightSpeed = baseSpeed - turnDelta;
}// ./void xyToDifferential()


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
  // set address size (len (botAddress))

  // This is a remote control (ground) vehicle, which should never be out of the
  // users sight.  Reduce the radio power level, since only short range will
  // ever be needed.
  radio.setPALevel ( RF24_PA_LOW ); // RF24_PA_MAX is default

  // Currently one way communications, so only need a writing pipe
  radio.openWritingPipe ( botAddress );

  // Start the radio listening for data
  radio.stopListening ();
}// ./void initRemoteControl ()
