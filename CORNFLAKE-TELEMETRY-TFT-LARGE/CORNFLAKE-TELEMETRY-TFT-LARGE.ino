/*
 * 
=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - TFT TEST SKETCH
=========================================================================
 WIRELESS ROBOT TELEMETRY DISPLAY WITH 2.8 TFT
 Hardware:
     2.8" Modified TFT Library (Non-Adafruit Chinese knock-off)
     Arduino Nano V3
     2600mAh Powerbank
     HC-12 433MHz Serial Transponder (NON-DUPLEX!) for transmission of joystick + button values in byte array

HC12 C033 (RX)

USE: tft.drawRect(Xaxis,Yaxis,Width,Height,Color);


Telemetry Array Config:

0-(0/1/2)         Remote to Robot communication status
1-(VALUE)         FWD/REV
2-(VALUE)         TURN
3-(VALUE)         TILT
4-(VALUE)         PAN
5-(VALUE)         PAN2 (joy dial)
6-(0/1/2/3/4/5)   LIGHTS
7-(0/1)           SERVO MODE
8-(0/1)           FIRE/SHOOT
9-(VALUE)         TEMP
10-(VALUE)        DOF1
11-(VALUE)        DOF2
12-(VALUE)        DOF3
13-(VALUE)        GYRO
14-(VALUE)        BATT
15-(0/1)          CRITICAL ERROR
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include "Andersmmg_TFTLCD.h" // Hardware-specific library

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define LINECOLOR1   0x07E0
#define CYAN    0x07FF

Andersmmg_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


// Debug on/off
boolean debug = false;

int telemetryPart;
int Ssr = 2; // SERIAL LED

// TELEMETRY DATA ARRAY

int incomingData[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

// HC12 SERIAL INPUT SERVO/VALUE
int userInput[3];    // raw input from serial buffer, 3 bytes
int startbyte;       // start byte, begin reading input
int servo;           // which servo to pulse?
int pos;             // servo angle 0-180
int i;               // iterator

// Serial timeout on lost connection
const long timeout = 1000;
long counter = 0;



  void setup(void) {

   Serial.begin(9600);
//   Serial.println(F("TFT LCD test"));
  
  #ifdef USE_ADAFRUIT_SHIELD_PINOUT
  //  Serial.println(F("Using Adafruit 2.8\" TFT Arduino Shield Pinout"));
  #else
   // Serial.println(F("Using Adafruit 2.8\" TFT Breakout Board Pinout"));
  #endif

  //Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();
  uint16_t identifier = tft.readID();
  
  tft.begin(identifier);
  tft.setRotation(-1);

  delay(100);  
  testFillScreen(); // boot ani
  delay(500);
  drawgrid();

}


void loop(void) {


  // Serial safety - Full stop on lost connection of controller.
     counter = counter + 1;
     
      if (counter >= timeout){
          if (debug == true){Serial.println("No connection!");}
            
            digitalWrite(Ssr, 0); // SERIAL COMMS STATUS LED
          }
        

// Start reading sender(s)

    if (Serial.available() > 2) {

       startbyte = Serial.read();
       counter = 0; // Reset connection timeout counter
       
       digitalWrite(Ssr, 0); // SERIAL COMMS STATUS LED
    
       if (startbyte == 255) {

          for (i=0;i<2;i++){ userInput[i] = Serial.read();}

            telemetryPart = userInput[0];
            pos = userInput[1];
            if (pos == 255){ servo = 255;}

        switch (telemetryPart) {
            case 0: // DRIVE
              if (debug == true){Serial.print("Channel 0: ");Serial.println(pos);}
    
               tft.fillRect(85,62, 60,14, BLACK);
               tft.setCursor(85, 62);
               tft.setTextColor(WHITE); 
               tft.setTextSize(2);
               tft.println(pos);
               
               break;
             case 1: // TURN
              if (debug == true){Serial.print("Channel 1: ");Serial.println(pos);}

               tft.fillRect(85,62, 60,14, BLACK);
               tft.setCursor(85, 62);
               tft.setTextColor(WHITE); 
               tft.setTextSize(2);
               tft.println(pos);
               
               break;

             }
      }            
    }


  delay(2);
  
}




void drawgrid(){

// main box
  tft.drawRect(0,0,320, 231, LINECOLOR1);

// Static Screen Content header

  tft.setCursor(13, 7);
  tft.setTextColor(WHITE); 
  tft.setTextSize(1.5);
  tft.println("CORNFLAKE TELEMETRY MODULE V1.2 VWR ROBOTICS 2020");


// ROWS (DRAW ONCE!)
// x y w h

// first row (header)
        tft.drawRect(0,0,320, 21, LINECOLOR1);

// second row
        tft.drawRect(0,20,161, 31, LINECOLOR1);
        tft.setCursor(10, 28);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("COMMS");
      
        tft.setCursor(142, 28);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("2");
      
        tft.drawRect(160,20,161, 31, LINECOLOR1);
        tft.setCursor(170, 28);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("LIGHTS");
      
        tft.setCursor(300, 28);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("5");

// third row
    tft.drawRect(0,50,161, 31, LINECOLOR1);
     tft.drawRect(160,50,160, 31, LINECOLOR1);

        tft.setCursor(10, 58);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("DRIVE");

        tft.setCursor(118, 58);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 58);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("TURN");
      
        tft.setCursor(276, 58);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");
        

// fourth row
     tft.drawRect(0,80,161, 31, LINECOLOR1);
     tft.drawRect(160,80,160, 31, LINECOLOR1);

         tft.setCursor(10, 88);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("TILT");

        tft.setCursor(118, 88);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 88);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("PAN");
      
        tft.setCursor(276, 88);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");

// fifth row
     tft.drawRect(0,110,161, 31, LINECOLOR1);
     tft.drawRect(160,110,160, 31, LINECOLOR1);

         tft.setCursor(10, 118);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("SMODE");

        tft.setCursor(118, 118);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 118);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("FIRE");
      
        tft.setCursor(300, 118);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("1");


// sixth row
     tft.drawRect(0,140,161, 31, LINECOLOR1);
     tft.drawRect(160,140,160, 31, LINECOLOR1);

         tft.setCursor(10, 148);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("DOF1");

        tft.setCursor(118, 148);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 148);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("DOF2");
      
        tft.setCursor(276, 148);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");

// seventh row
     tft.drawRect(0,170,161, 31, LINECOLOR1);
     tft.drawRect(160,170,160, 31, LINECOLOR1);

         tft.setCursor(10, 178);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("DOF3");

        tft.setCursor(118, 178);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 178);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("GYRO");
      
        tft.setCursor(276, 178);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");

// eighth row
     tft.drawRect(0,200,161, 31, LINECOLOR1);
     tft.drawRect(160,200,160, 31, LINECOLOR1);

         tft.setCursor(10, 208);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("BATT");

        tft.setCursor(118, 208);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("127");


        tft.setCursor(170, 208);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("ERROR");
      
        tft.setCursor(300, 208);
        tft.setTextColor(WHITE); 
        tft.setTextSize(2);
        tft.println("0");


 
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(BLACK);
  tft.fillScreen(RED);
  tft.fillScreen(GREEN);
  tft.fillScreen(BLUE);
  tft.fillScreen(BLACK);
  return micros() - start;
}




