#include <Arduino.h>
#include <DHT.h>

// WIFI
const char* WIFI_SSID = "Ziggo68968";
const char* WIFI_PWD = "duivendak";

// ThingSpeak
const unsigned long THINGSPEAK_CHANNEL = 173961;
const String THINGSPEAK_WRITE_KEY = "VGUGDB77WB036WIC";

// Wunderground
const boolean IS_METRIC = true;
const String WUNDERGRROUND_API_KEY = "116b5c9cc26deeec";
const String WUNDERGRROUND_LANGUAGE = "NL";
const String WUNDERGROUND_COUNTRY = "NL";
const String WUNDERGROUND_CITY = "Soest";

// Timers
const int UPDATE_INTERVAL_ROOM = 1 * 60;        // Update room every minute
const int UPDATE_INTERVAL_CONDITIONS = 10 * 60; // Update conditions every 10 minutes
const int UPDATE_INTERVAL_FORECAST = 60 * 60;   // Update every forecast every 60 minutes
const int WRITE_THINGSPEAK_INTERVAL = 10 * 60;  // Write temp and humidity to thingspeak every 10 minutes

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D2;
const int SDC_PIN = D1;

// DHT sensor
const int DHT_PIN = D4;
const int DHT_TYPE = DHT22;

// TimeClient settings
const float UTC_OFFSET = 0;
