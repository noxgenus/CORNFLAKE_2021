/*

=========================================================================
VWR ROBOTICS CORNFLAKE 2020 - SABRE V2 - HEAVY ROBOT
=========================================================================
-------------------------------------------------------------------------
CORNFLAKE LIDAR TEST MODULE V1.0
-------------------------------------------------------------------------
Hardware used for this sketch:

- Teensy 4.0
- HC-12 433MHz Serial Tranciever
- YDlidar X4


CONTROL CHANNEL: HC-12 #1 CHANNEL = C044
TELEMETRY CHANNEL: HC-12 #2 CHANNEL = C066

HC-12 SET CHANNEL: AT+C044

*/
 
#include <YDLidar.h>
YDLidar lidar;
bool isScanning = false;   

#define YDLIDAR_MOTOR_SCTP 2 // The PWM pin for speed control motor. 
#define YDLIDAR_MOTRO_EN   3 // The motor ENABLE PIN                
#define scanningLed 4        // scanning led
#define errorLed 5           // error scanning led
#define distanceLed 6           // distance threshold test led


int runningFlag = 0;

//#include <Adafruit_NeoPixel.h>
//
//#define PIN            4    // Node-MCU Input Pin
//#define NUMPIXELS      24    // Number of Pixels on the Ring
//#define BRIGHTNESS     20    // Brightness between 0 - 255
//
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//
//unsigned long now = millis();
//unsigned long lastStartStopTime = 0;
//
//

    
void setup() {
  
  //x4:128000

  // bind the YDLIDAR driver to the arduino hardware serial
  Serial.begin(128000);
  
  lidar.begin(Serial1, 128000);


  pinMode(YDLIDAR_MOTOR_SCTP, OUTPUT);
  pinMode(YDLIDAR_MOTRO_EN, OUTPUT);

//  pixels.begin();
//  pixels.setBrightness(BRIGHTNESS);
//  pixels.show();

  pinMode(scanningLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  pinMode(distanceLed, OUTPUT);


  
  digitalWrite(scanningLed, LOW);
  digitalWrite(errorLed, LOW);
  digitalWrite(distanceLed, LOW);
  delay(500);
  digitalWrite(scanningLed, HIGH);
  digitalWrite(errorLed, HIGH);
  digitalWrite(distanceLed, HIGH);
  delay(500);
  digitalWrite(scanningLed, LOW);
  digitalWrite(errorLed, LOW);
  digitalWrite(distanceLed, LOW);
  delay(500);
  digitalWrite(scanningLed, HIGH);
  digitalWrite(errorLed, HIGH);
  digitalWrite(distanceLed, HIGH);
  delay(500);
  digitalWrite(scanningLed, LOW);
  digitalWrite(errorLed, LOW);
  digitalWrite(distanceLed, LOW);
  delay(500);
  digitalWrite(scanningLed, HIGH);
  digitalWrite(errorLed, HIGH);
  digitalWrite(distanceLed, HIGH);
  delay(500);
  digitalWrite(scanningLed, LOW);
  digitalWrite(errorLed, LOW);
  digitalWrite(distanceLed, LOW);
  delay(500);
  digitalWrite(scanningLed, HIGH);
  digitalWrite(errorLed, HIGH);
  digitalWrite(distanceLed, HIGH);
  delay(500);
  digitalWrite(scanningLed, LOW);
  digitalWrite(errorLed, LOW);
  digitalWrite(distanceLed, LOW);
  delay(1000);
  //while(Serial1.read() >= 0){};

}

void loop() {


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

          if (((angle > 100 ) && (angle < 130)) && (distance < 300)){
              digitalWrite(distanceLed, HIGH);
          } else {
             // digitalWrite(distanceLed, LOW);
          }

          
//
//          if (((angle > 15 ) && (angle < 31)) && (distance < 300)){
//            pixels.setPixelColor(1, pixels.Color(200, 0, 0));
//          } else {
//            pixels.setPixelColor(1, pixels.Color(0, 200, 0));
//          }
//
//          if (((angle > 30 ) && (angle < 46)) && (distance < 300)){
//            pixels.setPixelColor(2, pixels.Color(200, 0, 0));
//          } else {
//            pixels.setPixelColor(2, pixels.Color(0, 200, 0));
//          }

          digitalWrite(scanningLed, HIGH);
          digitalWrite(errorLed, LOW);
          runningFlag = 0;
          
      }else{
        
         Serial.println(" YDLIDAR get Scandata fialed!!");

//          for(byte x=0;x<24;x++) {
//              pixels.setPixelColor(x, pixels.Color(200, 0, 0));
//          }
          digitalWrite(scanningLed, LOW);
          digitalWrite(errorLed, HIGH);

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

      digitalWrite(scanningLed, LOW);
      digitalWrite(errorLed, LOW);

//      for(byte x=0;x<24;x++) {
//              pixels.setPixelColor(x, pixels.Color(0, 0, 200));
//          }
      
    }

    //pixels.show();
    delay(20);
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
              //delay(1000);
              }else{
                  Serial.println("start YDLIDAR is failed!  Continue........");
              }
          }else{
              Serial.println("cannot retrieve YDLIDAR health");
          }
  
      
       }else{
             Serial.println("YDLIDAR get DeviceInfo Error!!!");
       }
}

