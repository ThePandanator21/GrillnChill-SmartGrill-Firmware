#include <SPI.h> //not nessary for our project, do not need to include when we merge this into our main program
#include <max6675.h>

bool  cooking = false;  // Is the grill cooking?
bool  ambtemp = false;  // Has the ambient temperature been reached?

int tProbeDIn = 25; //SPI MISO, we do not need MOSI because the thermal probes dont give a damn.
int tProbeCLK = 23; //SPI Serial Clock

//Temp Probe SPI Chip Select Vars
int tProbeAMB = 26;
int tProbeMT1 = 27;
int tProbeMT2 = 28;

//Probe Declarations
MAX6675 ambientProbe(tProbeCLK, tProbeAMB, tProbeDIn);
MAX6675 meatOneProbe(tProbeCLK, tProbeMT1, tProbeDIn);
MAX6675 meatTwoProbe(tProbeCLK, tProbeMT2, tProbeDIn);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Main Program Code");
  pinMode(50, OUTPUT);  // Setting up the pin for the blower control
}

void loop() {
  // put your main code here, to run repeatedly:

  if(cooking = true and ambtemp = false){
    digitalWrite(50, HIGH);                                                                      // Turn Blower on
    if( ambientProbe.readFahrenheit() >= targettemp){                                            // Turn off blower at target temperature
      ambtemp = true;                                                                            
      digitalWrite(50, LOW);                                                                     // Turn Blower off
    }
    
  }
}
