#include <Wire.h>

//PINs
#define ON_OFF_BUTTON_PIN 666
#define POTENTIOMETER_PIN 666
#define TX_RX_LED_PIN 666

#define NOISE_RANGE 10

//Adresses
#define LIGHTS_COUNT 2
const byte lights_adress[LIGHTS_COUNT] = {8, 9};

//Cycle durations
#define MIN_CYCLE 2000
#define MAX_CYCLE 10000

//API Commands
#define RED 0
#define PING 1
#define ACK 2
#define ON 3
#define OFF 4
#define GRN 5
#define TIME 6

//COLORS
#define CLR_GREEN 0
#define CLR_YELLOW 1
#define CLR_RED 3


//Status
byte currentLightColors[LIGHTS_COUNT] = {CLR_YELLOW, CLR_YELLOW};


void blinkTxRxLed(){
  static bool ledOn = false;
  ledOn = !ledOn;
  digitalWrite(TX_RX_LED_PIN, (ledOn)?HIGH:LOW);
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
  for(i=0;i<LIGHTS_COUNT;i++){
    if(lights_adress[i] == address)
      return i;
  }
  return 0; //In case of failure return the first light to avoid crashes
}

void changeLightStatus(byte light, byte color){
  if(light < LIGHTS_COUNT)
    currentLightColors[light] = color;
}

void stateOff(){
  for(int i=0; i<LIGHTS_COUNT; i++){
    send(lights_adress[i], OFF, 0);
  }
  delay(1000); //FIXME: Maybe this causes unresponsiveness for the On/Off button!
}

void stateOn(){
  /*TODO*/

  static int cycleLength = 0;

  //Check the potentiometer for changes and if necessary update the cycle on the traffic lights
  int newCycleLength = map(analogRead(POTENTIOMETER_PIN), 0, 1023, MIN_CYCLE, MAX_CYCLE);

  //Ignore changes < 10ms possibly caused by noise
  if((newCycleLength > cycleLength+NOISE_RANGE) || (newCycleLength < cycleLength-NOISE_RANGE)){
    cycleLength = newCycleLength;
    for(byte i=0;i<LIGHTS_COUNT;i++)
      send(lights_adress[i], TIME, cycleLength); //Update the traffic lights
  }
  
}

void registerACK(byte sender){
  /*TODO*/
}

void checkIncomingMessages(){
  for(byte i = 0; i<LIGHTS_COUNT; i++){
      //Request 3 bytes from each light
      blinkTxRxLed();
      Wire.requestFrom(lights_adress[i], (byte)3);
      if(Wire.available() >= 3){
        byte data = Wire.read();
        byte sender = Wire.read();
        Wire.read(); //Read extra byte (should be 0)
        
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
  bool buttonStatus = digitalRead(ON_OFF_BUTTON_PIN);
  if(buttonStatus != prevButtonStatus && buttonStatus)
    On = !On;

  return On;
}

void setup() {
  pinMode(ON_OFF_BUTTON_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  Wire.begin();

}

void loop() {
    
  checkIncomingMessages();
  
  if(isOn())
    stateOn();
  else
    stateOff();
}
