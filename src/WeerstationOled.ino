#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <JsonListener.h>
#include <SH1106.h>
#include <OLEDDisplayUi.h>
#include <Wire.h>

#include "WundergroundClient.h"
#include "TimeClient.h"
#include "DhtClient.h"
#include "ThingSpeakClient.h"

#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

#include "settings.h"

SH1106  display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi   ui( &display );

TimeClient timeClient(UTC_OFFSET);

// Set to false, if you prefere imperial/inches, Fahrenheit
WundergroundClient wunderground(IS_METRIC);

DhtClient dhtClient(DHT_PIN, DHT_TYPE);
ThingSpeakClient thingSpeakClient;

// flag changed in the ticker function
bool readyForRoomUpdate = true;
bool readyForConditionsUpdate = true;
bool readyForForecastUpdate = true;
bool readyForThingSpeak = true;

// tickers
Ticker tickerRoom;
Ticker tickerConditions;
Ticker tickerForecast;
Ticker tickerThingSpeak;

//declaring prototypes
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastToday(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastTomorrow(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDayAfterTomorrow(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y, int dayIndex);
void setReadyForRoomUpdate();
void setReadyForConditionsUpdate();
void setReadyForForecastUpdate();
void setReadyForThingSpeak();


// Add frames
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = { drawDateTime , drawCurrentWeather , drawForecastToday, drawForecastTomorrow };
int numberOfFrames = 4;

OverlayCallback overlays[] = {  };
int numberOfOverlays = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();

    counter++;
  }

  ui.setTargetFPS(30);

  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(RIGHT);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_DOWN);

  ui.setFrames(frames, numberOfFrames);

  ui.setOverlays(overlays, numberOfOverlays);

  // Inital UI takes care of initalising the display too.
  ui.init();

  Serial.println("");

  updateData(&display);

  tickerRoom.attach(UPDATE_INTERVAL_ROOM, setReadyForRoomUpdate);
  tickerConditions.attach(UPDATE_INTERVAL_CONDITIONS, setReadyForConditionsUpdate);
  tickerForecast.attach(UPDATE_INTERVAL_FORECAST, setReadyForForecastUpdate);
  tickerThingSpeak.attach(WRITE_THINGSPEAK_INTERVAL, setReadyForThingSpeak);
}

void loop() {
  if ((readyForRoomUpdate || readyForConditionsUpdate || readyForForecastUpdate)
      && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }

  if (readyForThingSpeak) {
    writeToThingSpeak();
  }
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {
  if (readyForRoomUpdate) {
    dhtClient.updateData();
    readyForRoomUpdate = false;
  }

  if (readyForConditionsUpdate) {
    drawProgress(display, 10, "Updating time...");
    timeClient.updateTime();
    drawProgress(display, 30, "Updating conditions...");
    wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
    readyForConditionsUpdate = false;
  }

  if (readyForForecastUpdate) {
    drawProgress(display, 50, "Updating astronomy...");
    wunderground.updateAstronomy(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
    drawProgress(display, 70, "Updating forecasts...");
    wunderground.updateForecast(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
    readyForForecastUpdate = false;
  }

  if (readyForConditionsUpdate || readyForForecastUpdate) {
    drawProgress(display, 100, "Done...");
    delay(1000);
  }
}



void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  String time = timeClient.getFormattedTime();
  display->drawString(64 + x, y, time);

  display->setFont(ArialMT_Plain_10);
  String sunRise = wunderground.getSunriseTime();
  String sunSet  = wunderground.getSunsetTime();
  String sunRiseSet = sunRise + " - " + sunSet;
  display->drawString(64 + x, 29 + y, sunRiseSet);

  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, 48 + y, dhtClient.getTemperatureAsString());
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(116 + x, 48 + y, dhtClient.getHumidityAsString());
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Meteocons_Plain_42);
  String weatherIcon = wunderground.getTodayIcon();
  display->drawString(x, y, weatherIcon);

  display->setFont(ArialMT_Plain_16);
  String temp = wunderground.getCurrentTemp() + "°C";
  display->drawString(x, 48 + y, temp);


  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(116 + x, y, wunderground.getCity());

  String weather = wunderground.getWeatherText() ;
  display->drawString(116 + x, y + 12, firstWord(weather));
  display->drawString(116 + x, y + 24, secondWord(weather));

  String wind = wunderground.getWindDir() + " " + wunderground.getWindSpeedBft();
  wind.toLowerCase();
  display->drawString(116 + x, 42 + y, wind);

  display->drawString(116 + x, 54 + y, wunderground.getPrecipitationToday());
}

void drawForecastToday(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (timeClient.getHours() < "12") {
    drawForecast( display, state, x, y, 0);
  } else {
    drawForecast( display, state, x, y, 2);
  }
}

void drawForecastTomorrow(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (timeClient.getHours() < "12") {
    drawForecast( display, state, x, y, 2);
  } else {
    drawForecast( display, state, x, y, 4);
  }
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y, int dayIndex) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Meteocons_Plain_42);
  String weatherIcon = wunderground.getForecastIcon(dayIndex);
  display->drawString(x, y, weatherIcon);

  display->setFont(ArialMT_Plain_16);
  display->drawString(x, 48 + y, wunderground.getForecastLowTemp(dayIndex) + "-" + wunderground.getForecastHighTemp(0) + "°C");

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);

  display->drawString(116 + x, y, wunderground.getForecastTitle(dayIndex));

  String weather = wunderground.getForecastConditions(dayIndex);
  display->drawString(116 + x, y + 12, firstWord(weather));
  display->drawString(116 + x, y + 24, secondWord(weather));

  display->drawString(116 + x, y + 42, wunderground.getForecastWindDir(dayIndex) + " " + wunderground.getForecastWindSpeedBft(dayIndex));
  display->drawString(116 + x, y + 54, wunderground.getForecastPop(dayIndex));
}


void writeToThingSpeak() {
  Serial.println("Writing temperature and humidity to ThingSpeak");
  float temperature = dhtClient.getTemperature();
  float humidity = dhtClient.getHumidity();
  thingSpeakClient.writeTempHum(THINGSPEAK_CHANNEL, THINGSPEAK_WRITE_KEY, temperature, humidity) ;

  readyForThingSpeak = false;
}

void setReadyForRoomUpdate() {
  readyForRoomUpdate = true;
}

void setReadyForConditionsUpdate() {
  readyForConditionsUpdate = true;
}

void setReadyForForecastUpdate() {
  readyForForecastUpdate = true;
}

void setReadyForThingSpeak() {
  readyForThingSpeak = true;
}

String firstWord(String input) {
  int index = input.indexOf(' ');
  if (index > 0) {
    return input.substring(0, index);
  } else {
    return input ;
  }
}

String secondWord(String input) {
  int index = input.indexOf(' ');
  if (index > 0) {
    return input.substring(index);
  } else {
    return "" ;
  }
}
