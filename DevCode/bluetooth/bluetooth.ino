
void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  // read from port 1, send to port 0:
  //boolean s1 = Serial1.available();
  //Serial.print(s1);
  int inByte;
  //delay(5000);
  if (Serial1.available()) {
    //Serial.write("Serial1 was available at some point");
    inByte =Serial1.read();
    Serial.write(inByte);
  }

  // (pc to bt mobile) read from port 0, send to port 1:
  
  if (Serial.available()) {
    inByte = Serial.read();
    Serial1.write(inByte);
  }
  
}
/*
#include <SoftwareSerial.h>

const int rxPin = 3;
const int txPin = 2;
const int btBAUDRate=9600;
SoftwareSerial bluetooth(txPin, rxPin);

void setup() {
  Serial.begin(9600);
  bluetooth.begin(btBAUDRate);
  bluetooth.print("$");
  bluetooth.print("$");
  delay(100);
}

void loop() {

  char data = ' ';
  boolean test = bluetooth.available();
  Serial.print(test);
  data = (char)bluetooth.read();
  Serial.print(data);
  if (bluetooth.available()>0)
  {
    Serial.println((char)bluetooth.read());
    
    Serial.print(bluetooth.read());

    data = Serial.read();
    
    //data = 'a';
    Serial.print(data);
    
  }

  if (Serial.available())
  {
    bluetooth.write(Serial.read());
    //bluetooth.print(Serial.read());
  }

  
  delay(500);
}

*/
