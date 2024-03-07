#include <Wire.h>
#include <Adafruit_Sensor.h>

#include "pitches.h"
#define BUZZZER_PIN 13

#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(17, 18); // PMS5003 Sensor 

#define FAN 12 // Fan Pin

#include "MQ135.h"
const int mq135Pin = 36;
int sensorValue = analogRead(mq135Pin);

// RGB LED 
#include <Adafruit_NeoPixel.h>
#define RGBLED 16
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, RGBLED, NEO_GRB + NEO_KHZ800);
int TOTAL_LEDS = 1;
float SpeedFactor;
float MaximumBrightness = 255;

// Sensor 
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
// Define air quality levels
#define GOOD_AIR_QUALITY 0
#define MODERATE_AIR_QUALITY 1
#define BAD_AIR_QUALITY 2

int airQualityLevel = GOOD_AIR_QUALITY;

#define USE_ESP32_DEV_MODULE

#include "BlynkEdgent.h"
BlynkTimer timer;

int melody[] = {
  NOTE_FS6, NOTE_C6, NOTE_FS6, NOTE_C6
};

int noteDurations[] = {
  4, 4, 4, 4
};