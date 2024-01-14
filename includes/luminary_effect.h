#pragma once
#include "led_rect_effect.h"
#include <Ephemeris.h>

using namespace esphome;
using namespace esphome::light;
using namespace esphome::time;

#define NORTH_ANGLE 340
#define UTC_OFFSET 3

class LuminaryEffect {
  private:
    static SolarSystemObject get_object_(ESPTime &time, SolarSystemObjectIndex index) {
      return Ephemeris::solarSystemObjectAtDateAndTime(
        index,
        time.day_of_month,
        time.month,
        time.year,
        time.hour,
        time.minute,
        time.second
      );
    }

    static bool get_is_sunrise_(ESPTime &time, SolarSystemObject sun) {
        int sunrize_hours, sunrize_minutes;
      int sunset_hours, sunset_minutes;
      FLOAT sunrize_seconds;
      FLOAT sunset_seconds;

      FLOAT sunrize_time = Ephemeris::floatingHoursWithUTCOffset(sun.rise, UTC_OFFSET);
      FLOAT sunset_time = Ephemeris::floatingHoursWithUTCOffset(sun.set, UTC_OFFSET);

      Ephemeris::floatingHoursToHoursMinutesSeconds(sunrize_time, &sunrize_hours, &sunrize_minutes, &sunrize_seconds);
      Ephemeris::floatingHoursToHoursMinutesSeconds(sunset_time, &sunset_hours, &sunset_minutes, &sunset_seconds);

      if (!time.is_dst) {
        sunrize_hours -= 1;
        sunset_hours -= 1;
      }

      ESP_LOGD("SUNMOON", "dst: %s", time.is_dst ? "true": "false");
      ESP_LOGD("SUNMOON", "time: %d:%d:%f", time.hour, time.minute, time.second);
      ESP_LOGD("SUNMOON", "sunrize time: %d:%d:%f", sunrize_hours, sunrize_minutes, sunrize_seconds);
      ESP_LOGD("SUNMOON", "sunset time: %d:%d:%f", sunset_hours, sunset_minutes, sunset_seconds);

      return (time.hour >= sunrize_hours && time.hour <= sunset_hours)
        && (time.minute >= sunrize_minutes && time.minute <= sunset_minutes)
        && (time.second >= sunrize_seconds && time.second <= sunset_seconds);
    }
    public:
      static void setup_effect(float latitude, float longitude, float elevation) {
        Ephemeris::setLocationOnEarth(latitude, longitude);
        Ephemeris::flipLongitude(false);
        Ephemeris::setAltitude(elevation);
      }

      static void do_effect(ESPTime &time, light::AddressableLight &light, Color &background_color,
          Color &effect_color) {
        SolarSystemObjectIndex index = true
          ? SolarSystemObjectIndex::Sun
          : SolarSystemObjectIndex::EarthsMoon;

        SolarSystemObject sun = get_object_(time, SolarSystemObjectIndex::Sun);
        bool is_sun_above_horizon = get_is_sunrise_(time, sun);

        FLOAT azimuth = 0;
        if (is_sun_above_horizon) {
          ESP_LOGD("SUNMOON", "The Sun is above the horizon!");
          azimuth = sun.horiCoordinates.azi;
        } else {
          ESP_LOGD("SUNMOON", "The Moon is above the horizon!");
          SolarSystemObject moon = get_object_(time, SolarSystemObjectIndex::EarthsMoon);
          azimuth = moon.horiCoordinates.azi;
        }
        ESP_LOGD("SUNMOON", "%s azimuth is %f", is_sun_above_horizon ? "Sun" : "Moon", azimuth);

        light.all().set(background_color);
        LedRectEffect::set_effect(light, effect_color, NORTH_ANGLE + azimuth);
      }
};