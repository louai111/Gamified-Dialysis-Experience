#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>

// Constants & variables for blood leakage
const int ldrPin = A7;  // Analog pin connected to the voltage divider
int sensorValue = 0;       // Variable to store the raw analog value
float voltage = 0;         // Variable to store the calculated voltage

// Constants & variables for temperature sensing
const int SENSOR_PIN = A13; // Arduino pin connected to Temperature DS18B20 sensor's DQ pin
float tempCelsius;    // temperature in Celsius
float tempFahrenheit; // temperature in Fahrenheit

// Constants & variables for flow senso
int buzzerPin = 53;
int bloodHighFlowLed = 9;
int bloodLowFlowLed = 8;
int leakageLed = 10;
int lowTempLed = 11;
int highTempLed = 12;

double weight;
float flowRate = 100;
float measuredFlowRate = 0;
int blood_pump_pin = 3;       
int dialysate_pump_pin = 2;
int flowPin = 21;       
volatile int count = 0; 
int blood_pump_speed = 128;  
int dialysate_pump_speed = 205; 
unsigned long lastMillis = 0;
char mode;
int leakage_detected;
// int high_temp_detected;
// int low_temp_detected;
// int high_flow_rate_detected;
// int low_flow_rate_detected;

// Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

OneWire oneWire(SENSOR_PIN);         // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library

void Flow() {
  count++;
}

void measureFlowRate() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= 1000) {
    noInterrupts();  
    int pulses = count;
    count = 0; 
    interrupts();   
    
    measuredFlowRate = (pulses * 1000.0) * 1000 / 5880.0 / (currentMillis - lastMillis);

    // Serial.print("Required Flow Rate: ");
    // Serial.print(flowRate);
    // Serial.println(" ml/min");

    // Serial.print("------------ Measured Flow Rate: ");
    // Serial.print(measuredFlowRate);
    // Serial.println(" mL/min");

    lastMillis = currentMillis;
  }
}

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(blood_pump_pin, OUTPUT);
  pinMode(dialysate_pump_pin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  analogWrite(dialysate_pump_pin, dialysate_pump_speed);
  // analogWrite(blood_pump_pin, dialysate_pump_speed);

  Serial.begin(9600);      // Start serial communication
  tempSensor.begin();    // initialize the sensor

  // Serial.println("Pump and Flow Rate Measurement Initialized"); 
  // manualSelection();
  // Serial.println("Invalid mode selected.");

  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), Flow, RISING);
}

void printValues() {
  Serial.print(tempCelsius);
  Serial.print("_");
  Serial.print(measuredFlowRate);
  Serial.print("_");
  Serial.print(leakage_detected);
  Serial.println();
}

void loop() {
  measureFlowRate();
  modifySpeed();
  detectLeakage();
  measureTemperature();
  tempSensor.requestTemperatures();             // send the command to get temperatures
  tempCelsius = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  tempFahrenheit = tempCelsius * 9 / 5 + 32;
  checkAlarms();
  // Optional: Map voltage to light intensity (requires calibration)
  
  // delay(500);              // Wait half a second before next reading

  printValues();
  // delay(500);
}

void detectLeakage() {
  // Read the analog value from photo sensor (0-1023)
  int sensorValue = analogRead(ldrPin);
  
  // Convert to voltage (assuming 5V reference voltage)
  float voltage = sensorValue * (5.0);
  // Serial.print("ldr voltage = ");
  // Serial.println(voltage);

  if (voltage > 600)
    leakage_detected = 1;
  else 
    leakage_detected = 0;

}

void measureTemperature() {
   // convert Celsius to Fahrenheit
}

void checkAlarms() {
  bool buzzerSounding = false;
  if (leakage_detected) {
    buzzerSounding = true;
    digitalWrite(leakageLed, HIGH);
  }
  else {
    digitalWrite(leakageLed, LOW);
  }
  if (tempCelsius > 30) {
    buzzerSounding = true;
    digitalWrite(highTempLed, HIGH);
  }
  else {
    digitalWrite(highTempLed, LOW);
  }
  if (tempCelsius < 25) {
    buzzerSounding = true;
    digitalWrite(lowTempLed, HIGH);
  }
  else {
     digitalWrite(lowTempLed, LOW);
  }
  if (measuredFlowRate > 120) {
    buzzerSounding = true;
    digitalWrite(bloodHighFlowLed, HIGH);
  }
  else {
    digitalWrite(bloodHighFlowLed, LOW);
  }
  if (measuredFlowRate < 30) {
    buzzerSounding = true;
    digitalWrite(bloodLowFlowLed, HIGH);
  }
  else {
    digitalWrite(bloodLowFlowLed, LOW);
  }
  if (buzzerSounding) {
    digitalWrite(buzzerPin, HIGH);
  }
  else {
    digitalWrite(buzzerPin, LOW);
  }
}

void modifySpeed() {
  if (measuredFlowRate < flowRate) blood_pump_speed = min(255, blood_pump_speed + 10);
    else if (measuredFlowRate > flowRate) blood_pump_speed = max(0, blood_pump_speed - 10);

  analogWrite(blood_pump_pin, blood_pump_speed);
}

void manualSelection() {
    // Serial.println("Manual Mode Selected.");
    // Serial.println("Enter flow rate:  mL/min");
    // // Serial.println("Use 'D' to delete, '*' for a decimal point, and '#' to confirm.");
    
    // String input = "";
    // char key;
    // while (true) {
    //     key = keypad.getKey();
    //     if (key) {
    //         if (key >= '0' && key <= '9') {
    //             input += key;
    //             Serial.print("Current Input: ");
    //             Serial.print(input);
    //             Serial.println(" mL/min");
    //         }
    //         } 
    //         else Serial.println("invalid rate!");
    //     }
    }

void reset() {
  wdt_enable(WDTO_15MS);
  while (true); 
}