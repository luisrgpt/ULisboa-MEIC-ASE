/*
  ASE Lab 2
  Joao Neves 70171
  Luis Ribeiro Gomes ?????
  Miguel Carvalho ?????
  20/10/2016

 */

//PINs
const int potPin = A3;  // Analog input pin that the potentiometer is attached to
const int lightPin = A1;  // Analog input pin that the light sensor is attached to

const int potLed = 3; // PWM pin the pot led is attached to
const int lightLed = 5; //Normal pin for the light led

//Global vars
unsigned long previousTime = 0;
int previousPotValue = 0;
int potBlinkTime = 200;
bool potBlinkOn = true;


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(potLed, OUTPUT);
  pinMode(lightLed, OUTPUT);
}

void handlePot(){
  int potValue = map(analogRead(potPin), 512, 1023, 200, 2000);
    
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  previousTime = currentTime;

  potBlinkTime -= timeDelta;

  if(potBlinkTime <= 0){
    potBlinkOn = !potBlinkOn;
    digitalWrite(potLed, (potBlinkOn)?HIGH:LOW);
    potBlinkTime = potValue;
  }

  if(abs(potValue-previousPotValue) > 5){
    previousPotValue = potValue;
  }

}

void handleLight(){
  int lightValue = analogRead(lightPin);
  int lightLedIntensity = map(lightValue, 0, 1023, 0, 255);
  analogWrite(lightLed, lightLedIntensity);
}

void loop() {
  handlePot();
  handleLight();

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);
}
