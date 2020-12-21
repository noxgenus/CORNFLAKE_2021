/*

=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - LYNXMOTION TANK
=========================================================================
-------------------------------------------------------------------------
CORNFLAKE LYNX V3.0 FOR TEENSY
-------------------------------------------------------------------------
Hardware used for this sketch:

- Teensy 3.2
- HC-12 433MHz Serial Trancievers (ON 3V!)
- Servos or Motor Controller (2x)
- APA102 Leds (Adafruit Dotstar) (Run Teensy on 72MHz!)
- 10A BEC (input 12V/ Output 5V)
- 5A LiPo battery 12V including charge unit
- Servos and shit

------------------------------------------------------------------------    

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

#include <Servo.h>
#include <Adafruit_DotStar.h>

#define NUMPIXELS 16 //10 front - 6 rear
#define DATAPIN    19
#define CLOCKPIN   18
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);


const int panOffset = 5;
int posipan = 90;
int positilt = 90;
int servomode = 1;

Servo servo1; // FWD
Servo servo2; // TURN
Servo servo3; // TILT
Servo servo4; // PAN
Servo servo5; // DOF1 // hand open/close
Servo servo6; // DOF2 // wrist turn
Servo servo7; // DOF3 // elbow bend

Servo rcLightUnit; // RC LIGHT UNIT

long time=0; // timer

int pulseLed = 800;
int RGBwarning = 10;
int lightState = 0;

// Common servo setup values
int minPulseHITEC = 900;    // minimum servo position, us (microseconds)
int maxPulseHITEC = 2100;    // maximum servo position, us

int minPulseFUTUBA = 1000;    // minimum servo position, us (microseconds)
int maxPulseFUTUBA = 2000;    // maximum servo position, us


// HC12 SERIAL INPUT SERVO/VALUE
int userInput[3];    // raw input from serial buffer, 3 bytes
int startbyte;       // start byte, begin reading input
int servo;           // which servo to pulse?
int pos;             // servo angle 0-180
int i;               // iterator

// Serial timeout on lost connection
const long timeout = 1000;
long counter = 0;

// Debug on/off
boolean debug = false;

boolean busyflag = false;

// COLOR SETS

uint32_t colorwhite = 0xFFFFFF;
//uint32_t colorred = 0xFF0000;
uint32_t colorredoff = 0x300000;
uint32_t colorblue = 0x0000FF;
uint32_t colorblueoff = 0x000039;
uint32_t coloryellow = 0xFFFF00;
uint32_t coloryellowoff = 0x302d00;
uint32_t colorgreen = 0x00dc1f;
uint32_t coloroff = 0x000000;

uint32_t colorred = 0xb40101;
uint32_t colorreddark = 0x120000;
uint32_t colorreddarker = 0x090000;
uint32_t off = 0x000000;

// Keep init servo positions in boot and general int array (we're adding and subtracting from these init values with encoder!)
int lastKnownPos[13] = {0,90,90,180,90,90,0,0,0,0,10,100,90};

int framerate = 100; // intro led speed lulz
int Ssr = 2; // Zwaailicht 5V switched by Phidget SSR relay (why? cos i had one left that's why)



void setup() {

  Serial.begin(9600);   // debug monitor
  Serial1.begin(9600);  // HC12 C044 (RX) (CONTROL INPUT)
  Serial2.begin(9600);  // HC12 C033 (TX/RX) (AUTONOM/ENCRYPTION MODULE)
  Serial3.begin(9600);  // RASPBERRY PI (TX) (CANON CONTROLLER)
    
    // Attach each Servo object to a digital pin
    servo1.attach(3, minPulseHITEC, maxPulseHITEC);
    servo2.attach(4, minPulseHITEC, maxPulseHITEC);
    servo3.attach(5, minPulseHITEC, maxPulseHITEC);     //tilt
    servo4.attach(6, minPulseHITEC, maxPulseHITEC);     // pan
    servo5.attach(20, minPulseFUTUBA, maxPulseFUTUBA);  // hand close/open
    servo6.attach(21, minPulseHITEC, maxPulseHITEC);    //wrist turn
    servo7.attach(22, minPulseHITEC, maxPulseHITEC);    // elbow bend
    rcLightUnit.attach(23, minPulseFUTUBA, maxPulseFUTUBA);

    // Set servos to neutral position
    servo1.write(lastKnownPos[1]);
    servo2.write(lastKnownPos[2]);
    servo3.write(lastKnownPos[3]);
    servo4.write(lastKnownPos[4]);
    servo5.write(lastKnownPos[10]);
    servo6.write(lastKnownPos[11]);
    servo7.write(lastKnownPos[12]);
    rcLightUnit.write(lastKnownPos[5]);

    pinMode(Ssr, OUTPUT); // zwaailicht ssr
    digitalWrite(Ssr, 0);

    if (debug == true){Serial.println("CORNFLAKE RECEIVER FOR LYNXMOTION!");}

    strip.begin();
    strip.show();

    ledBoot();
            
     if (debug == true){Serial.println("READY!");}

} 

void loop() {

  time = millis();

  RGBwarning = 128+127*cos(2*PI/pulseLed*time); // Led pulsing
  
 // Serial safety - Full stop on lost connection of controller.
     counter = counter + 1;
     
      if (counter >= timeout){
          if (debug == true){Serial.println("No connection!");}

            // SET motors and servos into neutral position and update array
            
            servo1.write(90);   lastKnownPos[1] = 90;
            servo2.write(90);   lastKnownPos[2] = 90;
            servo3.write(180);  lastKnownPos[3] = 180;
            servo4.write(90);   lastKnownPos[4] = 90;
            
            servo5.write(10);   lastKnownPos[10] = 10;
            servo6.write(100);  lastKnownPos[11] = 100;
            servo7.write(90);   lastKnownPos[12] = 90;

            
            // Show red pulsing leds
            for(byte x=0;x<NUMPIXELS;x++) {
              strip.setPixelColor(x, RGBwarning,0,0);
            }
            
            digitalWrite(Ssr, 0); // Zwaailichie uit
            telemetry(0, 0);
          }
        

// Start reading sender(s)

    if (Serial1.available() > 2) {

       startbyte = Serial1.read();
       counter = 0; // Reset connection timeout counter
       digitalWrite(Ssr, 1); // zwaailicht SSR switch
       telemetry(0, 1);
    
       if (startbyte == 255) {

          for (i=0;i<2;i++){ userInput[i] = Serial1.read();}

            servo = userInput[0];
            pos = userInput[1];
            if (pos == 255){ servo = 255;}

            offFront();

           if (lightState == 0){
                // Off anyway so nothing
                  
                } else if (lightState == 1){
                strip.setPixelColor(0, colorwhite);
                strip.setPixelColor(9, colorwhite);
                 rcLightUnit.write(0);
                
    
                } else if (lightState == 2){
                strip.setPixelColor(0, colorwhite);
                strip.setPixelColor(1, colorwhite);
                strip.setPixelColor(8, colorwhite);
                strip.setPixelColor(9, colorwhite);
                 rcLightUnit.write(90);
                 
                
                } else if (lightState == 3){
                strip.setPixelColor(0, colorwhite);
                strip.setPixelColor(1, colorwhite);
                strip.setPixelColor(2, colorwhite);
                strip.setPixelColor(7, colorwhite);
                strip.setPixelColor(8, colorwhite);
                strip.setPixelColor(9, colorwhite);
                  rcLightUnit.write(120);
                  
                
                } else if (lightState == 4){
                strip.setPixelColor(0, colorwhite);
                strip.setPixelColor(1, colorwhite);
                strip.setPixelColor(2, colorwhite);
                strip.setPixelColor(3, colorwhite);
                strip.setPixelColor(6, colorwhite);
                strip.setPixelColor(7, colorwhite);
                strip.setPixelColor(8, colorwhite);
                strip.setPixelColor(9, colorwhite);
                 rcLightUnit.write(180);
                 
                }
         
         switch (servo) {
            case 1: // DRIVE
              if (debug == true){Serial.print("Servo1: ");Serial.println(pos);}
              
              offRear();

                if (pos > 98){ 
                      strip.setPixelColor(10, colorred);
                      strip.setPixelColor(15, colorred);
                    } else if (pos < 80){
                      strip.setPixelColor(10, colorred);
                      strip.setPixelColor(11, colorwhite);
                      strip.setPixelColor(14, colorwhite);
                      strip.setPixelColor(15, colorred);
                    } else {
                      strip.setPixelColor(10, colorred);
                      strip.setPixelColor(11, colorred);
                      strip.setPixelColor(14, colorred);
                      strip.setPixelColor(15, colorred);
                    }
               
               servo1.write(pos);
               lastKnownPos[1] = pos;
               telemetry(1, pos);
                       
               break;
            case 2: // STEERING
              if (debug == true){Serial.print("Servo2: ");Serial.println(pos);}
              pos = map(pos, 0, 180, 180, 0); // REVERSE
              servo2.write(pos);
              lastKnownPos[2] = pos;
              telemetry(2, pos);
               break;
            case 3: // TILT
              if (debug == true){Serial.print("Servo3: ");Serial.println(pos);}
              //pos = map(pos, 0, 180, 180, 0);

                if (servomode == 1) { // ------------------ ADDITIVE CONTROL ---------
                     if ((pos > 100) && (pos < 160)) {
                          positilt=(positilt+1); // slow
                     } else if (pos > 160){
                          positilt=(positilt+2); // faster
                     } else if ((pos < 80) && (pos > 20)) {
                          positilt=(positilt-1); // slow
                     } else if (pos < 20) {
                          positilt=(positilt-2); // faster
                     }

                  if(positilt > 180) positilt=180; //limit upper value
                  if(positilt<0) positilt=0; //limit lower value
                  servo3.write(positilt);
                  lastKnownPos[4] = positilt;
                  telemetry(3, positilt);
                  
                } else if (servomode == 0) { // ------------ NORMAL CONTROL -----------
                    servo3.write(pos);
                    lastKnownPos[3] = pos;
                    telemetry(3, pos);
                }
               break;
            case 4: // PAN
              if (debug == true){Serial.print("Servo4: ");Serial.println(pos);}
              //pos = map(pos, 0, 180, 180, 0); // reverse

                 if (servomode == 1) { // ------------------ ADDITIVE CONTROL ---------
                     if ((pos > 100) && (pos < 160)) {
                          posipan=(posipan+1); // slow
                     } else if (pos > 160){
                          posipan=(posipan+2); // faster
                     } else if ((pos < 80) && (pos > 20)) {
                          posipan=(posipan-1); // slow
                     } else if (pos < 20) {
                          posipan=(posipan-2); // faster
                     }

                  if(posipan > 180) posipan=180; //limit upper value
                  if(posipan<0) posipan=0; //limit lower value
                  servo4.write(posipan);
                  lastKnownPos[4] = posipan;
                  telemetry(4, posipan);
                  
                } else if (servomode == 0) { // ------------ NORMAL CONTROL -----------
                    servo4.write(pos);
                    lastKnownPos[4] = pos;
                    telemetry(4, pos);
                }
               break;
            case 5: // PAN 2 (rotation stick)
              if (debug == true){Serial.print("Servo5 PAN 2: ");Serial.println(pos);}
              pos = map(pos, 0, 180, 180, 0);
              //servo4.write(pos - panOffset);
              lastKnownPos[5] = pos;
              telemetry(5, pos);
               break;
            case 6: // LIGHTS
                if (pos == 0) {
                   if (lightState != 0) {
                       lightState = 0; 
                    } 
                    
                 } else if (pos == 1) {
                   if (lightState != 1) {
                       lightState = 1; 
                    }
                    
                 } else if (pos == 2){
                  if (lightState != 2) {
                      lightState = 2;
                    }
                 
                 } else if (pos == 3) {
                  if (lightState != 3) {
                      lightState = 3;
                    }
                    
                 } else if (pos == 4) {
                   if (lightState != 4) {
                       lightState = 4;
                    }
                 }
                 lastKnownPos[6] = pos;
                 telemetry(6, pos);         
                break; 
             case 7: // SERVO MODE? (type of pan/tilt servo control OR/AND drive or pan/tilt?)
                if (pos == 0) {
                    servomode = 1;
                 } else if (pos == 1) {
                    servomode = 0;
                 }
                 lastKnownPos[7] = pos;
                 telemetry(7, pos);
                break;
             case 8: // FIRE BUTTON
                if (pos == 0) {
                     // Nothing yet...
                     busyflag = 0;
                 } else if (pos == 1) {
                    strip.setPixelColor(0, colorred);
                    strip.setPixelColor(1, colorred);
                    strip.setPixelColor(2, colorred);
                    strip.setPixelColor(3, colorred);
                    strip.setPixelColor(4, colorred);
                    strip.setPixelColor(5, colorred);
                    strip.setPixelColor(6, colorred);
                    strip.setPixelColor(7, colorred);
                    strip.setPixelColor(8, colorred);
                    strip.setPixelColor(9, colorred);
                    
                    photo(); // function on this button
                    
                    
                 }
                lastKnownPos[8] = pos;
                telemetry(8, pos);
                break; 
                
              case 10: // DOF1
                servo5.write(pos);
                    lastKnownPos[10] = pos;
                    telemetry(10, pos);
                break; 
              case 11: // DOF2
                servo6.write(pos);
                    lastKnownPos[11] = pos;
                    telemetry(11, pos);
                break; 
              case 12: // DOF3
                servo7.write(pos);
                    lastKnownPos[12] = pos;
                    telemetry(12, pos);
                break; 
         
         }
      }            
    }
    strip.show();
    delay(2);
} 




void offFront(){
          strip.setPixelColor(0, 0,0,0);
          strip.setPixelColor(1, 0,0,0);
          strip.setPixelColor(2, 0,0,0);
          strip.setPixelColor(3, 0,0,0);
          strip.setPixelColor(4, 0,0,0);
          strip.setPixelColor(5, 0,0,0);
          strip.setPixelColor(6, 0,0,0);
          strip.setPixelColor(7, 0,0,0);
          strip.setPixelColor(8, 0,0,0);
          strip.setPixelColor(9, 0,0,0);

}
void offRear(){
          strip.setPixelColor(10, 0,0,0);
          strip.setPixelColor(11, 0,0,0);
          strip.setPixelColor(12, 0,0,0);
          strip.setPixelColor(13, 0,0,0);
          strip.setPixelColor(14, 0,0,0);
          strip.setPixelColor(15, 0,0,0);
}

//------------------------------------------------------------
// TELEMETRY SEND TO TX 2
//------------------------------------------------------------

void telemetry(int item, int data) {
       Serial2.write(char(255));
       Serial2.write(char(lastKnownPos[0]));
       Serial2.write(char(lastKnownPos[1]));
       Serial2.write(char(lastKnownPos[2]));
       Serial2.write(char(lastKnownPos[3]));
       Serial2.write(char(lastKnownPos[4]));
       Serial2.write(char(lastKnownPos[5]));
       Serial2.write(char(lastKnownPos[6]));
       Serial2.write(char(lastKnownPos[7]));
       Serial2.write(char(lastKnownPos[8]));
       Serial2.write(char(lastKnownPos[9]));
       Serial2.write(char(lastKnownPos[10]));
       Serial2.write(char(lastKnownPos[11]));

}

//------------------------------------------------------------
// CANON PHOTO PI API TX 3
//------------------------------------------------------------

void photo(){
  if (busyflag == 0){
    Serial3.println("1:1");
    //Serial2.println("photo"); //telemetry
    //userdata();
    busyflag = 1;
  }
}

void ledBoot(){
   // Wait for other stuff to boot (lulz)

            strip.setPixelColor(0, colorreddarker);
            strip.setPixelColor(1, colorreddark);
            strip.setPixelColor(2, colorred);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, colorreddarker);
            strip.setPixelColor(2, colorreddark);
            strip.setPixelColor(3, colorred);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, colorreddark);
            strip.setPixelColor(4, colorred);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, colorreddarker);
            strip.setPixelColor(4, colorreddark);
            strip.setPixelColor(5, colorred);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, colorreddarker);
            strip.setPixelColor(5, colorreddark);
            strip.setPixelColor(6, colorred);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, colorreddarker);
            strip.setPixelColor(6, colorreddark);
            strip.setPixelColor(7, colorred);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, colorreddarker);
            strip.setPixelColor(7, colorreddark);
            strip.setPixelColor(8, colorred);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, colorreddark);
            strip.setPixelColor(9, colorred);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, colorred);
            strip.setPixelColor(9, colorreddark);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorred);
            strip.setPixelColor(8, colorreddark);
            strip.setPixelColor(9, colorreddarker);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, colorred);
            strip.setPixelColor(7, colorreddark);
            strip.setPixelColor(8, colorreddarker);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, colorred);
            strip.setPixelColor(6, colorreddark);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, colorred);
            strip.setPixelColor(5, colorreddark);
            strip.setPixelColor(6, colorreddarker);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, colorred);
            strip.setPixelColor(4, colorreddark);
            strip.setPixelColor(5, colorreddarker);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, colorred);
            strip.setPixelColor(3, colorreddark);
            strip.setPixelColor(4, colorreddarker);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, colorred);
            strip.setPixelColor(2, colorreddark);
            strip.setPixelColor(3, colorreddarker);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, colorred);
            strip.setPixelColor(1, colorreddark);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, colorreddark);
            strip.setPixelColor(1, colorred);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

             strip.setPixelColor(0, colorreddarker);
            strip.setPixelColor(1, colorreddark);
            strip.setPixelColor(2, colorred);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, colorreddarker);
            strip.setPixelColor(2, colorreddark);
            strip.setPixelColor(3, colorred);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, colorreddark);
            strip.setPixelColor(4, colorred);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, colorreddarker);
            strip.setPixelColor(4, colorreddark);
            strip.setPixelColor(5, colorred);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, colorreddarker);
            strip.setPixelColor(5, colorreddark);
            strip.setPixelColor(6, colorred);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, colorreddarker);
            strip.setPixelColor(6, colorreddark);
            strip.setPixelColor(7, colorred);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, colorreddarker);
            strip.setPixelColor(7, colorreddark);
            strip.setPixelColor(8, colorred);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, colorreddark);
            strip.setPixelColor(9, colorred);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, colorred);
            strip.setPixelColor(9, colorreddark);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, colorred);
            strip.setPixelColor(8, colorreddark);
            strip.setPixelColor(9, colorreddarker);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, colorred);
            strip.setPixelColor(7, colorreddark);
            strip.setPixelColor(8, colorreddarker);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, colorred);
            strip.setPixelColor(6, colorreddark);
            strip.setPixelColor(7, colorreddarker);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, colorred);
            strip.setPixelColor(5, colorreddark);
            strip.setPixelColor(6, colorreddarker);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, off);
            strip.setPixelColor(3, colorred);
            strip.setPixelColor(4, colorreddark);
            strip.setPixelColor(5, colorreddarker);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, off);
            strip.setPixelColor(2, colorred);
            strip.setPixelColor(3, colorreddark);
            strip.setPixelColor(4, colorreddarker);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, off);
            strip.setPixelColor(1, colorred);
            strip.setPixelColor(2, colorreddark);
            strip.setPixelColor(3, colorreddarker);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, colorred);
            strip.setPixelColor(1, colorreddark);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);

            strip.setPixelColor(0, colorreddark);
            strip.setPixelColor(1, colorred);
            strip.setPixelColor(2, colorreddarker);
            strip.setPixelColor(3, off);
            strip.setPixelColor(4, off);
            strip.setPixelColor(5, off);
            strip.setPixelColor(6, off);
            strip.setPixelColor(7, off);
            strip.setPixelColor(8, off);
            strip.setPixelColor(9, off);
            strip.show();
            delay(framerate);
}

