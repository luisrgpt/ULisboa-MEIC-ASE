/*
  ASE Lab 1
  Joao Neves
  70171
  13/10/2016
*/
enum lightsState{
  NormalFunction,    //R-Y-G-Y-R order
  PedestrianButton,  //Same order, reduced time
  ImminentDanger,    //Controller Disconnected
  test
} st;

const int highwayGreenLEDstate = 3;
const int highwayYellowLEDstate = 6;
const int highwayRedLEDstate = 8;

const int highwayGreenLEDPin = 2;
const int highwayYellowLEDPin = 5;
const int highwayRedLEDPin = 7;

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
  Serial.println("--------");
}

void setup() {
  
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);
  
  pinMode(8, INPUT);
  pinMode(6 , INPUT);
  pinMode(3 , INPUT);

  digitalWrite(2, LOW);
  digitalWrite(5, LOW);
  digitalWrite(7, LOW);
  
  st=ImminentDanger;
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  
  switch(st){
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
      for(int i=0; i<4;i++){
        if(event){  
        delay(basicTimeUnit);
      }
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
  }
}
