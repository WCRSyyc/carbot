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

struct txMessStru{
  int buffType;
//  char messBuff[80];
   int x,y,z;
};

//txMessStru myMessage ={0, "abcdefghi"};
txMessStru myMessage;

int xL, xR;   // working value for motor speed Left + Right
int dL, dR;   // working value for motor direction Left + Right

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
//bool role = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24 receive string"));
  Serial.print(F("radioNumber is "));
  Serial.println(radioNumber);

  Serial.println("Adafruit Motorshield v2 - DC Motor test!");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  // Set the speed to start, from 0 (off) to 255 (max speed)
  //myMotor->setSpeed(150);
 // myMotor->run(FORWARD);
  // turn on motor
  myMotorL->run(RELEASE);
  myMotorR->run(RELEASE);
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();
}

void loop() {


/****************** Ping Out Role ***************************/  

 //radio.startListening();                                    // Now, continue listening
/*
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
*/
  
                                                                     // Variable for the received timestamp
   while (radio.available()) {                     // While there is data ready
      //Serial.println("available");
      radio.read( &myMessage, sizeof(txMessStru) );  // Get the payload one char only
     
      //Serial.println(myMessage.messBuff);
    
     // strncpy( myMessage.messBuff,"empty", 6);
   Serial.print("before turn bias: ");
   Serial.print(myMessage.x);
   Serial.print(", ");
   Serial.print(myMessage.y);
   Serial.print(", ");
   Serial.println(myMessage.z);

   xL = myMessage.y; // forward/back
   xR = myMessage.y;
   if((myMessage.y <= -10)&&(myMessage.y >= 10)){  // ignore dead zone y
     if(myMessage.y < 0){  //turn Left 
       myMessage.y = abs(myMessage.y);
       xL -= myMessage.y;
       xR += myMessage.y;
     }else if(myMessage.y > 0){  //turn Right  
      xL += myMessage.y;
      xR -= myMessage.y;
     }
     if (xL < -255) { xL = -255;}  // -255 through 255 is max
     if (xL > 255)  { xL = 255; }
     if (xR < -255) { xR = -255;}
     if (xR > 255)  { xR = 255; }
   }
    
   Serial.print("after turn bias: ");
   Serial.print(xL);
   Serial.print(", ");
   Serial.println(xR);
  
    
   // foward back after turn applied
   if((xL >= -10) && (xL <= 10)){
    myMotorL->run(RELEASE);
    myMotorR->run(RELEASE);
     Serial.println("Release");
 
  }else {  // set both motor speeds
     if (xL < 0){   //negative speed == reverse
        myMotorL->run(BACKWARD);
 
       Serial.print("Back L ");
       Serial.println(abs(xL));
       myMotorL->setSpeed(abs(xL)); 
     }
     if (xR < 0){
       //negative speed == reverse
        myMotorR->run(BACKWARD);
 
       Serial.print("Back R ");
       Serial.println(abs(xR));
       myMotorR->setSpeed(abs(xR));
     }
     if (xL > 0){
       myMotorL->run(FORWARD);
       Serial.print("Forward L ");
       Serial.println(xL);

       myMotorL->setSpeed(xL);
     }
      if (xR > 0){
       myMotorR->run(FORWARD);
       Serial.print("Forward R ");
       Serial.println(xR);

       myMotorR->setSpeed(xR); 
      }
  }  // end of else not dead so power motors
   //Serial.println(myMessage.messBuff);



   
   } // end of while radio avail
}    //loop     

  
