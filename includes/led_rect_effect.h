#pragma once

using namespace esphome;
using namespace esphome::light;
using namespace esphome::sun;

class LedRectEffect {
    private:
        static inline uint16_t compute_len(uint16_t sideALength, uint16_t triangleAngle) ALWAYS_INLINE {
            return (uint16_t)(sideALength * tan(radians(triangleAngle)));
        }

        static uint16_t angle_to_led_number(uint16_t angle) {
            uint16_t sideBLength;
            bool isNegativeLength = false;
            uint16_t maxSideBLength = 45;
            uint16_t lengthOffset = 0;

            if (angle < 0) {
                angle += 360;
            }
            if (angle >= 360) {
                angle -= 360;
            }

            if (angle >= 0 && angle < 90) {
                sideBLength = compute_len(45, angle);
                if (sideBLength >= maxSideBLength) {
                    lengthOffset = 30;
                    maxSideBLength = 45;
                    isNegativeLength = true;
                    sideBLength = compute_len(30, 90 - angle);
                }
            } else if (angle >= 90 && angle < 180) {
                maxSideBLength = 45;
                lengthOffset = 75;
                sideBLength = compute_len(30, angle - 90);
                if (sideBLength >= maxSideBLength) {
                    lengthOffset = 120;
                    maxSideBLength = 30;
                    isNegativeLength = true;
                    sideBLength = compute_len(45, 180 - angle);
                }
            } else if (angle >= 180 && angle < 270) {
                maxSideBLength = 30;
                lengthOffset = 150;
                sideBLength = compute_len(45, angle - 180);
                if (sideBLength >= maxSideBLength) {
                    lengthOffset = 180;
                    maxSideBLength = 45;
                    isNegativeLength = true;
                    sideBLength = compute_len(30, 270 - angle);
                }
            } else {
                maxSideBLength = 45;
                lengthOffset = 225;
                sideBLength = compute_len(30, angle - 270);
                if (sideBLength >= maxSideBLength) {
                    lengthOffset = 270;
                    maxSideBLength = 30;
                    isNegativeLength = true;
                    sideBLength = compute_len(45, 360 - angle);
                }
            }

            return lengthOffset + abs(isNegativeLength ? maxSideBLength - sideBLength : sideBLength);
        }

    public:
        static void set_effect(light::AddressableLight &it, Color &color, uint16_t angle) {
            int16_t cw_angle = 360 - angle;
            int16_t startLedIdx = LedRectEffect::angle_to_led_number(cw_angle - 10);
            int16_t endLedIdx = LedRectEffect::angle_to_led_number(cw_angle + 10);

            uint16_t length = abs(endLedIdx - startLedIdx);
            if (startLedIdx > endLedIdx) {
                length = abs(endLedIdx + (300 - startLedIdx));
            }

            while (length > 0) {
                uint16_t point = startLedIdx + length;
                if (point > 300) {
                    point -= 300;
                }
                it[point] = color;
                length--;
            }
        }
};
