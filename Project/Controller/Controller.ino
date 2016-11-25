#include <Wire.h>

//PINs
const byte onOffButtonPin = 666;
const byte potentiometerPin = 666;

//Adresses
const byte lightsCount = 2;
const byte lights[2] = {8, 9};



void send(byte address, char* data){
  Wire.beginTransmission(address);
  Wire.write(data);
  Wire.endTransmission();  
}

/*NOT SURE IF THIS ACTUALLY WORKS*/
void checkIncomingMessages(){
  for(byte i = 0; i<lightsCount; i++){
      Wire.requestFrom(lights[i], 1);
      while(Wire.available()){
        
      }
  }
}

short handlePotentiometer(){
  /*TODO*/
}

void stateOff(){
  for(int i=0; i<lightsCount; i++){
    send(lights[i], "OFF");
  }
  delay(1000);
}

void stateOn(){
  /*TODO*/
}


void* handleOnOffButton(){
  static bool On = false;
  static bool prevButtonStatus = false;

  bool buttonStatus = digitalRead(onOffButtonPin);
  if(buttonStatus != prevButtonStatus && buttonStatus)
    On = !On;

  return (On)? &stateOn: &stateOff;
}

void setup() {
  pinMode(onOffButtonPin, INPUT);
  Wire.begin();

}

void loop() {
  void (*currentState)();

  currentState = handleOnOffButton();
  
  checkIncomingMessages();
  currentState();
  
}
