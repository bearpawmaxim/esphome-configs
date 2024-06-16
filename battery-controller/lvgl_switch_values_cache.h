#include "esphome.h"

class LvglSwitchValuesCache {
  private:
    std::map<lv_obj_t*, bool> _cache {};

  public:
    void cache(lv_obj_t* sw, bool value) {
      auto const result = _cache.insert(std::make_pair(sw, value));
      if (!result.second) {
        result.first->second = value;
      }
    }

    void restore() {
      for (auto it = _cache.begin(); it != _cache.end(); it ++) {
        if (it->second) {
          lv_obj_add_state(it->first, LV_STATE_CHECKED);
        } else {
          lv_obj_clear_state(it->first, LV_STATE_CHECKED);
        }
      }
    }
};

LvglSwitchValuesCache *lvgl_switch_values_cache;