/*
 
=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - HEXAPOD INSTRUCTOR (INTERBOTIX)
=========================================================================
-------------------------------------------------------------------------
Custom communication translator from Interbotix protocol to 
simplified Cornflake. Original Hexapod hardware and software stay intact.
Remove Xbee and connect 3.3V RX from Interbotix board to 32bit ARM board.
-------------------------------------------------------------------------

*/
#include <Adafruit_DotStar.h>
#define NUMPIXELS 4
#define DATAPIN    51
#define CLOCKPIN   50
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// LED SHIT
long timee = 0; // timer

int pulseLed = 500;
int pulseAlive = 1500;
int RGBwarning = 20;
int RGBalive = 20;

// COLOR SETS

uint32_t colorwhite = 0xFFFFFF;
uint32_t colorred = 0xFF0000;
uint32_t colorredoff = 0x300000;
uint32_t colorblue = 0x0000FF;
uint32_t colorblueoff = 0x000039;
uint32_t coloryellow = 0xFFFF00;
uint32_t colorgreen = 0x00dc1f;
uint32_t coloroff = 0x000000;

// HC12 SERIAL INPUT SERVO/VALUE
int userInput[3];    // raw input from serial buffer, 3 bytes
int startbyte;       // start byte, begin reading input
int servo;           // which servo to pulse?
int pos;             // servo angle 0-180
int i;               // iterator

// Serial timeout on lost connection
const long timeout = 2000;
long counter = 0;

// DEFAULT CYCLE BUTTONS STUFF
int walk = 0;
long countergait = 0;
int gait = 1;
long counterspeed = 0;
int doublespeed = 1;

// Debug on/off
boolean debug = false;

// Hexa Button Array
int buttonState[] ={0,0,0,0,0,0,0,0}; //Array to hold the current state of each button, 0=unpushed, 1=pushed
int allButtons;

/* PHOENIX BUTTON MAP
 *
buttonState[0] = 0; // R1 walk gait / change leg
buttonState[1] = 0; // R2 toggle walk method
buttonState[2] = 0; // R3 nothing?
buttonState[3] = 0; // L4 Balance mode on and off
buttonState[4] = 0; // L5 stand/sit
buttonState[5] = 0; // L6 Right Joy UP/DOWN - Body up/down - Right Joy Left/Right - Speed higher/lower
buttonState[6] = 0; // Right Top(S7) Cycle Normal walk/Double Height/Double Travel
buttonState[7] = 0; // Left Top(S8) Cycle through modes (Walk, Translate, Rotate, Single Leg)

                            
 */

// Hexa Movement
int right_V = 128;   // Vertical position of the right joystick. 0 = All the way down, 127 = centered, 255 = all the way up
int right_H = 128;   // Horizontal position of the right joystick. 0 = All the way left, 127 = centered, 255 = all the way right
int left_H = 128;    // Vertical position of the right joystick. 0 = All the way down, 127 = centered, 255 = all the way up
int left_V = 128;    // Horizontal position of the right joystick. 0 = All the way left, 127 = centered, 255 = all the way right



void setup() {

    Serial.begin(9600); // Debug
    Serial1.begin(9600); // HC-12 RX
    Serial2.begin(38400); // HEXA TX
    
    allButtons = 0;
 
    if (debug == true){Serial.println("HEXAPOD INSTRUCTOR!");}

    strip.begin();
    strip.show();

    strip.setPixelColor(0, coloryellow);
    strip.show();
    delay(250);
    strip.setPixelColor(0, coloryellow);
    strip.setPixelColor(1, coloryellow);
    strip.show();
    delay(250);
    strip.setPixelColor(0, coloryellow);
    strip.setPixelColor(1, coloryellow);
    strip.setPixelColor(2, coloryellow);
    strip.show();
    delay(250);
    strip.setPixelColor(0, coloryellow);
    strip.setPixelColor(1, coloryellow);
    strip.setPixelColor(2, coloryellow);
    strip.setPixelColor(3, coloryellow);
    strip.show();
    delay(250);

    // Wait for other stuff to boot
    
} 

void loop() {

  // Led pulsing
  timee = millis();
  RGBwarning = 128+127*cos(2*PI/pulseLed*timee);
  RGBalive = 128+127*cos(2*PI/pulseAlive*timee);

 // Serial safety - Full stop on lost connection of controller.
     counter = counter + 1;
      if (counter >= timeout){
          //if (debug == true){Serial.println("No connection!");}
          int right_V = 128;
          int right_H = 128;
          int left_H = 128;
          int left_V = 128;

              strip.setPixelColor(0, 0,0,RGBwarning);
              strip.setPixelColor(1, 0,0,RGBwarning);
              strip.setPixelColor(2, 0,0,RGBwarning);
              strip.setPixelColor(3, 0,0,RGBwarning);
       }
        


// Start reading HC-12

  if (Serial1.available() > 2) {

       startbyte = Serial1.read();
       counter = 0; // Reset connection timeout counter
    
       if (startbyte == 255) {
         for (i=0;i<2;i++){ userInput[i] = Serial1.read();}
         servo = userInput[0];
         pos = userInput[1];
         if (pos == 255){ servo = 255;}

              strip.setPixelColor(0, RGBalive,0,0);
              strip.setPixelColor(1, RGBalive,0,0);
              strip.setPixelColor(2, RGBalive,0,0);
              strip.setPixelColor(3, RGBalive,0,0);
         
         switch (servo) {
            case 1: // LEFT V
              pos = map(pos, 0, 180, 1, 254);
              if (debug == true){Serial.print("Left V 1: ");Serial.println(pos);}
              left_V = pos;
               break;
            case 2: // LEFT H
              pos = map(pos, 0, 180, 1, 254);
              if (debug == true){Serial.print("Left H 2: ");Serial.println(pos);}
              right_H = pos;
               break;
            case 3: // RIGHT V
              pos = map(pos, 0, 180, 1, 254);
              if (debug == true){Serial.print("Right V 3: ");Serial.println(pos);}
              right_V = pos;
               break;
            case 4: // RIGHT H
              pos = map(pos, 0, 180, 1, 254);
              if (debug == true){Serial.print("Right H 4: ");Serial.println(pos);}
              left_H = pos;
               break;
            case 5:
            // nothing yet
               break;
            case 6:  // WALK GAIT BUTTON INSTEAD OF LIGHTS
             if (pos == 0) {
                      // nothing
                 } else if (pos == 1) {
                          buttonState[0] = 1;
                          
                          strip.setPixelColor(0, colorgreen);
                          strip.setPixelColor(1, colorgreen);
                          strip.setPixelColor(2, colorgreen);
                          strip.setPixelColor(3, colorgreen);
                 }
                break; 
            case 7:
            // speed
              if (pos == 1) {
                      buttonState[6] = 1;
              }
               break;
            case 8: 
                break; 
         }
       }            
   }


// BUTTON ARRAY

  allButtons = 0; // Clear variable that holds all the buttons
  
      for(int i=0;i<8;i++){
        if(buttonState[i] == 1){
          allButtons += pow(2,i);
          buttonState[i] = 0;
        }
      }


// SEND SERIAL PACKETS TO HEXA

    Serial2.write(0xff);          // header
    Serial2.write((byte)right_V); // right vertical joystick
    Serial2.write((byte)right_H); // right horizontal joystick
    Serial2.write((byte)left_V);  // left vertical joystick
    Serial2.write((byte)left_H);  // left horizontal joystick
    Serial2.write(allButtons);    // single byte holds all the button data 
    Serial2.write((byte)0);       // 0 char
    Serial2.write((byte)(255 - (right_V+right_H+left_V+left_H+allButtons)%256));  //checksum
    

// SET DEFAULT STATES

// GET UP

    if (walk == 0){
      delay(500);
      buttonState[4] = 1; // Get up  
      walk = 1;
      gait = 0;
      }

// CYCLE SET DEFAULT GATE

    if (gait == 0){
      countergait = countergait + 1;
      delay(500);
      buttonState[0] = 1;
        if (countergait >= 4){
             gait = 1;
             doublespeed = 0;
           }
      }

// CYCLE SET DOUBLE LENGTH GAIT SPEED

     if (doublespeed == 0){
      counterspeed = counterspeed + 1;
      delay(500);
      buttonState[6] = 1;
        if (counterspeed >= 2){
             doublespeed = 1;
           }
      }

strip.show();
} 

