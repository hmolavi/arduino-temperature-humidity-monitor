/*
 * Author: Hossein Molavi
 * Date: Sept-21-2024
 *
 * Description: 
 * This program reads temperature and humidity data from a DHT11 sensor 
 * and displays the results on an I2C LCD screen. It also prints the 
 * temperature in Fahrenheit and humidity percentage to the Serial Monitor. 
 * The readings are averaged over the last five samples for accuracy.
 * 
 */

//--------------------------------//
//            Libraries           //
//--------------------------------//
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "DHT_Async.h"

//--------------------------------//
//             PINOUT             //
//--------------------------------//
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 10;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  


//--------------------------------//
//             Globals            //
//--------------------------------//
const unsigned long LCDUpdateIntervals = 3000ul;
unsigned long lastLCDUpdate = 0;



void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    
    Serial.begin(9600);
}

static void measure_environment(float *temperature, float *humidity) {
    const int sampleIntervals = 100;  // delay in ms between samples
    const int numSamples = 5;         // number of recent samples considered in average

    static unsigned long lastSampleTimestamp = millis();
    static float tempSamples[numSamples] = {0};
    static float humiditySamples[numSamples] = {0};
    static int sampleIndex = 0;
    static int validSamples = 0;

    if (millis() - lastSampleTimestamp > sampleIntervals) {
        lastSampleTimestamp = millis();

        float temp, hum;
        if (dht_sensor.measure(&temp, &hum)) {
            tempSamples[sampleIndex] = temp;
            humiditySamples[sampleIndex] = hum;
            
            // Circular buffer design
            sampleIndex = (sampleIndex + 1) % numSamples; 
            validSamples = min(validSamples + 1, numSamples);
        }

        float tempSum = 0;
        float humiditySum = 0;

        for (int i = 0; i < validSamples; i++) {
            tempSum += tempSamples[i];
            humiditySum += humiditySamples[i];
        }

        *temperature = tempSum / validSamples; 
        *humidity = humiditySum / validSamples;
    }
}


void loop() {
    float temperature;
    float humidity;

    measure_environment(&temperature, &humidity);

    if (millis() - lastLCDUpdate > LCDUpdateIntervals) {
        lastLCDUpdate = millis();

        float adjustedTemperatureC = temperature - 2.38; // Calibration 
        float temperatureF = (adjustedTemperatureC * 9.0 / 5.0) + 32;

        Serial.print("T = ");
        Serial.print(temperatureF, 1);
        Serial.print(" deg. F, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");

        lcd.setCursor(0, 0);
        lcd.print(temperatureF, 1);
        lcd.print(" F - ");
        lcd.print(adjustedTemperatureC, 1);
        lcd.print(" C");
        
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity, 1);
        lcd.print(" %");
    }
}
