
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
  long temperature;
  int rotate = 0;
  int timer = 0;
  String food;
  //delay(5000);
  if (Serial1.available() > 0) {
    //Serial.write("Serial1 was available at some point");
    //inByte =Serial1.read();
    food = String(Serial1.readString());
    String test = food;
    food.trim();
    Serial.print(food);
    if (food == "Chicken")
    {
      temperature = 300;
      timer = 480;
      rotate = 240;
    }
    else if ( food == "Steak" )
    {
      temperature = 350;
      timer = 300;
      rotate = 150;
    }
    
    //temperature = Serial1.parseInt();
    //Serial.write(inByte);
    if (temperature != 0)
    {
    Serial.print("The temperature is: ");
    Serial.println(temperature);
    }
    //delay(500);
  }

  // (pc to bt mobile) read from port 0, send to port 1:
  
  if (Serial.available()) {
    inByte = Serial.read();
    Serial1.write(inByte);
  }
  
}
