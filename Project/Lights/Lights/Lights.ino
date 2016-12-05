#include <QueueList.h>

#include <TimerOne.h>
#include <Wire.h>

/*
  ASE Projet
    LIGHTS
*/
#define TIMEOUT_PERIOD 10000

#define highwayGreenLEDstate 3
#define highwayYellowLEDstate 6
#define highwayRedLEDstate 8

#define highwayGreenLEDPin 2
#define highwayYellowLEDPin 5
#define highwayRedLEDPin 7

#define pedestrianRedLEDPin 10
#define pedestrianGreenLEDPin 11

#define pedestrianButtonPin 13

#define I2C_Address 8

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

//Lights mask
#define R_NONE   B0
#define R_RED    B1
#define R_YELLOW B10
#define R_GREEN  B100
#define P_RED    B1000
#define P_GREEN  B10000

#define COMMAND_BUFFER_LEN 15

enum lightsState{
  NormalFunction,    //R-Y-G-Y-R order
  PedestrianButton,  //Same order, reduced time
  ImminentDanger,    //Controller Disconnected
  test
} st;

enum LT{
  RoadFixedRED=1,    //R-Y order
  RoadFixedGREEN=2,  //G-Y
  RoadFixedYELLOW=3,    //Y-G
  RoadFixedYELLOW2=4,  //Y-R
  RoadBlinkingYELLOW1=5,
  RoadBlinkingYELLOW2=6,
} lt;



/****************DEBUG*******************/
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}


void printLedStates(){
  Serial.println("--------");
  Serial.print("Red Light->");Serial.print(digitalRead(highwayRedLEDstate));
  Serial.print("\n");
  Serial.print("Yellow Light->");Serial.print(digitalRead(highwayYellowLEDstate));
  Serial.print("\n");
  Serial.print("Green Light->");Serial.print(digitalRead(highwayGreenLEDstate));
  Serial.print("\n");
  Serial.print("Pedestrian->");Serial.print(digitalRead(13));
  Serial.print("\n");
  Serial.println("--------");
}

/****************DEBUG*****************/


int basicTimeUnit = 1000; //millseconds
unsigned long previousTime = 0;
static int switchTime = basicTimeUnit;
QueueList <String> cmdQueue;
int faults=0;

void pushRedStr(){
  String a = STR_RED ;
  a+=" {";
  a+=I2C_Address;
  a+="}";
  cmdQueue.push(a); //"RED {i2c_address}"
}

void pushPingStr(){
  String a = STR_PING;
  a+=" {";
  a+=I2C_Address;
  a+="}";  
  cmdQueue.push(a); //"PING {i2c_address}"
}

void pushACKStr(){
  String a = STR_ACK;
  a+=" {";
  a+=I2C_Address;
  a+="}";  
  cmdQueue.push(a); //"ACK {i2c_address}"
}

void setLights(int lmask){
 // Serial.println(lmask);
  digitalWrite(highwayRedLEDPin,   (lmask & R_RED)?HIGH:LOW);
  digitalWrite(highwayGreenLEDPin, (lmask & R_GREEN)?HIGH:LOW);
  digitalWrite(highwayYellowLEDPin,(lmask & R_YELLOW)?HIGH:LOW);
  digitalWrite(pedestrianRedLEDPin,(lmask & P_RED)?HIGH:LOW);
  digitalWrite(pedestrianGreenLEDPin,(lmask & P_GREEN)?HIGH:LOW);
}

void execComm(int comm, int arg){
  switch(comm){
      case ON:
        st=NormalFunction;
        lt=(LT) arg;
        if(lt == RoadFixedRED) {pushRedStr();}
        break;
      case OFF:
        st=ImminentDanger;
        lt=RoadBlinkingYELLOW1;
        break;
      case GRN:
        st=NormalFunction;
        lt=RoadFixedGREEN;
        break;
      case TIME:
        switchTime=arg;
        basicTimeUnit=arg;
        Serial.println(arg); 
        break;
       case ACK:
        faults = 0;    //Here when receiving an alive signal from the microcontroller we reset the fault counter.
        break;
       case PING:
        pushACKStr();
        break;
       default:
        Serial.println("errocmd");
        break;
    }
}

void receiveCommandFromController(int bytesReceived){
    byte command[COMMAND_BUFFER_LEN];
    int comm=-1;
    int arg = 0;
    Serial.println("--Comm Received--");
    Wire.readBytes(command, bytesReceived);
    Serial.println((char*)command);
    if (command[0] == 'T') {
      comm=TIME;
      char time[6];
      for(int i = 6; i<11; i++){  //Get TIME argument from received bytes
        if(command[i] != '}') 
          time[i-6] = command[i];
        else{
          time[i] = '\0';
          break;
        }
      }
      arg = atoi(time);          // Convert to int

    }
    else if (command[0] == 'G') comm=GRN;
    else if (command[0] == 'A') comm=ACK;
    else if (command[0] == 'O' && command[1] == 'F') comm=OFF;
    else if (command[0] == 'O' && command[1] == 'N'){ 
      comm=ON;
      arg = (command[4] - '0');
    }
    else if (command[0] == 'P') comm=PING;
    
    execComm(comm,arg);

  
    
}
/*void commandHandle(char* buffer){
  Wire.beginTransmission(I2C_Address);
  Wire.write("RED");
  Wire.write(I2C_Address);
  Wire.endTransmission();
}*/

void sendPing(){
  Wire.beginTransmission(I2C_Address);
  Wire.write(PING);
  Wire.write(I2C_Address);
  Wire.endTransmission();
}

void requestFromController(){
  if(cmdQueue.isEmpty() && st != ImminentDanger)
      pushACKStr();          //if Queue is empty send ACK by default
  
  String msg = cmdQueue.pop();  // get first command from queue
  
  //Wire.beginTransmission(I2C_Address);
  Wire.write(msg.c_str());
  //Wire.endTransmission();      // Send that command
}

void setup() {
  
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);  //DEBUG
  
  pinMode(highwayGreenLEDPin, OUTPUT);
  pinMode(highwayYellowLEDPin, OUTPUT);
  pinMode(highwayRedLEDPin, OUTPUT);
  pinMode(pedestrianRedLEDPin,OUTPUT);
  pinMode(pedestrianGreenLEDPin,OUTPUT);
  
  pinMode(pedestrianButtonPin, INPUT);
  
  pinMode(highwayRedLEDstate, INPUT);
  pinMode(highwayYellowLEDstate , INPUT);
  pinMode(highwayGreenLEDstate , INPUT);

  digitalWrite(highwayGreenLEDPin, LOW);
  digitalWrite(highwayYellowLEDPin, LOW);
  digitalWrite(highwayRedLEDPin, LOW);
  digitalWrite(pedestrianGreenLEDPin,LOW);
  digitalWrite(pedestrianRedLEDPin,LOW);
  
  Wire.begin(8);
  Wire.onReceive(receiveCommandFromController);
  Wire.onRequest(requestFromController);
//  st=NormalFunction;lt=RoadFixedRED;
  st=ImminentDanger;lt=RoadBlinkingYELLOW1;  //Initializes Yellow Blinking lights

}

void loop() {
  static bool set=false;
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  
  
  if(digitalRead(pedestrianButtonPin) == HIGH && st==NormalFunction) {
      st=PedestrianButton;
  }

  
  if(st != ImminentDanger){  //in case we're working in Normal Modes
    switch(lt){        
    case RoadFixedRED:
        if(st == PedestrianButton && !set){ //In case a guy presses the button, short the switching
          set = true;                       // time between lights!
          switchTime = (int) switchTime/2;
        }
        if(faults >=2){
            st=ImminentDanger;lt=RoadBlinkingYELLOW1;   //check for 2 or more faults. If yes turn danger mode on.
        }
        setLights( R_RED | P_GREEN);          
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){      //switch states Red->Yellow
          set=false;
          switchTime = basicTimeUnit;
          lt=RoadFixedYELLOW;
          setLights( R_YELLOW | P_RED);
        }
        break;
    case RoadFixedYELLOW:
        if(st == PedestrianButton && !set){ 
          set = true; 
          switchTime = (int) switchTime/2;
        }

        setLights( R_YELLOW | P_RED);
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){//Transition Yellow -> Green
          set=false;
          switchTime = 4*basicTimeUnit;
          lt=RoadFixedGREEN;
        }
        break;
    case RoadFixedGREEN:
        if(st == PedestrianButton && !set){ 
          set = true; 
          switchTime = (int) switchTime/2;
        }
        
        setLights( R_GREEN | P_RED);
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){//Transition Green -> Yellow
          set=false;
          switchTime = basicTimeUnit;
          lt=RoadFixedYELLOW2;
        }
        break;
    case RoadFixedYELLOW2:      
        if(st == PedestrianButton && !set){ 
          set = true; 
          switchTime = (int) switchTime/2;
        }
        setLights( R_YELLOW | P_RED);
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){//Transition Yellow -> Red
          set=false;       //Unset the Pedestrian flag
          switchTime = 4*basicTimeUnit;
          st=NormalFunction;
          lt=RoadFixedRED;    //restart Cycle
          pushRedStr();       //Push RED to the microcontroller
          faults++;           //Increment faults counter.
          pushPingStr();      //But also check if we really have faults.
          //sendPing();  
      }
        break;
    }
  }
  else if (st == ImminentDanger){
    set=false;
    switch(lt){
        case RoadBlinkingYELLOW1:
          setLights( R_YELLOW );
          previousTime = currentTime;
          switchTime -= timeDelta;
          if(switchTime <=0){
            switchTime = basicTimeUnit;
            st=ImminentDanger;
            lt=RoadBlinkingYELLOW2;
          }
          break;
      case RoadBlinkingYELLOW2:
          setLights( R_NONE );
          previousTime = currentTime;
          switchTime -= timeDelta;
          if(switchTime <=0){
            switchTime = basicTimeUnit;
            st=ImminentDanger;
            lt=RoadBlinkingYELLOW1;
          }
          break;
      default:
          lt = RoadBlinkingYELLOW1;
    }
  }
  
  
  
  
  
   if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    char arg0 = inputString[0];
    char arg1 = (inputString[1]!=0)?inputString[1]:0;
    st=(arg0 == 'N')?NormalFunction:ImminentDanger;
    if(st == NormalFunction) lt=(LT) (arg1 - '0');
    else if(st == ImminentDanger) lt = RoadBlinkingYELLOW1;
    inputString = "";
    stringComplete = false;
  }
}
