//LED pin constants
const int yellowLEDPin = 4;
const int redLEDPin    = 3;
const int greenLEDPin  = 2;

//Sensor pin constants
const int temperatureSensorPin = A0;
const int lightSensorPin       = A1;
const int presenceSensorPin    = A2;

//Loop constants
const int delayTime = 2/*milliseconds*/;

float mapTemperature(int temperatureSensorValue) {
  return (((temperatureSensorValue/1024.0) * 5.0) - 0.5) * 100;
}

float mapLight(int lightSensorValue) {
  return map(lightSensorValue,0,1023,0,255);
}

float mapPresence(int presenceSensorValue) {
  return map(presenceSensorValue,0,1023,0,179);
}

void manageYellowLED(float temperature) {
  //If environment temperature is greater than 26ºC
  if(temperature > 26.0/*ºC*/) {
    //Activate yellow led pin
    digitalWrite(yellowLEDPin, HIGH);
  } else {
    //Deactivate yellow led pin
    digitalWrite(yellowLEDPin, LOW);
  }
}

void manageRedLED(float presence) {
  float wait = map(presence,0,179,200,2000);
  digitalWrite(redLEDPin, HIGH);
  delay(wait);
  digitalWrite(redLEDPin, LOW);
  delay(wait);
}

void manageGreenLED(float light) {
  //Regulate green led pin
  analogWrite(greenLEDPin, light);
}

void setup() {
  //Setting LED pins to output voltage
  pinMode(yellowLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);

  //Reset LED pins output to low voltage, deactivating them
  digitalWrite(yellowLEDPin, LOW);
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, LOW);

  //DEBUG
  /*Serial.begin(9600);*/
}

void loop() {
  //Getting sensor values 
  int temperatureSensorValue = analogRead(temperatureSensorPin);
  int lightSensorValue       = analogRead(lightSensorPin);
  int presenceSensorValue    = analogRead(presenceSensorPin);

  //DEBUG
  /*Serial.print("Temperature sensor value: ");
  Serial.println(temperatureSensorValue);
  Serial.print("Light       sensor value: ");
  Serial.println(lightSensorValue);
  Serial.print("Presence    sensor value: ");
  Serial.println(presenceSensorValue);*/

  //Mapping environment variables
  float temperature = mapTemperature(temperatureSensorValue);
  float light       = mapLight(lightSensorValue);
  float presence    = mapPresence(presenceSensorValue);

  //DEBUG
  /*Serial.print("Temperature             : ");
  Serial.println(temperature);
  Serial.print("Light                   : ");
  Serial.println(light);
  Serial.print("Presence                : ");
  Serial.println(presence);*/

  //Manage LED's
  manageYellowLED(temperature);
  manageRedLED(presence);
  manageGreenLED(light);

  delay(delayTime);
}
