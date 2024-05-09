/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "c:/Users/Allie/Dropbox/Watch/Watch/src/Clock.ino"
#include <cstring>
#include <ctime>
#include <stdbool.h>

#include "epd2in13_V2.h"
#include "epdpaint.h"
#include "Particle.h"
#include <google-maps-device-locator.h>

#line 10 "c:/Users/Allie/Dropbox/Watch/Watch/src/Clock.ino"
SYSTEM_MODE(SEMI_AUTOMATIC);

void drawLocation(Paint *paint, const char* text);
void drawBattery(Paint *paint);
void locationCallback(const char *event, const char *data);
void weatherCallback(const char *event, const char *data);
void setup();
void loop();

#define COLORED 0
#define UNCOLORED 1

#define SUN " "
#define MOON "!"
#define CLOUDY "\""
#define CLOUDY_SUN "#"
#define CLOUDY_MOON "$"
#define HAIL "%"
#define RAIN "&"
#define SNOW "'"
#define STORM "("
#define BATTERY_1 ")"
#define BATTERY_2 "*"
#define BATTERY_3 "+"
#define BATTERY_4 ","
#define BATTERY_CHARGING "-"
#define DISPLAY_WIDTH 250
#define DISPLAY_HEIGHT 128

volatile bool updateQueued = true;
volatile bool waitingForUpdate = false;
volatile bool fullRefreshQueued = true;
volatile int previousHour = -1;
const char *weekdays[] = {"Sunday\0", "Monday\0", "Tuesday\0", "Wednesday\0", "Thursday\0", "Friday\0", "Saturday\0"};
GoogleMapsDeviceLocator locator;
char longlat[100];
SystemSleepConfiguration sleepHandler;
// StaticJsonDocument<500> doc;


struct datetime_t {
  int year;
  int month;
  int day;
  int dotw;
  int hour;
  int min;
  int sec;
};

struct weather_t {
  int code;
  int temperature;
  bool day;
};

struct location_t {
  String name;
  String region;
  String country;
};

struct datetime_t currentTime = {.year = -1, .month = -1, .day = -1, .dotw = -1, .hour = -1, .min = -1, .sec = -1};;
struct weather_t currentWeather = {.code = 0, .temperature = 0, .day = 0};
struct location_t currentLocation;


Epd epd;
SerialLogHandler logHandler(LOG_LEVEL_ERROR);
//((128 * 250)/8)
unsigned char framebuffer[4000];
Paint paint(framebuffer, EPD_WIDTH, EPD_HEIGHT);

const char* getWeatherIcon(weather_t *weather) {
  if(weather->code == 1000) {
    if(weather->day) {
      return SUN;
    } else {
      return MOON;
    }
  } else if(weather->code == 1003) {
    if(weather->day) {
      return CLOUDY_SUN;
    } else {
      return CLOUDY_MOON;
    }
  } else if((weather->code >= 1006 && weather->code <= 1030) || (weather->code >= 1135 && weather->code <= 1147)) {
    return CLOUDY;
  } else if(weather->code == 1063 || weather->code == 1069 || weather->code == 1072 || (weather->code >= 1150 && weather->code <= 1207) || weather->code == 1240 || weather->code == 1243 || weather->code == 1249 || weather->code == 1252) {
    return RAIN;
  } else if(weather->code == 1066 || weather->code == 1114 || weather->code == 1117 || (weather->code >= 1210 && weather->code <= 1225) || weather->code == 1255 || weather->code == 1258) {
    return SNOW;
  } else if(weather->code == 1237 || weather->code == 1261 || weather->code == 1264) {
    return HAIL;
  } else if((weather->code >= 1273 && weather->code <= 1282) || weather->code == 1087 || weather->code == 1246) {
    return STORM;
  };
  // Returns sunny if unknown weather code is returned
  return SUN;
}

void drawTime(Paint *paint, datetime_t *currentTime) {
  char temp[20];
  sprintf(temp, "%02d:%02d", currentTime->hour, currentTime->min);
  paint->DrawStringAt((DISPLAY_WIDTH - strlen(temp) * Font36.Width)/2, 45, temp, &Font36, COLORED);
}

void drawDate(Paint *paint, datetime_t *currentTime) {
  const char* weekday = weekdays[currentTime->dotw];
  char temp[30];
  sprintf(temp, "%d-%02d-%02d", currentTime->year, currentTime->month, currentTime->day);
  paint->DrawStringAt((DISPLAY_WIDTH - strlen(weekday) * Font24.Width)/2, 5, weekday, &Font24, COLORED);
  paint->DrawStringAt((DISPLAY_WIDTH - strlen(temp) * Font12.Width)/2, 30, temp, &Font12, COLORED);
  
}

void drawWeather(Paint *paint, weather_t *weather) {
  // If temperature is more than 5 digits, we have other problems
  char temperature[5];

  sprintf(temperature, "%dC", weather->temperature);
  
  paint->DrawStringAt(0, 5, getWeatherIcon(weather), &Icons32, COLORED);
  paint->DrawStringAt(5, 30, temperature, &Font12, COLORED);
}

void drawBattery(Paint *paint) {
  float batteryPercent = System.batteryCharge();
  int batteryState = System.batteryState();
  if(batteryState == BATTERY_STATE_CHARGING) {
    paint->DrawStringAt(215, 5, BATTERY_CHARGING, &Icons32, COLORED);
  } else {
    if(batteryPercent >= .75) {
      paint->DrawStringAt(215, 5, BATTERY_4, &Icons32, COLORED);
    } else if(batteryPercent >= .5) {
      paint->DrawStringAt(215, 5, BATTERY_3, &Icons32, COLORED);
    } else if(batteryPercent >= .25) {
      paint->DrawStringAt(215, 5, BATTERY_2, &Icons32, COLORED);
    } else {
      paint->DrawStringAt(215, 5, BATTERY_1, &Icons32, COLORED);
    }
  }

}

void drawLocation(Paint *paint, location_t *location) {
  char temp[100];

  sprintf(temp, "%s, %s", location->name.c_str(), location->region.c_str());
  paint->DrawStringAt((DISPLAY_WIDTH - strlen(temp) * Font12.Width)/2, 95, temp, &Font12, COLORED);

  sprintf(temp, "%s", location->country.c_str());
  paint->DrawStringAt((DISPLAY_WIDTH - strlen(temp) * Font12.Width)/2, 110, temp, &Font12, COLORED);
}


void locationCallback(const char *event, const char *data) {
  JSONValue locationJSON = JSONValue::parseCopy(data);
  JSONObjectIterator iter(locationJSON);
  char temp[50];
  String longitude = "";
  String lattitude = "";

  while(iter.next()) {
    if(iter.name() == "location") {
      JSONObjectIterator locationIter(iter.value());
      while(locationIter.next()) {
        if(locationIter.name() == "lng") {
          longitude = locationIter.value().toString().data();
        } else if(locationIter.name() == "lat") {
          lattitude = locationIter.value().toString().data();
        }
      }
    }
  }

  sprintf(temp, "%s,%s", lattitude.c_str(), longitude.c_str());
  Particle.publish("weather", temp, PRIVATE);
}

void weatherCallback(const char *event, const char *data) {
  JSONValue weatherJSON = JSONValue::parseCopy(data);  Time.zone(-5);
  JSONObjectIterator iter(weatherJSON);
  while(iter.next()) {
    if(iter.name() == "location") {
      JSONObjectIterator locationIter(iter.value());
      while(locationIter.next()) {
        if(locationIter.name() == "localtime") {

          std::tm timeStruct;
          String localtime = locationIter.value().toString().data();

          strptime(localtime, "%Y-%m-%d %H:%M", &timeStruct);
          // Manually set seconds to 0 because seconds are not included in the weather timestamp
          timeStruct.tm_sec = 0;

          long int localEpoch = mktime(&timeStruct);
          long int utcEpoch = Time.now();

          Time.zone((localEpoch - utcEpoch)/3600);
          previousHour = Time.hour();
        } else if(locationIter.name() == "name") {
          currentLocation.name = locationIter.value().toString().data();
        } else if(locationIter.name() == "region") {
          currentLocation.region = locationIter.value().toString().data();
        } else if(locationIter.name() == "country") {
          currentLocation.country = locationIter.value().toString().data();
        }
      }
    } else if(iter.name() == "current") {
      JSONObjectIterator currentIter(iter.value());
      while(currentIter.next()) {
        if(currentIter.name() == "condition") {
          JSONObjectIterator conditionIter(currentIter.value());
          while(conditionIter.next()) {
            if(conditionIter.name() == "code") {
              currentWeather.code = conditionIter.value().toInt();
            }
          }
        } else if(currentIter.name() == "temp_c"){
          currentWeather.temperature = currentIter.value().toInt();
        } else if(currentIter.name() == "is_day"){
          currentWeather.day = currentIter.value().toInt();
        }
      }
    }
  }
  waitingForUpdate = false;
}


void setup() {

  // Disable status LED
  RGB.control(true); 
  RGB.color(0, 0, 0);

  // Display waiting for network message
  paint.SetRotate(ROTATE_270);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt((DISPLAY_WIDTH - strlen("Waiting for") * Font24.Width)/2, 45, "Waiting for", &Font24, COLORED);
  paint.DrawStringAt((DISPLAY_WIDTH - strlen("network...") * Font24.Width)/2, 74, "network...", &Font24, COLORED);
  epd.Init(FULL);
  epd.DisplayPartBaseImage(framebuffer);
  epd.Sleep();

  Cellular.on();
  Particle.connect();


  waitUntil(Particle.connected);
  Particle.subscribe("hook-response/deviceLocator", locationCallback);
  Particle.subscribe("hook-response/weather", weatherCallback);
  
 

}

void loop() {

  // This can potentially delay a display refresh if the network is slow
  // Time is tracked with RTC, so overall accuracy will not be affected and the next refresh will compensate for the delay
  // I had issues with callbacks not processing correctly when going into ULP mode while waiting, so this is the solution unless I find a better one
  if(updateQueued) {
    if(Cellular.isOff()) {
      Cellular.on();
      Particle.connect();

      // Wait ~20 seconds; if not connected after 20 seconds, skip the network update and continue from previous data
      // Using delay allows the OS to run background tasks
      for(int i = 0; i < 2000; i++) {
        if(Particle.connected()) return;
        delay(10);
      }
      updateQueued = false;
    }
    if(!waitingForUpdate && Particle.connected()) {
      waitingForUpdate = true;
      updateQueued = false;
      locator.publishLocation();
    }
  } else if(!waitingForUpdate) {

    Particle.disconnect();
    Cellular.off();

    currentTime = {.year = Time.year(), .month = Time.month(), .day = Time.day(), .dotw = Time.weekday() - 1, .hour = Time.hour(), .min = Time.minute(), .sec = Time.second()};

    paint.Clear(UNCOLORED);
    drawTime(&paint, &currentTime);
    drawDate(&paint, &currentTime);
    drawWeather(&paint, &currentWeather);
    drawLocation(&paint, &currentLocation);
    drawBattery(&paint);

    if(fullRefreshQueued) {
      fullRefreshQueued = false;
      epd.Init(FULL);
      epd.DisplayPartBaseImage(framebuffer);
    } else {
      epd.Init(PART);
      epd.DisplayPart(framebuffer);
    }

    // Queue full refresh and resync time/location every hour
    if(currentTime.hour != previousHour) {
      fullRefreshQueued = true;
      updateQueued = true;
    }
    previousHour = currentTime.hour;

    // Sleep in ULP mode
    // According to Particle documentation, the device draws ~128 uA in this mode
    sleepHandler.mode(SystemSleepMode::ULTRA_LOW_POWER).duration((60 - Time.second()) * 1000);

    System.sleep(sleepHandler);
  }
    
  }
