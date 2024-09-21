#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "DHT_Async.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);  

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 10;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    
    Serial.begin(115200);
    lcd.clear();
}

static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();
    const int numSamples = 7;
    float tempSum = 0;
    float humiditySum = 0;
    int validSamples = 0;

    if (millis() - measurement_timestamp > 4000ul) {
        for (int i = 0; i < numSamples; i++) {
            float temp, hum;
            if (dht_sensor.measure(&temp, &hum)) {
                tempSum += temp;
                humiditySum += hum;
                validSamples++;
            }
            delay(500);
        }

        if (validSamples > 0) {
            *temperature = tempSum / validSamples; 
            *humidity = humiditySum / validSamples;
            measurement_timestamp = millis();
            return true;
        }
    }
    return false;
}


void loop() {
    float temperature;
    float humidity;

    if (measure_environment(&temperature, &humidity)) {

        float adjustedTemperatureC = temperature - 2.38; // Calibrated to match 
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
