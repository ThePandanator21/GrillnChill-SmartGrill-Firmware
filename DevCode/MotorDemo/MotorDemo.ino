#include "DualMC33926MotorShield.h"

DualMC33926MotorShield md;

int counter = 0;
int second = 0;
bool a = false, b = false;


void stopIfFault()
{
  if (md.getFault())
  {
    Serial.println("fault");
    while(1);
  }
}

void setup()
{
  Serial.begin(500000);
  Serial.println("Dual MC33926 Motor Shield");
  md.init();
}

void loop()
{
  if (counter == 0){
    md.setM1Speed(400);
  }

  if (digitalRead(40) == 1 && a == false){
    counter++;
    a = true;
    //Serial.println(counter);
  }

  if (digitalRead(40) == 0 && a == true){
    counter++;
    a = false;
    //Serial.println(counter);
  }

  if (digitalRead(41) == 1 && b == false){
    counter++;
    b = true;
    //Serial.println(counter);
  }

  if (digitalRead(41) == 0 && b == true){
    counter++;
    b = false;
    //Serial.println(counter);
  }

  Serial.println(digitalRead(40));

  if (counter > (23948)){
    md.setM1Speed(0);
    if (second == 0){
      counter = 1;
      second = 1;
      delay(2000);
      md.setM1Speed(-400);
    }
  }
}
