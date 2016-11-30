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

#define I2C_Address 8

//API Commands
#define RED 0
#define PING 1
#define ACK 2
#define ON 3
#define OFF 4
#define GRN 5
#define TIME 6

//Lights mask
#define R_NONE   B0
#define R_RED    B1
#define R_YELLOW B10
#define R_GREEN  B100
#define P_RED    B1000
#define P_GREEN  B10000



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
  Wire.beginTransmission(I2C_Address);
  Wire.write("RED");
  Wire.write(I2C_Address);
  Wire.endTransmission();
}

void sendPing(){
  Wire.beginTransmission(I2C_Address);
  Wire.write("PING");
  Wire.write(I2C_Address);
  Wire.endTransmission();  
}

void sendACK(){
  Wire.beginTransmission(I2C_Address);
  Wire.write("ACK");
  Wire.write(I2C_Address);
  Wire.endTransmission();  
}

void setLights(int lmask){
  digitalWrite(highwayRedLEDPin,   (lmask && R_RED)?HIGH:LOW);
  digitalWrite(highwayGreenLEDPin, (lmask && R_GREEN)?HIGH:LOW);
  digitalWrite(highwayYellowLEDPin,(lmask && R_YELLOW)?HIGH:LOW);
  digitalWrite(pedestrianRedLEDPin,(lmask && P_RED)?HIGH:LOW);
  digitalWrite(pedestrianGreenLEDPin,(lmask && P_GREEN)?HIGH:LOW); 
  
  
}

void receiveCommandFromController(int i){
    byte comm=Wire.read();
    int arg;
    byte lsb = Wire.read();
    byte msb = Wire.read();
    arg = (msb<<8 | lsb);
    Serial.print(comm);
    Serial.print(" ");
    Serial.print(arg);
    Serial.print(" ");
    Serial.print(arg);
    
    Serial.print("\n");
    switch(comm){
      case ON:
        st=NormalFunction;
        lt=(LT) arg;
        break;
      case OFF:
        st=ImminentDanger;
        lt=RoadBlinkingYELLOW1;
        break;
      case GRN:
        st=NormalFunction;
        lt=RoadFixedRED;
        break;
      case TIME:
        switchTime=arg;
        basicTimeUnit=arg;
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
//  st=NormalFunction;
  //lt=RoadFixedRED;
  st=ImminentDanger;lt=RoadBlinkingYELLOW1;  //Initializes Yellow Blinking lights

}

void loop() {
  static bool set=false;
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  
  
  if(digitalRead(pedestrianButtonPin) == HIGH && st==NormalFunction) {
      //st=ImminentDanger;lt=RoadBlinkingYELLOW1;
      st=PedestrianButton;
      
  }
  
  if(st != ImminentDanger){  //in case we're working in Normal Modes
    switch(lt){        
    case RoadFixedRED:
        if(st == PedestrianButton && !set){ //In case a guy presses the button, short the switching
          set = true;                       // time between lights!
          switchTime = (int) switchTime/2;
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
          set=false;
          switchTime = 4*basicTimeUnit;
          st=NormalFunction;
          lt=RoadFixedRED;
          //sendRed();  @TODO
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
    }
  }
}
