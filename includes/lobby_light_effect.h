#ifndef __LOBBY_LIGHT_EFFECT_H__
#define __LOBBY_LIGHT_EFFECT_H__

#include "esphome.h"

#define DayColor_R 0.0
#define DayColor_G 1.0
#define DayColor_B 1.0

#define NightColor_R 1.0
#define NightColor_G 0.82
#define NightColor_B 0.28

class LobbyLightEffect {

    private:
        light::LightState* _light;
        sun::Sun* _sun;

    public:
        LobbyLightEffect(light::LightState *light, sun::Sun *sun) {
            _light = light;
            _sun = sun;
        }

        void set_effect_color() {
            double elevation = _sun->elevation();
            ESPTime currentTime = _sun->get_time()->now();
            ESPTime sunsetTime = _sun->sunset(elevation).value();
            ESPTime sunriseTime = _sun->sunrise(elevation).value();
            auto lightCall = _light->make_call();
            lightCall.set_transition_length(1000);
            if (currentTime >= sunsetTime && currentTime < sunriseTime) {
                lightCall.set_rgb(NightColor_R, NightColor_G, NightColor_B);
            } else {
                lightCall.set_rgb(DayColor_R, DayColor_G, DayColor_B);
            }
            lightCall.perform();
        }

};

#endif // __LOBBY_LIGHT_EFFECT_H__
