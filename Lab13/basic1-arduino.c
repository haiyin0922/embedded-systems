#include <SoftwareSerial.h> 
// use pins 10 and 11 instead of Tx and Rx pins
SoftwareSerial mySerial(10,11);
void setup() { 
  Serial.begin(9600);   
  mySerial.begin(9600);   
}

void loop() {
  // useful functions
  char data;
  char send_data;
  
  //Serial.available();   // UART buffer not empty?
  if(Serial.available()){
    send_data = Serial.read();
    delay(3);
    mySerial.print(send_data);
    delay(5);
  }
  
  if(mySerial.available()){
    data = mySerial.read();
    delay(3);
    Serial.print(data);
    delay(5);
  }
}
