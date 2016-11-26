#include <Wire.h>

//PINs
#define onOffButtonPin 666
#define potentiometerPin 666
#define txrxLedPin 666

#define NOISE_RANGE 10

//Adresses
#define lightsCount 2
const byte lights[2] = {8, 9};

//API Commands
enum API {
  RED = 0,
  PING = 1,
  ACK = 2,
  ON = 3,
  OFF = 4,
  GRN = 5,
  TIME = 6
};

//COLORS
enum COLORS{
  CLR_GREEN = 0,
  CLR_YELLOW = 1,
  CLR_RED = 3
};

//Status
byte currentLightColors[lightsCount] = {CLR_YELLOW, CLR_YELLOW};


void blinkTxRxLed(){
  static bool ledOn = false;
  ledOn = !ledOn;
  digitalWrite(txrxLedPin, (ledOn)?HIGH:LOW);
}

void send(byte address, byte command, int argument){
  blinkTxRxLed();
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.write((byte)argument);
  Wire.write(argument >> sizeof(byte));
  Wire.endTransmission();
  blinkTxRxLed();

  /*Messages are 3 bytes long |command|argument-lsb|argument-msb|*/
}

byte slaveAddressToLightId(byte address){
  byte i;
  for(i=0;i<lightsCount;i++){
    if(lights[i] == address)
      return i;
  }
  return 0; //In case of failure return the first light to avoid crashes
}

short handlePotentiometer(){
  /*TODO*/
}

void changeLightStatus(byte light, byte color){
  if(light < lightsCount);
  currentLightColors[light] = color;
}

void stateOff(){
  for(int i=0; i<lightsCount; i++){
    send(lights[i], OFF, 0);
  }
  delay(1000); //FIXME: Maybe this causes unresponsiveness for the On/Off button!
}

void stateOn(){
  /*TODO*/

  static int cycleLength = 0;
   
  int newCycleLength = handlePotentiometer();
  if((newCycleLength > cycleLength+NOISE_RANGE) || (newCycleLength < cycleLength-NOISE_RANGE)){
    cycleLength = newCycleLength;
    for(byte i=0;i<lightsCount;i++)
      send(lights[i], TIME, cycleLength);
  }
}

void registerACK(byte sender){
  /*TODO*/
}

void checkIncomingMessages(){
  for(byte i = 0; i<lightsCount; i++){
      //Request 3 bytes from each light
      blinkTxRxLed();
      Wire.requestFrom(lights[i], (byte)3);
      if(Wire.available() >= 3){
        byte data = Wire.read();
        byte sender = Wire.read();
        Wire.read(); //Read the padding extra byte
        
        //Execute the commands appropriatly
        switch(data){
          case RED: changeLightStatus(slaveAddressToLightId(sender), CLR_RED); break;
          case PING: send(sender, ACK, 0); break;
          case ACK: registerACK(slaveAddressToLightId(sender));
        }
      }
      //Empty the buffer if there are more bytes left
      while(Wire.available())
        Wire.read();
      blinkTxRxLed();
  }
}

bool isOn(){
  static bool On = false;
  static bool prevButtonStatus = false;

  //Flip current status if button is pressed and its status is different from the previous execution
  bool buttonStatus = digitalRead(onOffButtonPin);
  if(buttonStatus != prevButtonStatus && buttonStatus)
    On = !On;

  return On;
}

void setup() {
  pinMode(onOffButtonPin, INPUT);
  pinMode(potentiometerPin, INPUT);
  Wire.begin();

}

void loop() {
    
  checkIncomingMessages();
  
  if(isOn())
    stateOn();
  else
    stateOff();
}
