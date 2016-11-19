#include <JsonListener.h>
#include <JsonStreamingParser.h>

#define MAX_FORECAST_PERIODS 7

class WundergroundClient: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";
    long localEpoc = 0;
    int gmtOffset = 1;
    long localMillisAtUpdate;
    String date = "-";
    boolean isMetric = true;
    String currentTemp;
	// JJG added ... ////////////////////////////////// define returns /////////////////////////////////
	String moonPctIlum;  // not used
	String moonAge;      // make this a long?
	String moonPhase;
	String sunriseTime;
	String sunsetTime;
	String moonriseTime;
	String moonsetTime;
	String windSpeed;
	String windDir;
	String city;
	// end JJG add ////////////////////////////////////////////////////////////////////////////////////
    String weatherIcon;
    String weatherText;
    String humidity;
    String pressure;
    String precipitationToday;
    void doUpdate(String url);
    String windKphToBft(String stringKph);

    // forecast
    boolean isForecast = false;
    boolean isSimpleForecast = true;
    int currentForecastPeriod;
    String forecastIcon [MAX_FORECAST_PERIODS];
    String forecastTitle [MAX_FORECAST_PERIODS];
    String forecastLowTemp [MAX_FORECAST_PERIODS];
    String forecastHighTemp [MAX_FORECAST_PERIODS];
    String forecastConditions [MAX_FORECAST_PERIODS];
    String forecastWindDir [MAX_FORECAST_PERIODS];
    String forecastWindSpeed [MAX_FORECAST_PERIODS];
    String forecastPop [MAX_FORECAST_PERIODS];


  public:
    WundergroundClient(boolean isMetric);
    void updateConditions(String apiKey, String language, String country, String city);
    void updateForecast(String apiKey, String language, String country, String city);
	void updateAstronomy(String apiKey, String language, String country, String city); // JJG added
    String getHours();
    String getMinutes();
    String getSeconds();
    String getDate();

	// JJG added ... ///////////////////function name to string ////////////////////////////
	String getMoonPctIlum();
	String getMoonAge();
	String getMoonPhase();
	String getSunriseTime();
	String getSunsetTime();
	String getMoonriseTime();
	String getMoonsetTime();
	String getWindSpeed();
	String getWindSpeedBft();
	String getWindDir();
	String getCity();

    long getCurrentEpoch();

    String getCurrentTemp();

    String getTodayIcon();

    String getMeteoconIcon(String iconText);

    String getWeatherText();

    String getHumidity();

    String getPressure();

    String getPrecipitationToday();

    String getForecastIcon(int period);

    String getForecastTitle(int period);

    String getForecastLowTemp(int period);

    String getForecastHighTemp(int period);

    String getForecastConditions(int period);

    String getForecastWindDir(int period);
    String getForecastWindSpeed(int period);
    String getForecastWindSpeedBft(int period);
    String getForecastPop(int period);

    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};
