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
const int tempPin = A0; // Analog input pin that the temperature sensor is attached to

const int potLed = 4; // Normal pin the pot led is attached to
const int lightLed = 3; //PWM pin for the light led
const int tempLed = 2; // Normal pin the tempreature led is attached to


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(potLed, OUTPUT);
  pinMode(lightLed, OUTPUT);
  pinMode(tempLed, OUTPUT);
}

void handlePot(){
  static unsigned long previousTime = 0;
  static int potBlinkTime = 200;
  static bool potBlinkOn = true;
  
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  previousTime = currentTime;

  potBlinkTime -= timeDelta;

  if(potBlinkTime <= 0){
    potBlinkOn = !potBlinkOn;
    digitalWrite(potLed, (potBlinkOn)?HIGH:LOW);
    potBlinkTime = map(analogRead(potPin), 0, 1023, 200, 2000);

  }

}

void handleLight(){
  int lightValue = analogRead(lightPin);
  int lightLedIntensity = map(lightValue, 0, 1023, 255, 0);
  analogWrite(lightLed, lightLedIntensity);
}

void handleTemperature(){
  const int triggerTemperature = 26;
  
  int sensorValue = analogRead(tempPin);
  int temperature = (((sensorValue/1024.0)*5)-0.5)*100;
  //int temperature = (sensorValue*0.1096)-31.34; //This transformation is only for joao's temp sensor

  digitalWrite(tempLed, (temperature > triggerTemperature)?HIGH:LOW);
}

void loop() {
  handlePot();
  delay(2);
  handleLight();
  delay(2);
  handleTemperature();
  delay(2);

  // wait 2 milliseconds 
  // for the analog-to-digital converter to settle
  // after the last reading:

}
