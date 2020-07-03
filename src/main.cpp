/*
 *  Battery-powered MySensors-2.x sensor
 */
#include <Arduino.h>

// MySensors configuration
#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_BAUD_RATE 9600

#include <MySensors.h>
#include <VoltageReference.h>
#include <HCSR04.h>
#include <SI7021.h>

#define SKETCH_NAME "Distance Sensor"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "7"

// Sensors' Child IDs
#define CHILD_ID_BATT 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_HUM  2
#define CHILD_ID_DIST 3

#ifdef MY_DEBUG
unsigned long SLEEP_TIME = 10 * 1000L;  // 10s
#else
unsigned long SLEEP_TIME = 10*60*1000; // 10min,  h*min*sec*1000
#endif

// #define FAKE_VCC uncomment to enable VCC "deviations"
#define VCC_CALIBRATION 1097820 // determined by voltage_calibration project
VoltageReference vRef;

#define TRIG_PIN 7
#define ECHO_PIN 6
UltraSonicDistanceSensor distanceSensor(TRIG_PIN, ECHO_PIN);

// Globals
float oldBatPercentage;
int unusedPins[] = {3, 4, 5};

// MySensors messages
MyMessage msgBatt(CHILD_ID_BATT, V_VOLTAGE);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgDist(CHILD_ID_DIST, V_DISTANCE);

#define ATSHA204_PIN   17 // A3
SI7021 siSensor;

/*
 * MySensors 2.x presentation
 */
void presentation() {
#ifdef MY_DEBUG
  Serial.println("presentation");
#endif
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  present(CHILD_ID_BATT, S_MULTIMETER, "Battery Voltage");
  present(CHILD_ID_TEMP, S_TEMP, "Temperature");
  present(CHILD_ID_HUM, S_HUM, "Humidity");
  present(CHILD_ID_DIST, S_DISTANCE, "Water Level");
}

/*
 * Setup
 */
void setup() {
#ifdef MY_DEBUG
  Serial.println("setup");
#endif
  // Make sure that ATSHA204 is not floating
  pinMode(ATSHA204_PIN, INPUT);
  digitalWrite(ATSHA204_PIN, HIGH);
  // Reset unused pins
  int count = sizeof(unusedPins)/sizeof(int);
  for (int i = 0; i < count; i++) {
    pinMode(unusedPins[i], INPUT);
    digitalWrite(unusedPins[i], LOW);
  }
#ifdef FAKE_VCC
  randomSeed(analogRead(0));
#endif
  oldBatPercentage = -1;
#ifdef MY_DEBUG
  Serial.println("Calibrating voltage reference");
#endif
	vRef.begin(VCC_CALIBRATION);
}

/*
 * Send sensor and battery values
 */
void sendValues() {
#ifdef MY_DEBUG
  Serial.println("sendValues");
#endif
  // Battery voltage
  float volts = vRef.readVcc() / 1000; // convert millivolts to volts
#ifdef MY_DEBUG
  Serial.print("VCC (volts) = ");
  Serial.println(volts);
#endif
#ifdef FAKE_VCC
  volts += random(-5, 5)/100.0;
  Serial.print("Random shifted VCC (volts) = ");
  Serial.println(volts);
#endif
  send(msgBatt.set(volts, 3));
  // Battery percentage
  float perc = 100.0 * (volts-VCC_MIN) / (VCC_MAX-VCC_MIN);
  perc = constrain(perc, 0.0, 100.0);
#ifdef MY_DEBUG
  Serial.print("VCC (percentage) = ");
  Serial.println(perc);
#endif
  if (perc != oldBatPercentage) {
    sendBatteryLevel(perc);
    oldBatPercentage = perc;
  }
  // Send other sensor values
  // Temperature and humidity
  si7021_thc data = siSensor.getTempAndRH();
  float temperature = data.celsiusHundredths / 100.0;
  int humidity = data.humidityPercent;
#ifdef MY_DEBUG
  Serial.print("T: ");
  Serial.println(temperature);
  Serial.print("H: ");
  Serial.println(humidity);
#endif
  send(msgTemp.set(temperature, 1));
  send(msgHum.set(humidity));
  // Distance
  double dist = distanceSensor.measureDistanceCm();
  send(msgDist.set(dist, 3));
}

/*
 * Loop
 */
void loop() {
#ifdef DEBUG
  Serial.println("loop");
#endif
  if (oldBatPercentage == -1) { // first start
    // Send the values before sleeping
    sendValues();
  }
  sleep(SLEEP_TIME);
  // Read sensors and send on wakeup
  sendValues();
}
