#pragma once

#include <color.hpp>
#include <optional>
#include <string>

class ds4Controller {
public:
  ds4Controller();
  void setColor(const color &col);
  color getColor();
  operator bool();
  void vibrate(uint8_t leftMotor, uint8_t rightMotor);
  int getBattery();
  int getBatteryPercentage();
  std::string getBatteryLevel();
  std::string getHIDFile();
  std::string getHEXColor();
  color getBatteryColor();
  void reset();
  void repair();

private:
  void applyColor();
  void saveConfig();
  void loadConfig();
  bool checkDeviceID(const std::string &path);
  std::optional<std::string> findDevice();

  color ledColor = {0, 0, 0};
  std::string HIDFile;
  bool isOK = false;

  static constexpr int LED_COMMAND = 0x04;
  static constexpr int UNKNOWN = 0xFF;
  static constexpr int REPORT_ID = 0x05;
  static constexpr int PACKET_SIZE = 32;

  static constexpr char VENDOR_ID[] = "0000054C";
  static constexpr char PRODUCT_ID[] = "000009CC";
};
