#include <SPI.h> //not nessary for our project, do not need to include when we merge this into our main program
#include <max6675.h>

int tProbeDIn = 50; //SPI MISO, we do not need MOSI because the thermal probes dont give a damn.
int tProbeCLK = 52; //SPI Serial Clock

//Temp Probe SPI Chip Select Vars
int tProbeAMB = 26;
int tProbeMT1 = 27;
int tProbeMT2 = 28;

//Probe Declarations
MAX6675 ambientProbe(tProbeCLK, tProbeAMB, tProbeDIn);
MAX6675 meatOneProbe(tProbeCLK, tProbeMT1, tProbeDIn);
MAX6675 meatTwoProbe(tProbeCLK, tProbeMT2, tProbeDIn);

unsigned long tim;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("MAX6675 test");
}

void loop() {
  // put your main code here, to run repeatedly:

  tim = millis();
  if ((tim%1000) == 0) //every second
  {
     Serial.print("AMB reads F = ");
     Serial.println(ambientProbe.readFahrenheit());
     //Serial.print("Meat 1 Reads F = ");
     //Serial.println(meatOneProbe.readFahrenheit());
     //Serial.print("Meat 2 Reads F = ");
     //Serial.println(meatTwoProbe.readFahrenheit());
  }
}
