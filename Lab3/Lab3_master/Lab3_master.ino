#include <Wire.h>

/*
  ASE Lab 3
  Joao Neves 70171
  Luis Ribeiro Gomes ?????
  Miguel Carvalho 77034
  15/11/2016

 */

/*METER FORA DO CODIGO
 *  MESTRE - Escravo
 *  Mestre ligado aos sensores e calcula o valor para os leds (faz a logica toda)
 *  Escravo liagado aos leds (limita-se a escrever o que le)
 * 
 */


/*
 * The funcions read* read the values from the sensors and return the value that should be written to the led port
 * The loop calculates all values and then sends a single transmission with all values. Each value is a single byte.
 */


//PINs
const int potPin = A3;  // Analog input pin that the potentiometer is attached to
const int lightPin = A1;  // Analog input pin that the light sensor is attached to
const int tempPin = A0; // Analog input pin that the temperature sensor is attached to

void setup() {
  Wire.begin();
}

byte readPot(){
  static unsigned long previousTime = 0;
  static int potBlinkTime = 200;
  static bool potBlinkOn = true;
  
  unsigned long currentTime = millis();
  unsigned long timeDelta = currentTime - previousTime; //Time elapsed between executions of this function
  previousTime = currentTime;

  potBlinkTime -= timeDelta;

  if(potBlinkTime <= 0){ //If enough time has elapsed flip the state of the LED and sample the potentiometer again
    potBlinkOn = !potBlinkOn;
    potBlinkTime = map(analogRead(potPin), 0, 1023, 200, 2000);
  }
  return (byte)potBlinkOn;
}

byte readLight(){
  int lightValue = analogRead(lightPin);
  int lightLedIntensity = map(lightValue, 0, 1023, 255, 0);
  return lightLedIntensity;
}

byte readTemperature(){
  const int triggerTemperature = 26;
  static float avg = 0;
  const float SAMPLES = 15.0;
  
  int sensorValue = analogRead(tempPin);
  int temperature = (((sensorValue/1024.0)*5)-0.5)*100;

  //Here we do a moving average to smooth spikes in temperature
  avg -= avg/SAMPLES;
  avg += (float)temperature/SAMPLES;

  return (avg > triggerTemperature)?1:0;
}

void loop() {
  byte potValue = readPot();
  byte lightValue = readLight();
  byte temperatureValue = readTemperature();
  
  Wire.beginTransmission(8);
  Wire.write(potValue);
  Wire.write(lightValue);
  Wire.write(temperatureValue);
  Wire.endTransmission();

}
