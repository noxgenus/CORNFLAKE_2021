/*
=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - SINGLE HAND MINI TRANSMITTER
=========================================================================
-------------------------------------------------------------------------
CORNFLAKE SINGLE HAND TRANSMITTER BOX V2.1
-------------------------------------------------------------------------

CONTROL CHANNEL: HC-12 #1 CHANNEL = C044
TELEMETRY CHANNEL: HC-12 #2 CHANNEL = C066

*/

#include <SoftwareSerial.h>

SoftwareSerial SSerial(3, 2); // RX, TX

// ANALOG INPUTS
const int joypin1 = A0; //Joystick X1
const int joypin2 = A1; //Joystick Y1

// JOYSTICK BUTTON PRESS LIGHT CONTROL
int count = 0;
int buttonState = 0; 
int lastButtonState = 0;       

// MODE SWITCH
int modecount = 0;
int modeState = 0; 
int lastModeState = 0;  

int servo;
int angle;

void setup() {
  
Serial.begin(9600);
SSerial.begin(9600);

pinMode(4, INPUT);
digitalWrite(4, 1);

pinMode(5, INPUT);
digitalWrite(5, 1);
}

void loop(){

  //READ ANALOG INPUTS TO INT
  int val1 = analogRead(joypin1);
  int val2 = analogRead(joypin2);

  //MAP ANALOG JOYSTICK VALS TO SERVO VALS (0-90-180)
  val1 = map(val1, 0, 1023, 0, 180);
  val2 = map(val2, 0, 1023, 0, 180);

  // BUTTON PRESS COUNTER FOR LIGHTS  
  int buttonState = digitalRead(4);

  if (buttonState != lastButtonState) {
       if (buttonState != 1) {    
          count++;  
          if (count >= 5) {
            count = 0;
          }
        } else {
          // nothing
        }
  }
  lastButtonState = buttonState;

      // 0- off
      // 1- lights half
      // 2- lights full
      // 3- police lights
      // 4- police + headlights
    
      if (count == 0) {
               move(6, 0);
               Serial.println("lights off");
          } else if (count == 1) {
               move(6, 1);
               Serial.println("lights half");
          } else if (count == 2) {
               move(6, 2);
               Serial.println("lights full");
          } else if (count == 3) {
               move(6, 3);
               Serial.println("lights fuller");
          } else if (count == 4) {
               move(6, 4);
               Serial.println("lights fullest");
       }



// BUTTON PRESS MODE SWITCH
  int modeState = digitalRead(5);

  if (modeState != lastModeState) {
       if (modeState != 1) {    
          modecount++;  
          if (modecount >= 2) {
            modecount = 0;
          }
        } else {
          // nothing
        }
  }
  
  lastModeState = modeState;

      // 0- MOTOR MODE
      // 1- PAN/TILT MODE
//
//          if (modecount == 0) {
//                // MOTOR MODE
//                move(1, val1);
//                move(2, val2);
//                move(3, 90); // hold pan
//                move(4, 90); // hold tilt
//                  Serial.println("motor control");
//      } else if (modecount == 1) {
//                 // PAN-TILT MODE
//                 move(1, 90); // hold drive
//                 move(2, 90); // hold turn
//                 move(3, val1);
//                 move(4, val2);
//                  Serial.println("pan/tilt control");
//      }

      
      if (modecount == 0) {
                // MOTOR MODE
                move(7, 0);
                  Serial.println("motor control");
      } else if (modecount == 1) {
                // PAN-TILT MODE
                move(7, 1);
                  Serial.println("pan/tilt control");
      }


     move(1, val1);
     move(2, val2);
       

// Short delay else loop goes bonkers
  delay(40);
}

//MOVE FUNCTION AND SEND TO TX
void move(int servo, int angle) {
       SSerial.write(char(255));
       SSerial.write(char(servo));
       SSerial.write(char(angle));
}

