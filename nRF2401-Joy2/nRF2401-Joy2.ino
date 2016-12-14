/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

struct txMessStru{
  int buffType;
//  char messBuff[ 80 ];
  int x, y, z; 
};
const unsigned long PACKET_DELAY = 500; // milliseconds (0.5 seconds)

txMessStru myMessage;
int ptr;                    // index into message buffer
char workBuff[ 6 ];         // ascii to char work buffer

const int analogInPinA0 = A0;  // Analog input pin that the potentiometer is attached to
const int analogInPinA1 = A1;  // Analog input pin that the potentiometer is attached to
const int buttonPin = 16;

int sensorValueX = 0;        // value read from the pot
int sensorValueY = 0;        // value read from the pot
int sensorValueZ = 0;        // value read from the button
//int x, y, z;                 // mapped values for motors


/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio( 7, 8 );
/**********************************************************/

byte addresses[][ 6 ] = { "1Node", "2Node" };

void setup() {
  Serial.begin( 115200 );
  pinMode( 16, INPUT_PULLUP);
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
 
  radio.setPALevel(RF24_PA_LOW);

  // Open a pipe to write to the radio
  radio.openWritingPipe(addresses[0]);
  radio.stopListening();    
}

void loop() {
  getSensorData();
   //buildPacket();

  // Serial.println(myMessage.messBuff);

  if (!radio.write(&myMessage, sizeof(txMessStru))){
    Serial.println("tx error");
  }
 
  delay( PACKET_DELAY );
}

void getSensorData()
{
  sensorValueX = analogRead( analogInPinA0 );
  sensorValueY = analogRead( analogInPinA1 );
  sensorValueZ = digitalRead( 16 );

  myMessage.x = map( sensorValueX, 0, 1023, -255, 255 );
  myMessage.y = map( sensorValueY, 0, 1023, 255, -255 );
  myMessage.z = map( sensorValueZ, 0, 1, 1, 0 );
} // ./getSensorData()

/*
void buildPacket()
{
  ptr = 0;
  buffAppend( x );
  buffAppend( y );
  buffAppend( z );
  myMessage.messBuff[ ptr - 1 ] = 0;// clear trailing comma
}


void buffAppend( const int value )
{
  itoa( value, workBuff, 10);
  strncpy( &myMessage.messBuff[ ptr ], workBuff, strlen( workBuff ));
  ptr += strlen( workBuff );
  myMessage.messBuff[ ptr ] = ',';
  ptr++;
}

*/
