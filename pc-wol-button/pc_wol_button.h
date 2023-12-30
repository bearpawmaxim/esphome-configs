#include "WiFi.h"
#include "WiFiUdp.h"
#include "WakeOnLan.h"


WiFiUDP UDP;
WakeOnLan WOL(UDP);

const char* TAG = "WOL";

void wol_setup() {
  WOL.setRepeat(3, 100);
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
}

void wol_send(const char *mac_addr) {
  bool result = WOL.sendMagicPacket(mac_addr);
  if (result) {
    ESP_LOGI(TAG, "WOL Packet successfully sent.");
  } else {
    ESP_LOGE(TAG, "Error sending WOL Packet!");
  }
}