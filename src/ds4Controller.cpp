#include <ds4Controller.hpp>

#include <cmath>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

color ds4Controller::getColor() { return this->ledColor; }

void ds4Controller::saveConfig() {
  struct passwd *pw = getpwuid(getuid());
  const std::string configPath = std::string(pw->pw_dir) + "/.config/cds4/";

  if (!std::filesystem::exists(configPath)) {
    std::filesystem::create_directories(configPath);
  }

  const std::string configFilePath = configPath + "config";
  std::ofstream configFile(configFilePath);
  if (!configFile) {
    throw std::runtime_error("Failed to open/create file: " + configFilePath);
  }

  configFile << (char)ledColor.r << (char)ledColor.g << (char)ledColor.b;
  configFile.close();
}

void ds4Controller::applyColor() {

  std::ofstream file(HIDFile, std::ios::binary | std::ios::out);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + HIDFile);
  }

  uint8_t buf[PACKET_SIZE] = {0};
  buf[0] = REPORT_ID;
  buf[1] = UNKNOWN;
  buf[2] = LED_COMMAND;
  buf[6] = ledColor.r;
  buf[7] = ledColor.g;
  buf[8] = ledColor.b;

  file.write(reinterpret_cast<char *>(buf), sizeof(buf));
  if (!file.good() || file.bad()) {
    throw std::runtime_error("Failed to write to the file: " + HIDFile);
  }

  file.close();
}

bool ds4Controller::checkDeviceID(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
    return 0;
  }

  std::string line;
  bool found = 0;

  while (std::getline(file, line)) {
    if (line.find(VENDOR_ID) != std::string::npos and
        line.find(PRODUCT_ID) != std::string::npos) {
      found = true;
      break;
    }
  }

  file.close();
  return found;
}

std::optional<std::string> ds4Controller::findDevice() {
  dirent *entry;
  DIR *dp = opendir("/sys/class/hidraw");
  if (!dp) {
    throw std::runtime_error("Failed to open directory: /sys/class/hidraw");
    return {};
  }

  static std::string devicePath;
  while ((entry = readdir(dp))) {
    if (entry->d_name[0] == '.') {
      continue;
    }

    std::string path =
        std::string("/sys/class/hidraw/") + entry->d_name + "/device/uevent";

    if (checkDeviceID(path)) {
      devicePath = std::string("/dev/") + entry->d_name;
      closedir(dp);
      return devicePath;
    }
  }

  closedir(dp);
  return {};
}

void ds4Controller::setColor(const color &col) {
  try {
    ledColor = col;
    saveConfig();
    applyColor();
  } catch (const std::runtime_error &e) {
    throw e;
  } catch (const std::logic_error &e) {
    throw e;
  }
}

void ds4Controller::loadConfig() {
  struct passwd *pw = getpwuid(getuid());
  const std::string configFilePath =
      std::string(pw->pw_dir) + "/.config/cds4/config";

  std::ifstream configFile(configFilePath);
  if (!configFile) {
    return;
  }

  char r, g, b; // uint8_t
  configFile >> r >> g >> b;
  ledColor.r = r;
  ledColor.g = g;
  ledColor.b = b;
}

ds4Controller::ds4Controller() {
  std::optional<std::string> DeviceFile = findDevice();
  if (!DeviceFile.has_value()) {
    return;
  }
  isOK = true;
  HIDFile = findDevice().value();
  loadConfig();
  applyColor();
}

ds4Controller::operator bool() { return isOK; }

void ds4Controller::vibrate(uint8_t leftMotor, uint8_t rightMotor) {
  std::ofstream file(HIDFile, std::ios::binary | std::ios::out);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + HIDFile);
  }

  unsigned char buf[PACKET_SIZE] = {0};

  buf[0] = REPORT_ID;
  buf[1] = UNKNOWN;
  buf[2] = 0x00;
  buf[3] = 0x00;
  buf[4] = leftMotor;
  buf[5] = rightMotor;
  buf[6] = ledColor.r;
  buf[7] = ledColor.g;
  buf[8] = ledColor.b;

  file.write(reinterpret_cast<char *>(buf), sizeof(buf));
  if (!file.good() || file.bad()) {
    throw std::runtime_error("Failed to write to the file: " + HIDFile);
  }

  file.close();
}

int ds4Controller::getBattery() {
  unsigned char buf[PACKET_SIZE];
  std::ifstream file(HIDFile, std::ios::binary);
  file.read(reinterpret_cast<char *>(buf), PACKET_SIZE);
  ssize_t len = file.gcount();
  if (len < 0) {
    throw std::runtime_error("Failed to read from file: " + HIDFile);
    return -1;
  }
  if (len < 31) {
    throw std::runtime_error("Unexpected report length: " +
                             std::to_string(len));
    return -1;
  }
  int battery_level =
      buf[30] & 0x0F; // Masking with 0x0F to get the lower nibble

  return battery_level;
}

std::string ds4Controller::getBatteryLevel() {
  int val = getBattery();
  if (val < 0) {
    throw std::runtime_error("Failed to read battery.");
    return "";
  }
  if (val == 0) {
    return "OFF";
  }
  if (val <= 3) {
    return "Low";
  }
  if (val <= 7) {
    return "Medium";
  }
  if (val < 11) {
    return "High";
  }
  if (val == 11) {
    return "Full";
  }
  throw std::logic_error("Battery level is invalid");
}

int ds4Controller::getBatteryPercentage() {
  int val = getBattery();
  static constexpr int maxBattery = 11;
  // maxBattery * x / 100 == val
  // maxBattery * x == val * 100
  // x = val * 100 / maxBattery
  return val * 100 / maxBattery;
}

void ds4Controller::reset() {
  ledColor = {0, 0, 0};
  HIDFile = "";
  isOK = false;
}

void ds4Controller::repair() {
  std::optional<std::string> DeviceFile = findDevice();
  if (!DeviceFile.has_value()) {
    return;
  }
  isOK = true;
  HIDFile = findDevice().value();
  loadConfig();
  applyColor();
}

std::string ds4Controller::getHIDFile() { return HIDFile; }

std::string ds4Controller::getHEXColor() {
  std::stringstream ss;
  ss << "#" << std::hex;
  if ((int)ledColor.r < 16) {
    ss << "0";
  }
  ss << (int)ledColor.r;
  if ((int)ledColor.g < 16) {
    ss << "0";
  }
  ss << (int)ledColor.g;
  if ((int)ledColor.b < 16) {
    ss << "0";
  }
  ss << (int)ledColor.b;
  return ss.str();
}

color ds4Controller::getBatteryColor() {
  int val = getBatteryPercentage();
  return {
      uint8_t(val < 50 ? 255 : std::floor(255 - (val * 2 - 100) * 255 / 100)),
      uint8_t(val > 50 ? 255 : std::floor((val * 2) * 255 / 100)), 0};
}
