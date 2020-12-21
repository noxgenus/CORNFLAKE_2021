/*

=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - SABRE V2 - HEAVY ROBOT
=========================================================================
-------------------------------------------------------------------------
CORNFLAKE RECEIVER V2.0 FOR SABRE V2 based on TEENSY 4.0
-------------------------------------------------------------------------
Hardware used for this sketch:

- Teensy 4.0
- HC-12 433MHz Serial Trancievers (ON 3V!)
- Sabretooth 2x25A Motor Controller
- APA102 Leds (Adafruit Dotstar)  (Run Teensy on 72MHz!)
- 20A BEC                         (input 12V/ Output 5V)
- 2x 5A LiPo batteries 12V        (12V 10A)
- 4x 5A LiPo batteries 24V        (24V 10A)

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

// SABERTOOTH PACKET SERIAL VIA LIB
#include <Sabertooth.h>
Sabertooth ST(128, Serial1); // The Sabertooth is on address 128. We'll name its object ST.

// X4 LIDAR
#include <YDLidar.h>
YDLidar lidar;
bool isScanning = false;   

#define YDLIDAR_MOTOR_SCTP 3 // The PWM pin for speed control motor. 
#define YDLIDAR_MOTRO_EN   2 // The motor ENABLE PIN                
#define errorLed 4           // error scanning led

int runningFlag = 0;


// DOTSTAR APA102
#include <Adafruit_DotStar.h>
#define NUMPIXELS 48
#define DATAPIN    19
#define CLOCKPIN   18
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// SERVOS
#include <Servo.h>

const int panOffset = 5;
int posipan = 90;
int positilt = 90;
int lidarToggle = 1;

Servo servo3; // TILT
Servo servo4; // PAN

long time=0; // timer

// Pulse APA102
int pulseLed = 800;
int RGBwarning = 10;
int RGBpolice; 
int RGBpolice2; 
int pulsespeed = 300;
int lightState = 0;

// Status led remote connection
static int serialLed = 23;


// Common servo setup values for Hitec and Futuba

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
const long timeout = 600;
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



void setup() {

// PORTS
  Serial.begin(9600);   // debug monitor
  Serial1.begin(9600);  // SABERTOOTH (TX)
  Serial2.begin(9600);  // HC12 C044 (RX) (AUTONOM/ENCRYPTION MODULE)
  Serial3.begin(9600);  // RASPBERRY PI (TX) (CANON CONTROLLER)

// LIDAR
  lidar.begin(Serial4, 128000);
  pinMode(YDLIDAR_MOTOR_SCTP, OUTPUT);
  pinMode(YDLIDAR_MOTRO_EN, OUTPUT);
  pinMode(errorLed, OUTPUT);

// SABERTOOTH
  ST.autobaud(); // Send the autobaud command to the Sabertooth controller(s).
  ST.drive(0); // Sabertooth Packed Serial FWD
  ST.turn(0); // Sabertooth Packed Serial TURN

  // PREVENT RUNAWAY ROBOT ON SIGNAL TIMEOUT FROM CONTROLLER OR MC REBOOT
  ST.setTimeout(950);

   
  // Attach each Servo object to a digital pin
    servo3.attach(5, minPulseHITEC, maxPulseHITEC);     //tilt
    servo4.attach(6, minPulseHITEC, maxPulseHITEC);     // pan

  // Set servos to neutral position
    servo3.write(lastKnownPos[3]);
    servo4.write(lastKnownPos[4]);

    if (debug == true){Serial.println("CORNFLAKE SABRE V2!");}

    strip.begin();
    strip.show();

    pinMode(serialLed, OUTPUT); // serial connection remote status led
    digitalWrite(serialLed, 0);
    digitalWrite(errorLed, 0);

    ledBoot();
            
     if (debug == true){Serial.println("READY!");}

} 

void loop() {

  time = millis();

    time = millis();
    RGBwarning = 128+127*cos(2*PI/pulseLed*time);
    RGBpolice = 128+127*cos(2*PI/pulsespeed*time);
    RGBpolice2 = 128+127*cos(2*PI/pulsespeed*time+255);
  
 // Serial safety - Full stop on lost connection of controller.
     counter = counter + 1;
     
      if (counter >= timeout){
          if (debug == true){Serial.println("No connection!");}

            // SET motors and servos into neutral position and update array
            
            ST.drive(0); // Sabertooth Packed Serial
            ST.turn(0); // Sabertooth Packed Serial
            servo3.write(180);  lastKnownPos[3] = 180;
            servo4.write(90);   lastKnownPos[4] = 90;
            
            
            // Show red pulsing leds
            for(byte x=0;x<NUMPIXELS;x++) {
              strip.setPixelColor(x, RGBwarning,RGBwarning,0);
            }
            
            digitalWrite(serialLed, 0);
          }
        

// Start reading sender(s)

    if (Serial2.available() > 2) {

       startbyte = Serial2.read();
       counter = 0; // Reset connection timeout counter
       
       digitalWrite(serialLed, 1);
    
       if (startbyte == 255) {

          for (i=0;i<2;i++){ userInput[i] = Serial2.read();}

            servo = userInput[0];
            pos = userInput[1];
            if (pos == 255){ servo = 255;}

            offFront();

           if (lightState == 0){
             // OFF
            } else if (lightState == 1){
                strip.setPixelColor(0, 255,255,255);
                strip.setPixelColor(1, 255,255,255);
      
                strip.setPixelColor(19, 255,255,255);
                strip.setPixelColor(20, 255,255,255);

            
            } else if (lightState == 2){
                strip.setPixelColor(0, 255,255,255);
                strip.setPixelColor(1, 255,255,255);
                strip.setPixelColor(2, 255,255,255);
                strip.setPixelColor(3, 255,255,255);

                strip.setPixelColor(17, 255,255,255);
                strip.setPixelColor(18, 255,255,255);
                strip.setPixelColor(19, 255,255,255);
                strip.setPixelColor(20, 255,255,255);

            
            } else if (lightState == 3){
                strip.setPixelColor(0, 255,255,255);
                strip.setPixelColor(1, 255,255,255);
                strip.setPixelColor(2, 255,255,255);
                strip.setPixelColor(3, 255,255,255);
                strip.setPixelColor(4, 255,255,255);
                strip.setPixelColor(5, 255,255,255);
                strip.setPixelColor(6, 255,255,255);

                strip.setPixelColor(14, 255,255,255);
                strip.setPixelColor(15, 255,255,255);
                strip.setPixelColor(16, 255,255,255);
                strip.setPixelColor(17, 255,255,255);
                strip.setPixelColor(18, 255,255,255);
                strip.setPixelColor(19, 255,255,255);
                strip.setPixelColor(20, 255,255,255);

            
            } else if (lightState == 4){
                strip.setPixelColor(0, 255,255,255);
                strip.setPixelColor(1, 255,255,255);
      
                strip.setPixelColor(19, 255,255,255);
                strip.setPixelColor(20, 255,255,255);
         
                // Police light pulse override
                strip.setPixelColor(2, 0,0,RGBpolice);
                strip.setPixelColor(3, 0,0,RGBpolice);
                strip.setPixelColor(4, 0,0,RGBpolice);
                strip.setPixelColor(5, 0,0,RGBpolice);
      
                strip.setPixelColor(15, 0,0,RGBpolice2);
                strip.setPixelColor(16, 0,0,RGBpolice2);
                strip.setPixelColor(17, 0,0,RGBpolice2);
                strip.setPixelColor(18, 0,0,RGBpolice2);
      
//                strip.setPixelColor(32, 0,0,RGBpolice);
//                strip.setPixelColor(33, 0,0,RGBpolice);
//                strip.setPixelColor(38, 0,0,RGBpolice2);
//                strip.setPixelColor(39, 0,0,RGBpolice2);

            }
         
         switch (servo) {
            case 1: // DRIVE
              if (debug == true){Serial.print("FWD/REV: ");Serial.println(pos);}
              
              offRear();

               if (pos > 98){ 
                      strip.setPixelColor(21, 255,0,0);
                      strip.setPixelColor(22, 255,0,0);
                      strip.setPixelColor(23, 255,0,0);
                      strip.setPixelColor(24, 255,0,0);
          
                      strip.setPixelColor(38, 255,0,0);
                      strip.setPixelColor(39, 255,0,0);
                      strip.setPixelColor(40, 255,0,0);
                      strip.setPixelColor(41, 255,0,0);
                      
                      
                    } else if (pos < 80){
                      strip.setPixelColor(21, 255,0,0);
                      strip.setPixelColor(22, 255,0,0);
                      strip.setPixelColor(23, 255,0,0);
                      strip.setPixelColor(24, 255,255,255);
                      strip.setPixelColor(25, 255,255,255);
                      strip.setPixelColor(26, 255,255,255);
          
                      strip.setPixelColor(36, 255,255,255);
                      strip.setPixelColor(37, 255,255,255);
                      strip.setPixelColor(38, 255,255,255);
                      strip.setPixelColor(39, 255,0,0);
                      strip.setPixelColor(40, 255,0,0);
                      strip.setPixelColor(41, 255,0,0);
                     
                    } else {
                      strip.setPixelColor(21, 255,0,0);
                      strip.setPixelColor(22, 255,0,0);
                      strip.setPixelColor(23, 255,0,0);
                      strip.setPixelColor(24, 255,0,0);
                      strip.setPixelColor(25, 255,0,0);
                      strip.setPixelColor(26, 255,0,0);
                      strip.setPixelColor(27, 255,0,0);
                      strip.setPixelColor(28, 255,0,0);
          
                      strip.setPixelColor(34, 255,0,0);
                      strip.setPixelColor(35, 255,0,0);
                      strip.setPixelColor(36, 255,0,0);
                      strip.setPixelColor(37, 255,0,0);
                      strip.setPixelColor(38, 255,0,0);
                      strip.setPixelColor(39, 255,0,0);
                      strip.setPixelColor(40, 255,0,0);
                      strip.setPixelColor(41, 255,0,0);
                     
                    }
               
               pos = map(pos, 0, 180, -127, 127);
               ST.drive(pos);
               
               lastKnownPos[1] = pos;
               
                       
               break;
            case 2: // TURN
              if (debug == true){Serial.print("TURN: ");Serial.println(pos);}
              pos = map(pos, 0, 180, 127, -127);
              ST.turn(pos);
              
              lastKnownPos[2] = pos;
              
               break;
            case 3: // TILT
              if (debug == true){Serial.print("TILT: ");Serial.println(pos);}

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
                  

               break;
            case 4: // PAN
              if (debug == true){Serial.print("PAN: ");Serial.println(pos);}
              //pos = map(pos, 0, 180, 180, 0); // reverse

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
                 
               break;
            case 5: // PAN 2 (rotation stick)
            
              if (debug == true){Serial.print("Servo5 PAN 2: ");Serial.println(pos);}
              pos = map(pos, 0, 180, 180, 0);
              //servo4.write(pos - panOffset);
              lastKnownPos[5] = pos;
              
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
                        
                break; 
             case 7: // SERVO MODE/LIDAR SWITCH/GAIT CHANGE
                if (pos == 0) {
                    lidarToggle = 1;
                 } else if (pos == 1) {
                    lidarToggle = 0;
                 }
                 lastKnownPos[7] = pos;
                
                break;
             case 8: // FIRE BUTTON
                if (pos == 0) {
                     // Nothing yet...
                     busyflag = 0;
                 } else if (pos == 1) {
                    strip.setPixelColor(0, 150,0,0);
                    strip.setPixelColor(1, 150,0,0);
                    strip.setPixelColor(2, 150,0,0);
                    strip.setPixelColor(3, 150,0,0);
                    strip.setPixelColor(4, 150,0,0);
                    strip.setPixelColor(5, 150,0,0);
                    strip.setPixelColor(6, 150,0,0);
                    strip.setPixelColor(7, 150,0,0);

                    strip.setPixelColor(13, 150,0,0);
                    strip.setPixelColor(14, 150,0,0);
                    strip.setPixelColor(15, 150,0,0);
                    strip.setPixelColor(16, 150,0,0);
                    strip.setPixelColor(17, 150,0,0);
                    strip.setPixelColor(18, 150,0,0);
                    strip.setPixelColor(19, 150,0,0);
                    strip.setPixelColor(20, 150,0,0);
                    
                    photo(); // function on this button
                    
                    
                 }
                lastKnownPos[8] = pos;
              
                break; 
         
         }
      }            
    }


// LIDAR INPUT ------------------------------------------------------------------

if(lidarToggle == 1){ // LIDAR ON
  
    if(isScanning){
      if (lidar.waitScanDot() == RESULT_OK) {
        
          float distance = lidar.getCurrentScanPoint().distance; //distance value in mm unit
          float angle    = lidar.getCurrentScanPoint().angle; //anglue value in degree
          byte  quality  = lidar.getCurrentScanPoint().quality; //quality of the current measurement
          bool  startBit = lidar.getCurrentScanPoint().startBit;
          
          Serial.print("current angle:");
          Serial.println(angle);
          Serial.print("current distance:");
          Serial.println(distance);
          digitalWrite(errorLed, 0);
          runningFlag = 0;
          
      }else{
         Serial.println(" YDLIDAR get Scandata fialed!!");
          digitalWrite(errorLed, 1);
          runningFlag ++;
          if (runningFlag > 500){
            restartScan();
          }
      }
      
    }else{
      //stop motor
      digitalWrite(YDLIDAR_MOTRO_EN, LOW);
      setMotorSpeed(0);
      restartScan();
      digitalWrite(errorLed, LOW);
    }

} else { // LIDAR OFF

  //stop motor
      digitalWrite(YDLIDAR_MOTRO_EN, LOW);
      setMotorSpeed(0);
      digitalWrite(errorLed, 0);
      isScanning = false;
}

//-------------------------------------------------------------------------------
    
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
          strip.setPixelColor(10, 0,0,0);
          strip.setPixelColor(11, 0,0,0);
          strip.setPixelColor(12, 0,0,0);
          strip.setPixelColor(13, 0,0,0);
          strip.setPixelColor(14, 0,0,0);
          strip.setPixelColor(15, 0,0,0);
          strip.setPixelColor(16, 0,0,0);
          strip.setPixelColor(17, 0,0,0);
          strip.setPixelColor(18, 0,0,0);
          strip.setPixelColor(19, 0,0,0);
          strip.setPixelColor(20, 0,0,0);


}
void offRear(){

            strip.setPixelColor(21, 0,0,0);
            strip.setPixelColor(22, 0,0,0);
            strip.setPixelColor(23, 0,0,0);
            strip.setPixelColor(24, 0,0,0);
            strip.setPixelColor(25, 0,0,0);
            strip.setPixelColor(26, 0,0,0);
            strip.setPixelColor(27, 0,0,0);
            strip.setPixelColor(28, 0,0,0);
            strip.setPixelColor(29, 0,0,0);
            strip.setPixelColor(30, 0,0,0);
            strip.setPixelColor(31, 0,0,0);
            strip.setPixelColor(32, 0,0,0);
            strip.setPixelColor(33, 0,0,0);
            strip.setPixelColor(34, 0,0,0);
            strip.setPixelColor(35, 0,0,0);
            strip.setPixelColor(36, 0,0,0);
            strip.setPixelColor(37, 0,0,0);
            strip.setPixelColor(38, 0,0,0);
            strip.setPixelColor(39, 0,0,0);
            strip.setPixelColor(40, 0,0,0);
            strip.setPixelColor(41, 0,0,0);

}


//------------------------------------------------------------
// TELEMETRY SEND TO TX 2
//------------------------------------------------------------

void telemetry(int item, int data) {
//       Serial2.write(char(255));
//       Serial2.write(char(lastKnownPos[0]));
//       Serial2.write(char(lastKnownPos[1]));
//       Serial2.write(char(lastKnownPos[2]));
//       Serial2.write(char(lastKnownPos[3]));
//       Serial2.write(char(lastKnownPos[4]));
//       Serial2.write(char(lastKnownPos[5]));
//       Serial2.write(char(lastKnownPos[6]));
//       Serial2.write(char(lastKnownPos[7]));
//       Serial2.write(char(lastKnownPos[8]));
//       Serial2.write(char(lastKnownPos[9]));
//       Serial2.write(char(lastKnownPos[10]));
//       Serial2.write(char(lastKnownPos[11]));

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

            strip.setPixelColor(0, 150,0,0);
            strip.setPixelColor(1, 150,0,0);
            strip.setPixelColor(19, 150,0,0);
            strip.setPixelColor(20, 150,0,0);
            strip.show();
            delay(500); 
          
            strip.setPixelColor(2, 150,0,0);
            strip.setPixelColor(3, 150,0,0);
            strip.setPixelColor(17, 150,0,0);
            strip.setPixelColor(18, 150,0,0);

            strip.show();
            delay(500); 
         
            strip.setPixelColor(4, 150,0,0);
            strip.setPixelColor(5, 150,0,0);
            strip.setPixelColor(15, 150,0,0);
            strip.setPixelColor(16, 150,0,0);
  
            strip.show();
            delay(500); 

            strip.setPixelColor(6, 150,0,0);
            strip.setPixelColor(7, 150,0,0);
            strip.setPixelColor(13, 150,0,0);
            strip.setPixelColor(14, 150,0,0);
  
            strip.show();
            delay(500); 

            strip.setPixelColor(8, 150,0,0);
            strip.setPixelColor(9, 150,0,0);
            strip.setPixelColor(11, 150,0,0);
            strip.setPixelColor(12, 150,0,0);
  
            strip.show();
            delay(500); 

            //systemready();
}


void setMotorSpeed(float vol){
  uint8_t PWM = (uint8_t)(51*vol);
  analogWrite(YDLIDAR_MOTOR_SCTP, PWM);
}
void setMotorSpeed2(float vol){
  analogWrite(YDLIDAR_MOTOR_SCTP, vol);
}


void restartScan(){
    device_info deviceinfo;
    if (lidar.getDeviceInfo(deviceinfo, 100) == RESULT_OK) {
         int _samp_rate=4;
         String model;
         float freq = 7.0f;
         switch(deviceinfo.model){
            case 1:
                model="F4";
                _samp_rate=4;
                freq = 7.0;
                break;
            case 4:
                model="S4";
                _samp_rate=4;
                freq = 7.0;
                break;
            case 5:
                model="G4";
                _samp_rate=9;
                freq = 7.0;
                break;
            case 6:
                model="X4";
                _samp_rate=5;
                freq = 8.0;
                break;
            default:
                model = "Unknown";
          }

          uint16_t maxv = (uint16_t)(deviceinfo.firmware_version>>8);
          uint16_t midv = (uint16_t)(deviceinfo.firmware_version&0xff)/10;
          uint16_t minv = (uint16_t)(deviceinfo.firmware_version&0xff)%10;
          if(midv==0){
            midv = minv;
            minv = 0;
          }

          
          Serial.print("Firmware version:");
          Serial.print(maxv,DEC);
          Serial.print(".");
          Serial.print(midv,DEC);
          Serial.print(".");
          Serial.println(minv,DEC);

          Serial.print("Hardware version:");
          Serial.println((uint16_t)deviceinfo.hardware_version,DEC);

          Serial.print("Model:");
          Serial.println(model);

          Serial.print("Serial:");
          for (int i=0;i<16;i++){
            Serial.print(deviceinfo.serialnum[i]&0xff, DEC);
          }
          Serial.println("");

          Serial.print("[YDLIDAR INFO] Current Sampling Rate:");
          Serial.print(_samp_rate,DEC);
          Serial.println("K");

          Serial.print("[YDLIDAR INFO] Current Scan Frequency:");
          Serial.print(freq,DEC);
          Serial.println("Hz");
          delay(100);
          device_health healthinfo;
          if (lidar.getHealth(healthinfo, 100) == RESULT_OK){
             // detected...
              Serial.print("[YDLIDAR INFO] YDLIDAR running correctly! The health status:");
              Serial.println( healthinfo.status==0?"well":"bad");
              if(lidar.startScan() == RESULT_OK){
                isScanning = true;
                //start motor in 1.8v
                setMotorSpeed(1.8);
                
              //setMotorSpeed2(255);
                digitalWrite(YDLIDAR_MOTRO_EN, HIGH);
                Serial.println("Now YDLIDAR is scanning ......");

              }else{
                  Serial.println("start YDLIDAR is failed!  Continue........");
                   digitalWrite(errorLed, 1);
              }
          }else{
              Serial.println("cannot retrieve YDLIDAR health");
               digitalWrite(errorLed, 1);
          }
       }else{
             Serial.println("YDLIDAR get DeviceInfo Error!!!");
             digitalWrite(errorLed, 1);
       }
}

