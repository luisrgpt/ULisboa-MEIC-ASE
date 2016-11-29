#include <Wire.h>

/*
  ASE Lab 1
  Joao Neves
  70171
  13/10/2016
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

//API Commands
#define RED 0
#define PING 1
#define ACK 2
#define ON 3
#define OFF 4
#define GRN 5
#define TIME 6

enum lightsState{
  NormalFunction,    //R-Y-G-Y-R order
  PedestrianButton,  //Same order, reduced time
  ImminentDanger,    //Controller Disconnected
  test
} st;

enum LT{
  RoadFixedRED,    //R-Y order
  RoadFixedGREEN,  //G-Y
  RoadFixedYELLOW,    //Y-G
  RoadFixedYELLOW2,  //Y-R
  RoadBlinkingYELLOW1,
  RoadBlinkingYELLOW2,
} lt;



const int basicTimeUnit = 1000; //millseconds
unsigned long previousTime = 0;
static int switchTime = 4*basicTimeUnit;

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

void sendRed(){
  Wire.beginTransmission(8);
  Wire.write("RED");
  Wire.write(0);
  Wire.endTransmission();
}

void sendPing(){
  Wire.beginTransmission(8);
  Wire.write("PING");
  Wire.write(0);
  Wire.endTransmission();  
}

void sendACK(){
  Wire.beginTransmission(8);
  Wire.write("ACK");
  Wire.write(0);
  Wire.endTransmission();  
}


void receiveCommandFromController(int i){
    byte comm=Wire.read();
    int arg;
    byte lsb = Wire.read();
    byte msb = Wire.read();
    arg = (msb<<sizeof(byte) | lsb);
    switch(comm){
      case ON:
        break;
      case OFF:
        break;
      case GRN:
        st=NormalFunction;
        lt=RoadFixedRED;
        break;
      case TIME:
        switchTime=arg;
        break;
    }
}

void requestFromController(){
    
}

void setup() {
  
  Serial.begin(9600);
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
  st=NormalFunction;
  lt=RoadFixedRED;
}

void loop() {
  static bool set=false;
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  
  //st=ImminentDanger;lt=RoadBlinkingYELLOW1;  //Initializes Yellow Blinking lights
  
  if(digitalRead(pedestrianButtonPin) == HIGH) {
      //st=ImminentDanger;lt=RoadBlinkingYELLOW1;
      st=PedestrianButton;
  }
  /*switch(st){
    case test:
      digitalWrite(highwayRedLEDPin, HIGH);   
      printLedStates();
      delay(1000);
      digitalWrite(highwayRedLEDPin, LOW);
      digitalWrite(highwayYellowLEDPin, HIGH);   
      printLedStates();
      delay(1000);
      digitalWrite(highwayYellowLEDPin, LOW);
      digitalWrite(highwayGreenLEDPin, HIGH);   
      printLedStates();
      delay(1000);
      digitalWrite(highwayGreenLEDPin, LOW);
      printLedStates();
      delay(1000);
      break;
  }*/
  if(st != ImminentDanger){  //in case we're working in Normal Modes
    switch(lt){        
    case RoadFixedRED:
      if(st == PedestrianButton && !set){ //In case a guy presses the button, short the switching
        set = true;                       // time between lights!
        switchTime = (int) switchTime/2;
      }
      digitalWrite(highwayRedLEDPin, HIGH);
      digitalWrite(pedestrianGreenLEDPin, HIGH); 
      digitalWrite(highwayYellowLEDPin, LOW);   
      digitalWrite(highwayGreenLEDPin, LOW);
      digitalWrite(pedestrianRedLEDPin, LOW); 
      previousTime = currentTime;
      switchTime -= timeDelta;
      if(switchTime <=0){      //switch states Red->Yellow
        set=false;
        switchTime = basicTimeUnit;
        lt=RoadFixedYELLOW;
        digitalWrite(highwayRedLEDPin, HIGH);
        digitalWrite(highwayGreenLEDPin, LOW);
      }
      break;
    case RoadFixedYELLOW:
      if(st == PedestrianButton && !set){ 
        set = true; 
        switchTime = (int) switchTime/2;
      }
      digitalWrite(highwayRedLEDPin, LOW); 
      digitalWrite(highwayYellowLEDPin, HIGH);   
      digitalWrite(highwayGreenLEDPin, LOW);
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
      digitalWrite(highwayRedLEDPin, LOW); 
      digitalWrite(highwayYellowLEDPin, LOW);   
      digitalWrite(highwayGreenLEDPin, HIGH);
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
      digitalWrite(highwayRedLEDPin, LOW); 
      digitalWrite(highwayYellowLEDPin, HIGH);   
      digitalWrite(highwayGreenLEDPin, LOW);
      previousTime = currentTime;
      switchTime -= timeDelta;
      if(switchTime <=0){//Transition Yellow -> Red
        set=false;
        switchTime = 4*basicTimeUnit;
        st=NormalFunction;
        lt=RoadFixedRED;
        //sendRed();
      }
      break;
    }
  }
  else if (st == ImminentDanger){
    switch(lt){
      case RoadBlinkingYELLOW1:
        digitalWrite(highwayRedLEDPin, LOW);
        digitalWrite(highwayGreenLEDPin, LOW);
        digitalWrite(highwayYellowLEDPin, HIGH);   
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){
          switchTime = basicTimeUnit;
          st=ImminentDanger;
          lt=RoadBlinkingYELLOW2;
        }
        break;
      case RoadBlinkingYELLOW2:
        digitalWrite(highwayRedLEDPin, LOW);
        digitalWrite(highwayGreenLEDPin, LOW);
        digitalWrite(highwayYellowLEDPin, LOW);
        previousTime = currentTime;
        switchTime -= timeDelta;
        if(switchTime <=0){
          switchTime = basicTimeUnit;
          st=ImminentDanger;
          lt=RoadBlinkingYELLOW1;
        }
        break;
    }
  }
}
