#include <Arduino.h>
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <Wire.h>
#include <BH1750.h>

// ----------- Variable declaration -------------

// Shift Register
int pinDato = 14;                   // Pin connected to DS pin 14 of 74HC595
int pinClock = 2;                   // Pin connected to SHCP pin 11 of 74HC595
int pinRegistro = 13;               // Pin connected to STCP pin 12 of 74HC595
int datosALTO = B00000000;          // Combination of digital outputs HIGH
int datosBAJO = B00000000;          // Combination of digital outputs LOW

// Level Sensor
int LevelSensorPin = 33;            // Lilygo TTGO Lora 32 pin 33 is defined for reading the level sensor
int LevelState = 0;                 // Variable that will store the reading of the level sensor pin

// Humidity Sensor
int sensorshumidity01 = 0;          // Lilygo TTGO Lora 32 pin 0 is defined for reading the humidity sensor
int humidity01 = 0;                 // Variable that will store the reading of the humidity sensor pin
int sensorshumidity02 = 4;          // Lilygo TTGO Lora 32 pin 4 is defined for reading the humidity sensor
int humidity02 = 0;                 // Variable that will store the reading of the humidity sensor pin

// Temperature Sensor
OneWire ourWire(15);                // Lilygo TTGO Lora 32 pin 15 is defined for reading the temperature sensor
DallasTemperature sensors(&ourWire);

// Flow Sensor
const int FlowsensorPin = 25;        // Lilygo TTGO Lora 32 pin 25 is defined for reading the flow sensor. (Don't change pin)    
const int measureInterval = 2500;
volatile int pulseConter;
const float factorK = 7.5;
float volume = 0;
long t0 = 0;

// Brightness Sensor
BH1750 lightMeter;                  // Defined for reading the digital ambient light sensor

// ----------- Functions declaration -------------

//Flow sensor functions
void ISRCountPulse(){ 
    pulseConter++;
    }
float GetFrequency(){ 
    pulseConter = 0; 
    interrupts();
    delay(measureInterval);
    noInterrupts();
    return (float)pulseConter * 1000 / measureInterval;
    }
void SumVolume(float dV){
        volume += dV / 60 * (millis() - t0) / 1000.0;
        t0 = millis();
    }

// -------------- Declaration code --------------

void setup() {
    Serial.begin(9600);             // Start the serial port
    // Flow Sensor
    attachInterrupt(digitalPinToInterrupt(FlowsensorPin), ISRCountPulse, RISING); 
    t0 = millis();

    // Level Sensor 
    pinMode(LevelSensorPin, INPUT); // Declare the level sensor pin as input

    // Temperature Sensor
    sensors.begin();                // Temperature sensor is started

    // Ambient light Sensor
    Wire.begin();                   // Initialize the Wire library and connect Arduino to the bus
    lightMeter.begin();

    // Shift Register
    pinMode (pinDato, OUTPUT);
    pinMode (pinClock, OUTPUT);
    pinMode (pinRegistro, OUTPUT);
    }
 
// ------------ Repetition code --------------
void loop() {

    // Shift Register
    shiftOut(pinDato, pinClock, MSBFIRST, datosALTO); // Sends the high part of the byte
    shiftOut(pinDato, pinClock, MSBFIRST, datosBAJO); // Sends the lower part of the byte
    digitalWrite(pinRegistro,HIGH);                   // Pulse for registration HIGH
    digitalWrite(pinRegistro,LOW);                    // Pulse for LOW register

    // Level Sensor
    LevelState = digitalRead(LevelSensorPin);   // Level sensor pin reading

    // Humidity Sensor
    humidity01 = analogRead(sensorshumidity01); // Soil moisture reading via analog pin
    humidity01=map(humidity01,0,4095,100,0);    // Change ranges to express moisture measurement in percent
    humidity02 = analogRead(sensorshumidity02); // Soil moisture reading via analog pin
    humidity02=map(humidity02,0,4095,100,0);    // Change ranges to express moisture measurement in percent

    // Temperature Sensor
    sensors.requestTemperatures();              // The command to read the temperature is sent.
    float temp= sensors.getTempCByIndex(0);     // Temperature in ºC is obtained

    // Flow Sensor
    float frequency = GetFrequency();            // Get frequency in Hz
    float flow_Lmin = frequency / factorK;       // Calculate flow rate L/min
    SumVolume(flow_Lmin);
    float tanque=10;

    // Ambient light Sensor
    float lux = lightMeter.readLightLevel();

    // Serial port printing
    Serial.println("                                                ....                                                       ");
    Serial.println("-----------------------------------------Visualizacion metricas--------------------------------------------");
    Serial.println("                                                ....                                                       ");
    Serial.print("Sensor de Flujo:");
    Serial.print(" Caudal: "); Serial.print(flow_Lmin, 3); Serial.print(" (L/min)    Consumo total:");Serial.print(volume, 1);Serial.print(" (L)"); Serial.print(" (L/min)    En reserva quedan:");Serial.print(tanque-volume, 1);Serial.println(" (L)");
    Serial.print("Humedad 1:"); Serial.print(humidity01); Serial.println("  ");
    Serial.print("Humedad 2:"); Serial.print(humidity02); Serial.println("  ");
    Serial.print("Temperatura: "); Serial.print(temp); Serial.println(" C");
    Serial.print("Luminosidad: ");    Serial.print(lux);    Serial.println(" lx");

    // Conditional flow sensor
    if(LevelState == 0){                        // Alarm activation: If the sensor is in state 0, the LED turns on
        Serial.println("Sensor de Nivel: Activado");
        datosALTO = B00010001;
        datosBAJO = B00010001;
        }
   else{                                       // Otherwise the led will be off
        Serial.println("Sensor de Nivel: Desactivado");
        datosBAJO = B10000000;
        datosALTO = B10000000;
        }
}