//HC-12 AT Commander
//The HC-12 commander mode runs at baud rate 9600

// AT+RX // Get settings

// AT+C044 //set channel

// OK+B9600
// OK+RC022
// OK+RP:+20dBm
// OK+FU3

// AT+V // get firmware version

// www.hc01.com HC-12 v2.6



#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  mySerial.begin(9600);
}

void loop(){
  if (mySerial.available()){
    Serial.write(mySerial.read());
  }
  if (Serial.available()){
    mySerial.write(Serial.read());
  }
}
