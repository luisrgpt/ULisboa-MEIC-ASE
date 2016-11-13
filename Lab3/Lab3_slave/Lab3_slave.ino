#include <Wire.h>

/*
  ASE Lab 2
  Joao Neves 70171
  Luis Ribeiro Gomes ?????
  Miguel Carvalho ?????
  20/10/2016

 */

const int potLed = 4; // Normal pin the pot led is attached to
const int lightLed = 3; //PWM pin for the light led
const int tempLed = 2; // Normal pin the tempreature led is attached to

void callbackFunction(int i){
  byte potBlinkOn = Wire.read();
  byte lightLedIntensity = Wire.read();
  byte tempOn = Wire.read();
  
  digitalWrite(potLed, (potBlinkOn)?HIGH:LOW);
  analogWrite(lightLed, lightLedIntensity);
  digitalWrite(tempLed, tempOn);
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Wire.begin(8);
  Wire.onReceive(callbackFunction);
  
  pinMode(potLed, OUTPUT);
  pinMode(lightLed, OUTPUT);
  pinMode(tempLed, OUTPUT);
}

void loop() {
  delay(1000);
}
