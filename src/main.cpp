/*
 *  Powered MySensors-2.x sensor
 */
#include <Arduino.h>

// MySensors configuration
#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_BAUD_RATE 9600
#define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <VoltageReference.h>

#define SKETCH_NAME "Repeater Node"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "7"

// Sensors' Child IDs
#define CHILD_ID_VOLTAGE 0
// #d254efine SEND_PERIOD 200000 // Every 20000 loops will send data
unsigned long UPDATE_PERIOD = 30*1000L; // 10s

// #define FAKE_VCC uncomment to enable VCC "deviations"
#define VCC_CALIBRATION 1128380 // determined by voltage_calibration project
VoltageReference vRef;

// Globals
float oldVoltage = -1;
unsigned long lastTx = 0;
// MySensors messages
MyMessage msgVoltage(CHILD_ID_VOLTAGE, V_VOLTAGE);

/*
 * MySensors 2.x presentation
 */
void presentation() {
#ifdef MY_DEBUG
  Serial.println("presentation");
#endif
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
  present(CHILD_ID_VOLTAGE, S_MULTIMETER, "VCC Voltage");
}

/*
 * Setup
 */
void setup() {
#ifdef MY_DEBUG
  Serial.println("setup");
#endif
  oldVoltage = -1;
#ifdef MY_DEBUG
  Serial.println("Calibrating voltage reference");
#endif
	vRef.begin(VCC_CALIBRATION);
}

/*
 * Send sensor and voltage values
 */
void sendValues() {
#ifdef MY_DEBUG
  Serial.println("sendValues");
#endif
  // Voltage
  float volts = vRef.readVcc() / 1000; // convert millivolts to volts
  Serial.print("VCC (volts) = ");
  Serial.println(volts);
  send(msgVoltage.set(volts, 3));
  oldVoltage = volts;
  // Send other sensor values
  // ...
}

/*
 * Loop
 */
void loop() {
  if (oldVoltage == -1 || millis() - lastTx > UPDATE_PERIOD) { // first start or time to send
    sendValues();
    lastTx = millis();
  }
}
