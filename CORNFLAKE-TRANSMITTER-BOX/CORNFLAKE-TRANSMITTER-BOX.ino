/*
=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - MULTI JOYSTICK CONTROLLER BOX
=========================================================================
-------------------------------------------------------------------------
CORNFLAKE TRANSMITTER BOX V2.0
-------------------------------------------------------------------------
Hardware used for this sketch:

- Nano V3
- 2X HC-12 433MHz Serial Tranciever
- APA102 Leds (Adafruit Dotstar)
- Analog Joystick (Servocity)
- Adafruit Analog joypad
- Push buttons
- 10k pots

CONTROL CHANNEL: HC-12 #1 CHANNEL = C044
TELEMETRY CHANNEL: HC-12 #2 CHANNEL = C066

HC-12 SET CHANNEL: AT+C044

*/

#include <SoftwareSerial.h>
SoftwareSerial SSerial(3, 2); // RX, TX

#include <Adafruit_DotStar.h>
#define NUMPIXELS 4 //10 front - 6 rear
#define DATAPIN    8
#define CLOCKPIN   9
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// COLOR SETS

uint32_t colorwhite = 0xFFFFFF;
uint32_t colorred = 0xFF0000;
uint32_t colorredoff = 0x300000;
uint32_t colorblue = 0x0000FF;
uint32_t colorblueoff = 0x000039;
uint32_t coloryellow = 0xFFFF00;
uint32_t coloryellowoff = 0x302d00;
uint32_t colorgreen = 0x00dc1f;
uint32_t coloroff = 0x000000;

// ANALOG INPUTS

// Joystick
const int joypin1 = A0; //Joystick X1
const int joypin2 = A1; //Joystick Y1

// Joypad
const int joypin3 = A2; //Joystick X1
const int joypin4 = A3; //Joystick Y1

// Top dial
const int joypin5 = A4; //Joystick Y2

// DOF POTS
const int dofpin1 = A5; // POT
const int dofpin2 = A6; // POT
const int dofpin3 = A7; // POT


// Buttons
const int lightButton = 4;
const int modeButton = 5;
const int thirdButton = 6;
const int fireButton = 7;


// JOYSTICK BUTTON PRESS LIGHT CONTROL
int count = 0;
int buttonState = 0; 
int lastButtonState = 0;  

// JOYSTICK BUTTON PRESS MODE CONTROL
//int count1 = 0;
//int buttonState1 = 0; 
//int lastButtonState1 = 0; 

// JOYSTICK BUTTON PRESS 3rd CONTROL
int count2 = 0;
int buttonState2 = 0; 
int lastButtonState2 = 0; 

// JOYSTICK BUTTON TOP JOYSTICK FIRE CONTROL
int count3 = 0;
int buttonState3 = 0; 
int lastButtonState3 = 0; 

// MODE SWITCH
int modecount = 0;
int modeState = 0; 
int lastModeState = 0;  

// SERVO MODE SWITCH
int servomodecount = 0;
int servomodeState = 0; 
int servolastModeState = 0;  

int servo;
int angle;

void setup() {
  
    Serial.begin(9600);
    SSerial.begin(9600);
    
    pinMode(lightButton, INPUT);
    digitalWrite(lightButton, 1);
    
    pinMode(modeButton, INPUT);
    digitalWrite(modeButton, 1);
    
    pinMode(thirdButton, INPUT);
    digitalWrite(thirdButton, 1);
    
    pinMode(fireButton, INPUT);
    digitalWrite(fireButton, 1);

    strip.begin();
    //strip.setBrightness(64); //set brightness low
    strip.show();

    strip.setPixelColor(0, coloroff);
    strip.setPixelColor(1, coloroff);
    strip.setPixelColor(2, coloroff);
    strip.setPixelColor(3, colorgreen);
    strip.show();
    delay(200);
    strip.setPixelColor(0, coloroff);
    strip.setPixelColor(1, coloroff);
    strip.setPixelColor(2, colorgreen);
    strip.setPixelColor(3, colorgreen);
    strip.show();
    delay(200);
    strip.setPixelColor(0, coloroff);
    strip.setPixelColor(1, colorgreen);
    strip.setPixelColor(2, colorgreen);
    strip.setPixelColor(3, colorgreen);
    strip.show();
    delay(200);
    strip.setPixelColor(0, colorgreen);
    strip.setPixelColor(1, colorgreen);
    strip.setPixelColor(2, colorgreen);
    strip.setPixelColor(3, colorgreen);
    strip.show();
    delay(500);
}

void loop(){

//------------------------------------------------------------
  //READ ANALOG INPUTS TO INT
  int val1 = analogRead(joypin1);
  int val2 = analogRead(joypin2);
  int val3 = analogRead(joypin3);
  int val4 = analogRead(joypin4);
  int val5 = analogRead(joypin5);

  int val6 = analogRead(dofpin1);
  int val7 = analogRead(dofpin2);
  int val8 = analogRead(dofpin3);

//  Serial.print("VAL1: ");
//  Serial.print(val1);
//  Serial.print(" VAL2: ");
//  Serial.print(val2);
//  Serial.print(" VAL3: ");
//  Serial.print(val3);
//  Serial.print(" VAL4: ");
//  Serial.print(val4);
//  Serial.print(" VAL5: ");
//  Serial.println(val5);

  //MAP ANALOG JOYSTICK VALS TO SERVO VALS (0-90-180)
  val1 = map(val1, 0, 1023, 180, 0); //reverse pot
  val2 = map(val2, 0, 1023, 180, 0); //reverse pot
  val3 = map(val3, 0, 1023, 0, 180);
  val4 = map(val4, 0, 1023, 0, 180);
  val5 = map(val5, 0, 1023, 0, 180);

  val6 = map(val6, 0, 1023, 0, 180);
  val7 = map(val7, 0, 1023, 0, 180);
  val8 = map(val8, 0, 1023, 0, 180);


//------------------------------------------------------------
  // BUTTON PRESS COUNTER FOR LIGHTS  
  int buttonState = digitalRead(lightButton);

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

      if (count == 0) {
               move(6, 0);
                strip.setPixelColor(3, coloroff);
               //Serial.println("lights off");
          } else if (count == 1) {
               move(6, 1);
                strip.setPixelColor(3, colorgreen);
               //Serial.println("lights half");
          } else if (count == 2) {
               move(6, 2);
                strip.setPixelColor(3, colorblue);
               //Serial.println("lights full");
          } else if (count == 3) {
               move(6, 3);
                strip.setPixelColor(3, coloryellow);
               //Serial.println("lights fuller");
          } else if (count == 4) {
               move(6, 4);
                strip.setPixelColor(3, colorred);
               //Serial.println("lights fullest");
       }


//------------------------------------------------------------
// BUTTON PRESS MODE JOYSTICK SWITCH (BLUE)
  int modeState = digitalRead(modeButton);

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

//SWAP FUNCTIONALITY JOYSTICK WITH JOYPAD (YELLOW)
    
      if (modecount == 0) {
        
                move(1, val1);
                move(2, val2);
                move(3, val3);
                move(4, val4);
                move(5, val5);

                move(10, val6);
                move(11, val7);
                move(12, val8);
                  strip.setPixelColor(2, colorgreen);
                //Serial.println("MODE 1");
                  
      } else if (modecount == 1) {
          
                move(1, val3);
                move(2, val4);
                move(3, val1);
                move(4, val2);
                move(5, val5);

                move(10, val6);
                move(11, val7);
                move(12, val8);
                  strip.setPixelColor(2, colorwhite);
                //Serial.println("MODE 2");
      }



// BUTTON PRESS FOR SERVO MODE (RED)

  int servomodeState = digitalRead(thirdButton);

       if (servomodeState != servolastModeState) {
       if (servomodeState != 1) {    
          servomodecount++;  
          if (servomodecount >= 2) {
            servomodecount = 0;
            }
          } else {
          // nothing
          }
        }

  servolastModeState = servomodeState;

        if (servomodecount == 0) {
              move(7, 0);
              strip.setPixelColor(1, colorredoff);
          } else if (servomodecount == 1) {
              move(7, 1);
              strip.setPixelColor(1, colorred);
        }


// BUTTON ON TOP JOYSTICK PRESS FOR FIRE

  int buttonState3 = digitalRead(fireButton);

       if (buttonState3 != 1) {    
          move(8, 1);
            strip.setPixelColor(0, colorblue);
       } else if (buttonState3 != 0){
          move(8, 0);
            strip.setPixelColor(0, colorblueoff);
       }


 strip.show();
 
// Short delay else loop goes bonkers
  delay(20);
}


//------------------------------------------------------------
//MOVE FUNCTION AND SEND TO TX
void move(int servo, int angle) {
       SSerial.write(char(255));
       SSerial.write(char(servo));
       SSerial.write(char(angle));
}

