#include <QueueList.h>

#include <TimerOne.h>
#include <Wire.h>

/*
  ASE Projet
    LIGHTS
*/
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



int basicTimeUnit = 1000; //millseconds
unsigned long previousTime = 0;
static int switchTime = basicTimeUnit;
QueueList <String> cmdQueue;
int faults=0;
bool start_cycle = false;

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
        //lt=RoadFixedGREEN;
        start_cycle = true;
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
void checkLedHealth(int led){
  switch(led){
    case RoadFixedRED :
      if(!highwayRedLEDstate){
         st = ImminentDanger;
      }
      break;
    case RoadFixedYELLOW : 
      if(!highwayYellowLEDstate){
         st = ImminentDanger;
      }
      break;
    case RoadFixedGREEN : // Not decided yet what to do here
      break;
  }
}

void requestFromController(){
  if(cmdQueue.isEmpty() && st != ImminentDanger){
      pushACKStr();          //if Queue is empty send ACK by default
      String msg = cmdQueue.pop();  // get first command from queue
      Wire.write(msg.c_str());
  }
  else if (cmdQueue.isEmpty() && st == ImminentDanger);
  else{
      String msg = cmdQueue.pop();  // get first command from queue
      Wire.write(msg.c_str());
  }
}

void ping(){
   //pushPingStr();
   //faults++;
}

void setup() {
  
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  
  
  pinMode(highwayGreenLEDPin, OUTPUT);
  pinMode(highwayYellowLEDPin, OUTPUT);
  pinMode(highwayRedLEDPin, OUTPUT);
  pinMode(pedestrianRedLEDPin,OUTPUT);
  pinMode(pedestrianGreenLEDPin,OUTPUT);
  
  pinMode(pedestrianButtonPin, INPUT);
  
  pinMode(highwayRedLEDstate, INPUT);
  pinMode(highwayYellowLEDstate , INPUT);
  pinMode(highwayGreenLEDstate , INPUT);

  setLights( R_NONE );
  
  Wire.begin(8);
  Wire.onReceive(receiveCommandFromController);
  Wire.onRequest(requestFromController);
//  st=NormalFunction;lt=RoadFixedRED;
  st=ImminentDanger;lt=RoadBlinkingYELLOW1;  //Initializes Yellow Blinking lights
  //Timer1.initialize(1000000);
  //Timer1.attachInterrupt(ping, 1000000);

}

void loop() {
  static bool Pset=false;
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  static int interval = 1000;

  interval-= timeDelta;
  if(interval <= 0){
    ping();
    interval = 1000;
  }
  
  if(digitalRead(pedestrianButtonPin) == HIGH && st==NormalFunction) {
      st=PedestrianButton;
  }

  
  if(st != ImminentDanger){  //in case we're working in Normal Modes
    switch(lt){        
    case RoadFixedRED:
        if(st == PedestrianButton && !Pset){ //In case a guy presses the button, short the switching
          Pset = true;                       // time between lights!
          switchTime = (int) switchTime/2;
        }
        if(faults >=2){
            st=ImminentDanger;   //check for 2 or more faults. If yes turn danger mode on.
        }
        
        setLights( R_RED | P_GREEN);          
        checkLedHealth(RoadFixedRED);      // Red Light Fault tolerance
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0 && start_cycle){      //switch states Red->Yellow
          Pset=false;
          switchTime = basicTimeUnit;
          lt=RoadFixedYELLOW;
          setLights( R_YELLOW | P_RED);
        }
        break;
    case RoadFixedYELLOW:
        if(st == PedestrianButton && !Pset){ 
          Pset = true; 
          switchTime = (int) switchTime/2;
        }

        setLights( R_YELLOW | P_RED);
        checkLedHealth(RoadFixedYELLOW);    //Yellow light Fault tolerance
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0 && start_cycle){//Transition Yellow -> Green
          Pset=false;
          switchTime = 4*basicTimeUnit;
          lt=RoadFixedGREEN;
        }
        break;
    case RoadFixedGREEN:
        if(st == PedestrianButton && !Pset){ 
          Pset = true; 
          switchTime = (int) switchTime/2;
        }
        
        setLights( R_GREEN | P_RED);
        
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0 && start_cycle){//Transition Green -> Yellow
          Pset=false;
          switchTime = basicTimeUnit;
          lt=RoadFixedYELLOW2;
        }
        break;
    case RoadFixedYELLOW2:      
        if(st == PedestrianButton && !Pset){ 
          Pset = true; 
          switchTime = (int) switchTime/2;
        }
        setLights( R_YELLOW | P_RED);
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0 && start_cycle){//Transition Yellow -> Red
          Pset=false;       //Unset the Pedestrian flag
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
    Pset=false;
    start_cycle=false;
    //Intermitent Traffic Lights states
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
          switchTime=basicTimeUnit=1000;
          lt = RoadBlinkingYELLOW1;
    }
  }
}
