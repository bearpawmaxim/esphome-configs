#include "WiFi.h"
#include "WiFiUdp.h"
#include "WakeOnLan.h"


WiFiUDP UDP;
WakeOnLan WOL(UDP);

const char* TAG = "WOL";

void wol_setup() {
  WOL.setRepeat(10, 200);
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
}

void wol_send(const char *mac_addr) {
  bool result = WOL.sendMagicPacket(mac_addr, 7);
  if (result) {
    ESP_LOGI(TAG, "WOL Packet successfully sent.");
  } else {
    ESP_LOGE(TAG, "Error sending WOL Packet!");
  }
}

void draw_bezel(Display *it, Color color, int centerX, int centerY, int outerRadius, int innerRadius) {
    // Draw outer circle
    it->circle(centerX, centerY, outerRadius, color);

    // Constant angle increment for 60 tick marks
    const double angleIncrement = 2 * 3.14159265358979323846 / 60;

    // Static array to store tick start and end values
    static uint8_t tickValues[60][4];

    // Fill tickValues array only if it is empty
    if (tickValues[0][0] == 0) {
        for (int i = 0; i < 60; ++i) {
            tickValues[i][0] = static_cast<uint8_t>(centerX + outerRadius * cos(i * angleIncrement));
            tickValues[i][1] = static_cast<uint8_t>(centerY + outerRadius * sin(i * angleIncrement));
            tickValues[i][2] = static_cast<uint8_t>(centerX + innerRadius * cos(i * angleIncrement));
            tickValues[i][3] = static_cast<uint8_t>(centerY + innerRadius * sin(i * angleIncrement));
        }
    }

    // Draw tick marks using precomputed values
    for (int i = 0; i < 60; ++i) {
        it->line(tickValues[i][0], tickValues[i][1], tickValues[i][2], tickValues[i][3], color);
    }
}