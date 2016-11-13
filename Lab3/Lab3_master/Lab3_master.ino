#include <Wire.h>

/*
  ASE Lab 3
  Joao Neves 70171
  Luis Ribeiro Gomes ?????
  Miguel Carvalho ?????
  20/10/2016

 */


//PINs
const int potPin = A3;  // Analog input pin that the potentiometer is attached to
const int lightPin = A1;  // Analog input pin that the light sensor is attached to
const int tempPin = A0; // Analog input pin that the temperature sensor is attached to

void setup() {
  Wire.begin();
}

byte handlePot(){
  static unsigned long previousTime = 0;
  static int potBlinkTime = 200;
  static bool potBlinkOn = true;
  
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime;
  previousTime = currentTime;

  potBlinkTime -= timeDelta;

  if(potBlinkTime <= 0){
    potBlinkOn = !potBlinkOn;
    potBlinkTime = map(analogRead(potPin), 0, 1023, 200, 2000);
  }
  return (byte)potBlinkOn;
}

byte handleLight(){
  int lightValue = analogRead(lightPin);
  int lightLedIntensity = map(lightValue, 0, 1023, 255, 0);
  return lightLedIntensity;
}

byte handleTemperature(){
  const int triggerTemperature = 10;
  
  int sensorValue = analogRead(tempPin);
  int temperature = (((sensorValue/1024.0)*5)-0.5)*100;

  return (temperature > triggerTemperature)?1:0;
}

void loop() {
  Wire.beginTransmission(8);
  Wire.write(handlePot());
  Wire.write(handleLight());
  Wire.write(handleTemperature());
  Wire.endTransmission();

}
