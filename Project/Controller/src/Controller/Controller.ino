#include <TimerOne.h>
#include <Wire.h>

//PINs
#define ON_OFF_BUTTON_PIN 4
#define OFF_LED_PIN 5
#define ON_LED_PIN 6
#define TX_RX_LED_PIN 7
#define POTENTIOMETER_PIN A1

#define NOISE_RANGE 100

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

//TIMEOUT (in microseconds)
#define TIMEOUT_PERIOD 10000


//Status
bool bothYellow = true;
bool confirmedRed[LIGHTS_COUNT] = {false, false};
bool confirmedAlive[LIGHTS_COUNT] = {false, false};
int  heartbeatInterval[LIGHTS_COUNT] = {0, 0};

//Function to blink the I2C LED
void updateTxRxLed(bool sending){
  static unsigned long previousTime = 0;
  static int timer = 0;
  static bool ledOn = false;
  
  if(sending && timer <= -250){
    timer = 250;
    ledOn = true;
  }else{
    unsigned long currentTime = millis();
    unsigned long timeDelta = currentTime - previousTime;
    previousTime = currentTime;
    timer -= timeDelta;

    if(timer<=0){
      ledOn = false;
    }
  }

  digitalWrite(TX_RX_LED_PIN, (ledOn)?HIGH:LOW);
}

/*Function to execute when I2C times out*/
/*HACK since Wire functions are blocking the only way to unfreeze the program 
  is to reinitialize the channel*/
void timeout(){
  Wire.begin();
}

/*Function to send commands to the slaves*/
/*Messages are 3 bytes long |command|argument-lsb|argument-msb|*/
void send(byte address, byte command, int argument){
  Timer1.attachInterrupt(timeout, TIMEOUT_PERIOD);
  updateTxRxLed(true);
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.write((byte)argument);
  Wire.write((byte)(argument >> 8));
  Wire.endTransmission();
  Timer1.detachInterrupt();
}

void string_send(byte address, byte command, int argument){
  char outputBuffer[COMMAND_BUFFER_LEN];
  switch(command){
    case PING:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_PING); break;
    case ACK:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_ACK); break;
    case ON:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s {%d}", STR_ACK, argument); break;
    case OFF:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_OFF); break;
    case GRN:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s", STR_GRN); break;
    case TIME:
      snprintf(outputBuffer, COMMAND_BUFFER_LEN, "%s {%d}", STR_TIME, argument); break;
    default:
      return; 
  }
  
  Timer1.attachInterrupt(timeout, TIMEOUT_PERIOD);
  updateTxRxLed(true);
  Wire.beginTransmission(address);
  Wire.write(outputBuffer);
  Wire.endTransmission();
  Timer1.detachInterrupt();

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

void parseCommands(byte sender, char* data){
  //Execute the commands appropriatly
  switch(data[0]){
    case RED: confirmRed(sender); break;
    case PING: send(sender, ACK, 0); break;
    case ACK: registerACK(sender);
  }
}

/*This function should be used to interface with Textual traffic lights*/
void string_parseCommands(byte sender, char* data){
  char command = -1;
  if(data[0] == 'R')
    command = RED;
  if(data[0] == 'P')
    command = PING;
  if(data[0] == 'A')
    command = ACK;

  parseCommands(sender, &command);
}

/*Check the slaves for incomming commands*/
void checkIncomingMessages(){
  for(byte i = 0; i<LIGHTS_COUNT; i++){
      //Request 3 bytes from each light
      char inputbuffer[COMMAND_BUFFER_LEN];
      Timer1.attachInterrupt(timeout, TIMEOUT_PERIOD);
      Wire.requestFrom(lights_adress[i], (byte)COMMAND_BUFFER_LEN);
      updateTxRxLed(true);
      for(byte j=0;j<COMMAND_BUFFER_LEN;j++){
        if(Wire.available()){
          inputbuffer[j] = Wire.read();
        }else{
          break;
        }
      }
      //Empty the buffer if there are more bytes left
      while(Wire.available())
        Wire.read();
      Timer1.detachInterrupt();
      
      parseCommands(lights_adress[i], inputbuffer);
  }

}

/*Shutdown the controller and the lights*/
void shutdown(){
  bothYellow = true;
  for(int i=0; i<LIGHTS_COUNT; i++){
    send(lights_adress[i], OFF, 0);
  }
}

/*Initialize a light to green and the other to red*/
void initialize(){
    bothYellow = false;
    send(lights_adress[0], ON, CLR_GREEN);
    send(lights_adress[1], ON, CLR_RED);
}

/*Funcion to check if the controller is ON or OFF*/
bool handleOnOff(){
  static bool On = false;
  static bool prevButtonStatus = false;

  //Flip current status if button is pressed and its status is different from the previous execution
  bool buttonStatus = digitalRead(ON_OFF_BUTTON_PIN);
  if(buttonStatus != prevButtonStatus && buttonStatus){
    On = !On;
    if(!On)
      shutdown();  //If the state changes to OFF then execute shutdown
  }
  prevButtonStatus = buttonStatus;

  digitalWrite(ON_LED_PIN, (On)?HIGH:LOW);
  digitalWrite(OFF_LED_PIN, (On)?LOW:HIGH);

  return On;
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
  for(byte i=0;i<LIGHTS_COUNT;i++){
    if(confirmedAlive[i]){
      confirmedAlive[i] = false;
      heartbeatInterval[i] = cycleLength;
    }
    heartbeatInterval[i] -= timeDelta;

    if(heartbeatInterval[i] <= 0){
      send(lights_adress[i], PING, 0);
    }

    if(heartbeatInterval[i] <= -(cycleLength))
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
  Timer1.initialize(1000000);

  digitalWrite(TX_RX_LED_PIN, LOW);
  digitalWrite(ON_LED_PIN, LOW);
  digitalWrite(OFF_LED_PIN, LOW);

}

void loop() {
  static unsigned long previousTime = 0;
  static int cycleLength = 0;  
  bool On = false; 
  unsigned long currentTime = millis();

  On = handleOnOff();
  if(!On){
    previousTime = currentTime;
    return;
  }  
  
  //Request data from the traffic lights
  checkIncomingMessages();
  //Update the transfer LED
  updateTxRxLed(false);
  //Recalculate the cycleInterval
  cycleLength = handleCycleAdjustment(cycleLength);
    
  //Check if we need to initialize the traffic lights
  if(bothYellow)
    initialize();

  //Everytime we receive a confirmation of RED tell the other light do begin its cycle
  if(confirmedRed[0]){
     confirmedRed[0] = false;
     send(lights_adress[1], GRN, 0);
  }else if(confirmedRed[1]){
     confirmedRed[1] = false;
     send(lights_adress[0], GRN, 0);
  }

  //Check if all traffic lights are alive
  checkHeartBeat(cycleLength, currentTime-previousTime);


  previousTime = currentTime;
}
