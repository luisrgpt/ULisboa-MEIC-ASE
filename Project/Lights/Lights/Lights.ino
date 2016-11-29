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

#define pedestrianButtonPin 13
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
  RoadBlinkingYELLOW,
} lt;



const int basicTimeUnit = 1000; //millseconds
unsigned long previousTime = 0;

void printLedStates(){
//  Serial.println((float)4000/3);
  Serial.println("--------");
  Serial.print("Red Light->");Serial.print(digitalRead(highwayRedLEDstate));
  Serial.print("\n");
  Serial.print("Yellow Light->");Serial.print(digitalRead(highwayYellowLEDstate));
  Serial.print("\n");
  Serial.print("Green Light->");Serial.print(digitalRead(highwayGreenLEDstate));
  Serial.print("\n");
  Serial.print("Pin13->");Serial.print(digitalRead(13));
  Serial.print("\n");
  Serial.println("--------");
}

void setup() {
  
  Serial.begin(9600);
  pinMode(highwayGreenLEDPin, OUTPUT);
  pinMode(highwayYellowLEDPin, OUTPUT);
  pinMode(highwayRedLEDPin, OUTPUT);
  
  pinMode(13, INPUT);
  pinMode(8, INPUT);
  pinMode(6 , INPUT);
  pinMode(3 , INPUT);

  digitalWrite(2, LOW);
  digitalWrite(5, LOW);
  digitalWrite(7, LOW);
 // digitalWrite(13, LOW);
  
  st=ImminentDanger;
  lt=RoadFixedRED;
}

void loop() {
  static bool set=false;
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  static int switchTime = 4*basicTimeUnit;
  
  if(digitalRead(pedestrianButtonPin) == HIGH) st=PedestrianButton;
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
    case NormalFunction:
      digitalWrite(highwayRedLEDPin, HIGH);
      digitalWrite(highwayRedLEDPin, LOW);
      digitalWrite(highwayYellowLEDPin, HIGH);   
      delay(basicTimeUnit);
      digitalWrite(highwayYellowLEDPin, LOW);
      digitalWrite(highwayGreenLEDPin, HIGH);   
      printLedStates();
      delay(4*basicTimeUnit);
      digitalWrite(highwayGreenLEDPin, LOW);
      digitalWrite(highwayRedLEDPin, HIGH); 
      break;
    case ImminentDanger:
      digitalWrite(highwayRedLEDPin, LOW);
      digitalWrite(highwayGreenLEDPin, LOW);
      digitalWrite(highwayYellowLEDPin, HIGH);   
      delay(basicTimeUnit);
      digitalWrite(highwayYellowLEDPin, LOW);
      delay(basicTimeUnit);
      break;
  }*/
  
    switch(lt){
    case RoadFixedRED:
      if(st == PedestrianButton && !set){ 
        set = true; 
        switchTime = (int) switchTime/2;
      }
      digitalWrite(highwayRedLEDPin, HIGH); 
      digitalWrite(highwayYellowLEDPin, LOW);   
      digitalWrite(highwayGreenLEDPin, LOW);
      previousTime = currentTime;
      switchTime -= timeDelta;
      if(switchTime <=0){
        set=false;
        switchTime = basicTimeUnit;
        lt=RoadFixedYELLOW;
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
      if(switchTime <=0){
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
      if(switchTime <=0){
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
      if(switchTime <=0){
        set=false;
        switchTime = 4*basicTimeUnit;
        st=NormalFunction;
        lt=RoadFixedRED;
      }
      break;
    }
    
 }
