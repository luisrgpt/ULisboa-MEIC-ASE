#include <Wire.h>

/*
  ASE Projet
    CONTROLLER
*/

//PINs
#define ON_OFF_BUTTON_PIN 4
#define OFF_LED_PIN 5
#define ON_LED_PIN 6
#define TX_RX_LED_PIN 7
#define POTENTIOMETER_PIN A1

#define NOISE_RANGE 100
#define LED_BLINK_DURATION 250 //in milliseconds

//Adresses
#define LIGHTS_COUNT 2
const byte lights_adress[LIGHTS_COUNT] = {8, 9};

//Cycle durations
#define MIN_CYCLE 2000 //in milliseconds
#define MAX_CYCLE 10000 //in milliseconds

//API Commands
#define RED 0
#define PING 1
#define ACK 2
#define ON 3
#define OFF 4
#define GRN 5
#define TIME 6

#define STR_RED "RED"
#define STR_PING "PING"
#define STR_ACK "ACK"
#define STR_ON "ON"
#define STR_OFF "OFF"
#define STR_GRN "GRN"
#define STR_TIME "TIME"

#define COMMAND_BUFFER_LEN 15

//COLORS
#define CLR_GREEN 1
#define CLR_YELLOW 2
#define CLR_RED 3


//Status
bool On = false;
bool confirmedRed[LIGHTS_COUNT] = {false, false};
bool confirmedAlive[LIGHTS_COUNT] = {false, false};
int  heartbeatTimeout[LIGHTS_COUNT] = {MIN_CYCLE, MIN_CYCLE};

//Function to blink the I2C LED
void updateTxRxLed(bool sending){
  static unsigned long previousTime = 0;
  static int timer = 0;
  static bool ledOn = false;
  
  if(sending && timer <= -(LED_BLINK_DURATION)){ //If the Led has been off for at least DURATION
    timer = LED_BLINK_DURATION; //Set it to be on for DURATION
    ledOn = true;
  }
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  previousTime = currentTime;
  timer -= timeDelta;

  if(timer<=0){
    ledOn = false;
  }
  digitalWrite(TX_RX_LED_PIN, (ledOn)?HIGH:LOW);
}

/*Function to send commands to the slaves*/
/*Messages are 3 bytes long |command|argument-lsb|argument-msb|*/
void sendBytes(byte address, byte command, int argument){
  updateTxRxLed(true);
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.write((byte)argument);
  Wire.write((byte)(argument >> 8));
  Wire.endTransmission();
}

/*Function to send commands to the slaves*/
/*String implementation for compatibility*/
void send(byte address, byte command, int argument){
  char outputBuffer[COMMAND_BUFFER_LEN];
  switch(command){
    case PING:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_PING); break;
    case ACK:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_ACK); break;
    case ON:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s {%d}", STR_ON, argument); break;
    case OFF:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_OFF); break;
    case GRN:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_GRN); break;
    case TIME:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s {%d}", STR_TIME, argument); break;
    default:
      return; 
  }
  //Serial.println(outputBuffer);
  updateTxRxLed(true);
  Wire.beginTransmission(address);
  Wire.write(outputBuffer);
  Wire.endTransmission();

}

/*Convert a slave address to an index for ligths arrays*/
byte slaveAddressToLightId(byte address){
  byte i;
  for(i=0;i<LIGHTS_COUNT;i++){
    if(lights_adress[i] == address)
      return i;
  }
  return 0; //In case of failure return the first light to avoid crashes
}



void registerACK(byte sender){
  confirmedAlive[slaveAddressToLightId(sender)] = true;
}

void confirmRed(byte sender){
  confirmedRed[slaveAddressToLightId(sender)] = true;
}

/*Parse the commands received from traffic lights*/
void parseCommands(byte sender, byte data){
  //Execute the commands appropriatly
  switch(data){
    case RED: confirmRed(sender); break;
    case PING: send(sender, ACK, 0); break;
    case ACK: registerACK(sender);
  }
}

/*Parse the commands received from traffic lights*/
/*String version for compatibility*/
void string_parseCommands(byte sender, char* data){
  char command = -1;
  if(data[0] == 'R')
    command = RED;
  if(data[0] == 'P')
    command = PING;
  if(data[0] == 'A')
    command = ACK;

  parseCommands(sender, command);
}

/*Check the slaves for incomming commands*/
void checkIncomingMessages(){
  for(byte i = 0; i<LIGHTS_COUNT; i++){
      
      char inputbuffer[COMMAND_BUFFER_LEN];
      inputbuffer[0] = 0;
      Wire.requestFrom(lights_adress[i], (byte)COMMAND_BUFFER_LEN);
      if(Wire.available())
        updateTxRxLed(true);
      
      for(byte j=0;j<COMMAND_BUFFER_LEN;j++){
        if(Wire.available()){
          inputbuffer[j] = Wire.read();
        }else{
          break;
        }
      }
      //Empty the buffer if there are more bytes left
      while(Wire.available()){
        Wire.read();
      }
      
      if(inputbuffer[0] != 0)
        string_parseCommands(lights_adress[i], inputbuffer);
  }

}

/*Shutdown the controller and the lights*/
void shutdown(){
  for(int i=0; i<LIGHTS_COUNT; i++){
    send(lights_adress[i], OFF, 0);
  }

  //Reset our state
  On = false;
  
  heartbeatTimeout[0] = MIN_CYCLE;
  heartbeatTimeout[1] = MIN_CYCLE;

  confirmedRed[0] = false;
  confirmedRed[1] = false;

  confirmedAlive[0] = false;
  confirmedAlive[1] = false;
}

/*Initialize a light to green and the other to red*/
void initialize(){
    send(lights_adress[0], ON, CLR_GREEN);
    send(lights_adress[1], ON, CLR_RED);
}

/*Funcion to check if the controller is ON or OFF*/
void handleOnOff(){
  static bool prevButtonStatus = false;

  //Flip current status if button is pressed and its status is different from the previous execution
  bool buttonStatus = digitalRead(ON_OFF_BUTTON_PIN);
  if(buttonStatus != prevButtonStatus && buttonStatus){
    On = !On;
    if(On)
      initialize(); //If the state changes to ON then initialize the traffic lights
    else
      shutdown();  //If the state changes to OFF then shutdown the traffic lights
  }
  prevButtonStatus = buttonStatus;

  digitalWrite(ON_LED_PIN, (On)?HIGH:LOW);
  digitalWrite(OFF_LED_PIN, (On)?LOW:HIGH);
}

/*Check the Potentiometer for cycle interval changes*/
int handleCycleAdjustment(int currentCycleLength){
  
  int newCycleLength = map(analogRead(POTENTIOMETER_PIN), 0, 1023, MIN_CYCLE, MAX_CYCLE);

  //Ignore changes < 100ms possibly caused by noise
  if((newCycleLength > currentCycleLength+NOISE_RANGE) || (newCycleLength < currentCycleLength-NOISE_RANGE)){
    for(byte i=0;i<LIGHTS_COUNT;i++)
      send(lights_adress[i], TIME, newCycleLength); //Update the traffic lights
    return newCycleLength;
  }
  return currentCycleLength;
}

/*Check if all traffic lights are responding
  if they dont respond by them selves for 1 cycle ping them
  if they dont respond for 2 cycles shut everything down*/
void checkHeartBeat(int cycleLength, int timeDelta){
  for(byte i=0;i<LIGHTS_COUNT;i++){   //Comment this line to work with just one traffic light
  //for(byte i=0;i<1;i++){            //uncomment this line to work with just one traffic light
    if(confirmedAlive[i]){
      confirmedAlive[i] = false;
      heartbeatTimeout[i] = cycleLength;
    }
    heartbeatTimeout[i] -= timeDelta;

    if(heartbeatTimeout[i] <= 0){
      send(lights_adress[i], PING, 0);
    }

    if(heartbeatTimeout[i] <= -(cycleLength))
      shutdown();
  }
}

void setup() {
  pinMode(ON_OFF_BUTTON_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(TX_RX_LED_PIN, OUTPUT);
  pinMode(ON_LED_PIN, OUTPUT);
  pinMode(OFF_LED_PIN, OUTPUT);
  Wire.begin();
  Serial.begin(9600);

  digitalWrite(TX_RX_LED_PIN, LOW);
  digitalWrite(ON_LED_PIN, LOW);
  digitalWrite(OFF_LED_PIN, LOW);

}

void loop() {
  static unsigned long previousTime = 0;
  static int cycleLength = 0;  
  unsigned long currentTime;

  currentTime = millis();
  
  //Handle the ON/OFF Button
  handleOnOff();

  //Update the transfer LED
  updateTxRxLed(false);
  
  if(!On){
    previousTime = currentTime;
    return;
  }
  //The rest of the code is only executed if the Controller is ON
  
  //Request data from the traffic lights
  checkIncomingMessages();

  //Recalculate the cycleInterval
  cycleLength = handleCycleAdjustment(cycleLength);
    
  //Everytime we receive a confirmation of RED tell the other light to begin its cycle
  if(confirmedRed[0]){
     confirmedRed[0] = false;
     //send(lights_adress[0], GRN, 0);//Uncomment this line to work with just one traffic light 
     send(lights_adress[1], GRN, 0);  //Comment this line to work with just one traffic light
  }else if(confirmedRed[1]){
     confirmedRed[1] = false;
     send(lights_adress[0], GRN, 0);
  }

  //Check if all traffic lights are alive
  checkHeartBeat(cycleLength, currentTime-previousTime);

  previousTime = currentTime;
}
