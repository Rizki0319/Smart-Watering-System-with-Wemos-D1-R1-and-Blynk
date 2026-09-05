#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_DEVICE_NAME "YOUR_DEVICE_NAME"
#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial

//#define BLYNK_DEBUG

#define APP_DEBUG

#include <DHT.h>  // DHT library

#define DHTPIN D2      // Declare D2 as the output pin for DHT11
#define DHTTYPE DHT11  // DHT11 sensor type

DHT dht(DHTPIN, DHTTYPE);

#define soilDry 90      // Define the minimum value considered as soil "dry"
#define sensorPin A0    // Declare A0 as the output pin for Soil Moisture Sensor
#define relay D7        // Declare D7 as the output pin for Pump Relay

int manual;

// Uncomment your board, or configure a custom board in Settings.h

//#define USE_SPARKFUN_BLYNK_BOARD
//#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"

BlynkTimer timer;

void sendSensor()
{
    if (manual == 0 || manual == 1) {

        float t = dht.readTemperature();  // Read temperature
        float h = dht.readHumidity();     // Read humidity

        // Display temperature and humidity on the Serial Monitor
        Serial.print("% Humidity: ");
        Serial.print(h);
        Serial.println("% ");

        Serial.print("% Temperature: ");
        Serial.print(t);
        Serial.println("C ");

        // Determine temperature status
        if (t < 28) {

            Serial.println("Status: Temp is too wet");

            Blynk.email(
                "Your_Email_Here",
                "Warning",
                "Status: Room temperature is too low and may affect plant growth!"
            );

            Blynk.logEvent(
                "tempwet_alert",
                "Status: Room temperature is too low and may affect plant growth!"
            );

        } else if (t > 34) {

            Serial.println("Status: Temp is too hot");

            Blynk.email(
                "Your_Email_Here",
                "Warning",
                "Status: Room temperature is too high and may affect plant growth!"
            );

            Blynk.logEvent(
                "tempdry_alert",
                "Status: Room temperature is too high and may affect plant growth!"
            );
        }

        // Determine humidity status
        if (h < 60) {

            Serial.println("Status: Humidity is less than 60%!");

            Blynk.email(
                "Your_Email_Here",
                "Warning",
                "Status: Room humidity is less than 60%!"
            );

            Blynk.logEvent(
                "hum_alert",
                "Status: Room humidity is less than 60%!"
            );
        }

        Blynk.virtualWrite(V0, h);  // Send humidity data to Virtual Pin V0
        Blynk.virtualWrite(V1, t);  // Send temperature data to Virtual Pin V1
    }
}

void soilSensor()
{
    // Get the reading from the function below and print it
    int m = readSensor();

    Serial.print("Analog Output: ");
    Serial.println(m);

    float moisture;

    moisture = ((m / 1024.00) * 100);

    Serial.print("Moisture Percentage = ");
    Serial.print(moisture);
    Serial.println("%");

    // Determine soil moisture status
    if (moisture < soilDry && manual == 0) {

        Serial.println("Status: Soil is too dry - time to water!");

        Blynk.email(
            "Your_Email_Here",
            "Warning",
            "Soil is too dry - time to water!"
        );

        Blynk.logEvent(
            "moisturedry_alert",
            "Status: Soil is too dry - time to water!"
        );

        digitalWrite(relay, HIGH);

    } else if (moisture >= soilDry && manual == 0) {

        Serial.println("Status: Soil moisture is sufficient");

        Blynk.email(
            "Your_Email_Here",
            "Warning",
            "Soil moisture is sufficient - no more watering is needed!"
        );

        Blynk.logEvent(
            "moistureperfect_alert",
            "Status: Soil moisture is sufficient - no more watering is needed!"
        );

        digitalWrite(relay, LOW);
    }

    Blynk.virtualWrite(V2, moisture);  // Send soil moisture data to Virtual Pin V2
}

BLYNK_WRITE(V3)
{
    int pinValue = param.asInt();

    if (pinValue == 1 && manual == 1) {

        digitalWrite(relay, HIGH);

    } else if (pinValue == 0 && manual == 1) {

        digitalWrite(relay, LOW);
    }
}

BLYNK_WRITE(V4)
{
    int pinValue = param.asInt();

    if (pinValue == 1) {

        manual = 1;

    } else {

        manual = 0;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    dht.begin();  // Initialize DHT11

    digitalWrite(relay, LOW);
    pinMode(relay, OUTPUT);

    BlynkEdgent.begin();

    timer.setInterval(1000L, sendSensor);
    timer.setInterval(1000L, soilSensor);
}

void loop()
{
    BlynkEdgent.run();
    timer.run();
}

// This function returns the analog soil moisture measurement
int readSensor()
{
    delay(10);  // Allow power to settle

    int val = analogRead(sensorPin);  // Read the analog value from the sensor

    return val;  // Return the analog soil moisture value
}
