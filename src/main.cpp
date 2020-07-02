/*
 *  Battery-powered MySensors-2.x sensor
 */
#include <Arduino.h>

#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_BAUD_RATE 9600

#include <MySensors.h>
#include <VoltageReference.h>

#define SKETCH_NAME "Battery Sensor"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "6"

// Sensors' Child IDs
#define CHILD_ID_BATT 0

#ifdef MY_DEBUG
unsigned long SLEEP_TIME = 5 * 1000L;  // 5s
#else
unsigned long SLEEP_TIME = 10*60*1000; // 10min,  h*min*sec*1000
#endif

#define VCC_CALIBRATION 1128380 // determined by voltage_calibration project
VoltageReference vRef;

// Globals
float oldBatPercentage;
int unusedPins[] = {2, 3, 4, 5, 6, 7, 8};
// MySensors messages
MyMessage msgBatt(CHILD_ID_BATT, V_VOLTAGE);

/*
 * MySensors 2.x presentation
 */
void presentation() {
#ifdef MY_DEBUG
  Serial.println("presentation");
#endif
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  present(CHILD_ID_BATT, S_MULTIMETER, "Battery Voltage");
}

/*
 * Setup
 */
void setup() {
#ifdef MY_DEBUG
  Serial.println("setup");
#endif
  // Reset unused pins
  int count = sizeof(unusedPins)/sizeof(int);
  for (int i = 0; i < count; i++) {
    pinMode(unusedPins[i], INPUT);
    digitalWrite(unusedPins[i], LOW);
  }
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
  Serial.print("Real VCC (volts) = ");
  Serial.println(volts);
  volts = volts + random(0, 5)/100;
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
  // ...
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
