#define BLYNK_TEMPLATE_ID "TMPL64cCnrGn1"
#define BLYNK_TEMPLATE_NAME "Air Pollution Monitoring System G4"
#define BLYNK_FIRMWARE_VERSION "0.0.1"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "pitches.h"
#define BUZZZER_PIN 13
#include <Arduino.h>
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(17, 18);
#include <Adafruit_NeoPixel.h>
#include "MQ135.h"
#define RGBLED 16
#define FAN 12
const int mq135Pin = 36;
int sensorValue = analogRead(mq135Pin);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, RGBLED, NEO_GRB + NEO_KHZ800);
int TOTAL_LEDS = 1;
float SpeedFactor;
float MaximumBrightness = 255;

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

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;

boolean readPMSdata(Stream *s) {
  if (!s->available()) {
    return false;
  }

  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  s->readBytes(buffer, 32);

  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  memcpy((void *)&data, (void *)buffer_u16, 30);

  return true;
}

void goodLed() {
strip.setPixelColor(0, 0, 255, 0);
strip.show();
}

void moderateLed() {

SpeedFactor = 0.008; 

for (int i = 0; i < 1000; i++) {
    float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
    strip.setBrightness(intensity);

    for (int ledNumber=0; ledNumber<TOTAL_LEDS; ledNumber++) {
        strip.setPixelColor(ledNumber, 255, 255, 0);
        }
    strip.show();
    }
}

void badLed() {

SpeedFactor = 0.08; 

for (int i = 0; i < 1000; i++) {
    float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
    strip.setBrightness(intensity);

    for (int ledNumber=0; ledNumber<TOTAL_LEDS; ledNumber++) {
        strip.setPixelColor(ledNumber, 255, 0, 0);
        }
    strip.show();
    }
}

void setAirQuality(int level) {
  airQualityLevel = level;
  switch (level) {
    case GOOD_AIR_QUALITY:
      // Good air quality (PM2.5 < 15)
      goodLed();
      // strip.show();
      analogWrite(FAN, 60);
      break;

    case MODERATE_AIR_QUALITY:
      // Moderate air quality (15 <= PM2.5 <= 35)
      moderateLed();
      // strip.show();
      analogWrite(FAN, 220);
      break;

    case BAD_AIR_QUALITY:
      // Bad air quality (PM2.5 > 35)
      badLed();
      // strip.show();
      for (int thisNote = 0; thisNote < 4; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(BUZZZER_PIN, melody[thisNote], noteDuration);
      }
      analogWrite(FAN, 255);
      break;

    default:
      // Default case for unknown air quality level
      // You can choose to do nothing or define your own behavior.
      break;
  }
}

void sendSensorData() {
  // if (readPMSdata(&pmsSerial)) {
  // int pm25 = data.pm25_standard;

  // if (pm25 < 25) {
  //   setAirQuality(GOOD_AIR_QUALITY);
  // } else if (pm25 >= 25 && pm25 <= 65) {
  //   setAirQuality(MODERATE_AIR_QUALITY);
  // } else {
  //   setAirQuality(BAD_AIR_QUALITY);
  // }

  // Send data to Blynk
  Blynk.virtualWrite(V2, data.pm10_standard);
  Blynk.virtualWrite(V1, data.pm25_standard);
  Blynk.virtualWrite(V3, data.pm100_standard);
  Blynk.virtualWrite(V4, bme.readTemperature());
  Blynk.virtualWrite(V5, bme.readHumidity());
  Blynk.virtualWrite(V0, sensorValue);
  // }
}

void setup() {
  // Initialize the serial communication with baud rate 9600bps.
  Serial.begin(9600);
  // Initialize BME280 sensor and if the connection is not successful,
  // print the failed status to the Serial Monitor.
  if (!bme.begin()) {
    Serial.println("Failed to find Hibiscus Sense BME280 chip");
  }
  pmsSerial.begin(9600);

  strip.begin();
  strip.setBrightness(85);  // Lower brightness to save eyeballs!
  strip.show();             // Initialize all pixels to 'off'
  BlynkEdgent.begin();

  sendSensorData();
  // Call sendSensorData every 5 seconds
  timer.setInterval(5000L, sendSensorData);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
  if (readPMSdata(&pmsSerial)) {
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("PMS5003 Standard Data:");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_standard);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.print("PM 2.5: ");
    Serial.print(data.pm25_standard);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.print("PM 10: ");
    Serial.print(data.pm100_standard);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("PMS5003 Environmental Data:");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_env);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.print("PM 2.5: ");
    Serial.print(data.pm25_env);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.print("PM 10: ");
    Serial.print(data.pm100_env);
    Serial.print(" ug/m3");
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("MQ135 Sensor Data:");
    sensorValue = analogRead(mq135Pin);
    Serial.print("Air Quality: ");
    Serial.print(sensorValue);
    Serial.println(" PPM");
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("BME280 Sensor Data:");
    Serial.print("Temperature: ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    Serial.print("Pressure: ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    int pm25 = data.pm25_standard;

    if (pm25 < 12) {
      setAirQuality(GOOD_AIR_QUALITY);
    } else if (pm25 >= 12 && pm25 <=35) {
      setAirQuality(MODERATE_AIR_QUALITY);
    } else {
      setAirQuality(BAD_AIR_QUALITY);
    }
  }
}
