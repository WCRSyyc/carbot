/*
 * Joystick controller for demonstration carbot
 *
 * This was written as a remote control for the WCRS carbot demonstration
 * chassis.  See nRF2401-carbot-remote.
 *
 * This contains the logic to turn the joystick x, y values into left and right
 * motor speed values, for differential steering.
 *
 * Updated: Dec 2016 by H Phil Duby
 *
 * Arduino UNO
 * RF2401 transceiver (through custom shield (protoshield))
 * Analog joystick (through custom shield (protoshield))
 */

#include <SPI.h>
#include "RF24.h"

// Analog input pins that the joystick potentiometers are connected to
const int analogInPinA0 = A0;  // x
const int analogInPinA1 = A1;  // y
const int buttonPin = 16;

struct speedSettings {
  int leftSpeed, rightSpeed;
};
const unsigned int packetSize = sizeof( speedSettings );

const int MIN_RAW_ADC = 0;
const int MAX_RAW_ADC = 1023; // 10 bits
const int MAX_SPEED_SETTING = 255; // Forward
const int MIN_SPEED_SETTING = -MAX_SPEED_SETTING; // Backward
const int MAX_TURN_DELTA = 63; // Left
const int MIN_TURN_DELTA = -MAX_TURN_DELTA; // Right

// (RF24) address of the remote carbot
const byte address[] = "rCBot";

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio ( 7, 8 );

const unsigned long PACKET_DELAY = 50; // milliseconds (0.05 seconds)
// Lower values give faster response
// IDEA use variable delay: send packet when values change
// No current need for a heartbeat to maintain a connection

speedSettings command;

int sensorValueX = 0;        // value read from the pot
int sensorValueY = 0;        // value read from the pot
int sensorValueZ = 0;        // value read from the button

void setup() {
  Serial.begin ( 115200 );
  Serial.print (F( "Carbot Joystick starting up" ));

  pinMode ( buttonPin, INPUT_PULLUP);

  initRemoteControl ();
}

void loop()
{
  getSensorData ();
  xyToDifferential();
  if ( !radio.write ( &command, packetSize )) {
    Serial.println (F( "tx error" )); // no ACK?
  }

  delay ( PACKET_DELAY );
}// ./void loop()


void getSensorData()
{
  sensorValueX = analogRead ( analogInPinA0 );
  sensorValueY = analogRead ( analogInPinA1 );
  sensorValueZ = digitalRead ( buttonPin );
} // ./getSensorData()


/**
 * Convert joystick x and y values into motor speed settings for differential
 * steering
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
  // set address size (len (address))

  // This is a remote control (ground) vehicle, which should never be out of the
  // users sight.  Reduce the radio power level, since only short range will
  // ever be needed.
  radio.setPALevel ( RF24_PA_LOW ); // RF24_PA_MAX is default

  // Currently one way communications, so only need a writing pipe
  radio.openWritingPipe ( address );

  // Start the radio listening for data
  radio.stopListening ();
}// ./void initRemoteControl ()
