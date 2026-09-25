#pragma once
#include "led_rect_effect.h"
#include <Ephemeris.h>

using namespace esphome;
using namespace esphome::light;
using namespace esphome::time;

#define NORTH_ANGLE 330

class LuminaryEffect {
  private:
    static SolarSystemObject get_object_(ESPTime &time, SolarSystemObjectIndex index) {
      int utc_hour = time.hour;
      int utc_day = time.day_of_month;
      int utc_month = time.month;
      int utc_year = time.year;

      int offset_hours = time.is_dst ? 3 : 2;
      utc_hour -= offset_hours;

      // Handle day rollover
      while (utc_hour < 0) {
        utc_hour += 24;
        utc_day--;
        if (utc_day == 0) {
          utc_month--;
          if (utc_month == 0) {
            utc_month = 12;
            utc_year--;
          }
          // Approximate days per month (good enough for astronomy)
          static const int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
          utc_day = days_in_month[utc_month - 1];
          // Leap year Feb 29
          if (utc_month == 2 && utc_year % 4 == 0 && (utc_year % 100 != 0 || utc_year % 400 == 0)) {
            utc_day = 29;
          }
        }
      }

      return Ephemeris::solarSystemObjectAtDateAndTime(
        index,
        utc_day,
        utc_month,
        utc_year,
        utc_hour,
        time.minute,
        time.second
      );
    }

    static bool get_is_rise_time_(ESPTime &time, SolarSystemObject luminary) {
      int offset_hours = time.is_dst ? 3 : 2;

      FLOAT rize_time = Ephemeris::floatingHoursWithUTCOffset(luminary.rise, offset_hours);
      FLOAT set_time = Ephemeris::floatingHoursWithUTCOffset(luminary.set, offset_hours);

      int rize_hours, rize_minutes;
      int set_hours, set_minutes;
      FLOAT rize_seconds;
      FLOAT set_seconds;

      Ephemeris::floatingHoursToHoursMinutesSeconds(rize_time, &rize_hours, &rize_minutes, &rize_seconds);
      Ephemeris::floatingHoursToHoursMinutesSeconds(set_time, &set_hours, &set_minutes, &set_seconds);

      ESP_LOGD("SUNMOON", "  rize time (local): %d:%d:%f", rize_hours, rize_minutes, rize_seconds);
      ESP_LOGD("SUNMOON", "  set time (local): %d:%d:%f", set_hours, set_minutes, set_seconds);

      // Convert current time to minutes since midnight for comparison
      int current_minutes = time.hour * 60 + time.minute;
      int rize_minutes_total = rize_hours * 60 + rize_minutes;
      int set_minutes_total = set_hours * 60 + set_minutes;

      bool is_rize;
      if (rize_minutes_total <= set_minutes_total) {
        is_rize = (current_minutes >= rize_minutes_total && current_minutes <= set_minutes_total);
      } else {
        is_rize = (current_minutes >= rize_minutes_total || current_minutes <= set_minutes_total);
      }

      return is_rize;
    }

  public:
    static void setup_effect(float latitude, float longitude, float elevation) {
      Ephemeris::setLocationOnEarth(latitude, longitude);
      Ephemeris::flipLongitude(false);
      Ephemeris::setAltitude(elevation);
    }

    static void do_effect(ESPTime &time, light::AddressableLight &light, Color &background_color,
        Color &effect_color) {
      ESP_LOGD("SUNMOON", "dst: %s", time.is_dst ? "true": "false");
      ESP_LOGD("SUNMOON", "time: %d:%d:%d", time.hour, time.minute, time.second);
      
      ESP_LOGD("SUNMOON", "sun:");
      SolarSystemObject sun = get_object_(time, SolarSystemObjectIndex::Sun);
      bool is_sun_above_horizon = get_is_rise_time_(time, sun);

      ESP_LOGD("SUNMOON", "moon:");
      SolarSystemObject moon = get_object_(time, SolarSystemObjectIndex::EarthsMoon);
      bool is_moon_above_horizon = get_is_rise_time_(time, moon);

      FLOAT azimuth = 0;
      if (is_sun_above_horizon) {
        ESP_LOGD("SUNMOON", "The Sun is above the horizon!");
        azimuth = sun.horiCoordinates.azi;
      } else if (is_moon_above_horizon) {
        ESP_LOGD("SUNMOON", "The Moon is above the horizon!");
        azimuth = moon.horiCoordinates.azi;
      }

      light.all().set(background_color);
      LedRectEffect::set_effect(light, effect_color, NORTH_ANGLE + azimuth);
    }
};