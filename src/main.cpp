#include <button.hpp>
#include <ds4Controller.hpp>
#include <filesystem>
#include <fstream>
#include <future>
#include <menu.hpp>
#include <pwd.h>
#include <raylib.h>
#include <slider.hpp>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <toggleswitch.hpp>
#include <unistd.h>

static constexpr char gamepadName[] =
    "Sony Interactive Entertainment Wireless Controller";
static constexpr char gamepadName2[] =
    "Sony Computer Entertainment Wireless Controller";

MENU rescan(int &gamepad, ds4Controller &controller, bool changeMenu) {
  while (gamepad >= 0 and gamepad <= 30 and
         strcmp(GetGamepadName(gamepad), gamepadName) != 0 and
         strcmp(GetGamepadName(gamepad), gamepadName2) != 0) {
    gamepad++;
  }
  if ((strcmp(GetGamepadName(gamepad), gamepadName) == 0 or
       strcmp(GetGamepadName(gamepad), gamepadName2) == 0) and
      changeMenu) {
    return MENU::INFO;
    controller = ds4Controller();
    if (!controller) {
      throw std::runtime_error("Failed to find DualShock 4 controller");
    }
  }
  return MENU::SCAN;
}

void RGBtoHSV(Color color, float *hue, float *saturation, float *value) {
  float r = color.r / 255.0f;
  float g = color.g / 255.0f;
  float b = color.b / 255.0f;

  float max = std::max(std::max(r, g), b);
  float min = std::min(std::min(r, g), b);
  float delta = max - min;

  *value = max;

  if (delta == 0.0f) {
    *hue = 0.0f;
    *saturation = 0.0f;
  } else {
    *saturation = delta / max;

    if (max == r) {
      *hue = (g - b) / delta;
    } else if (max == g) {
      *hue = 2.0f + (b - r) / delta;
    } else {
      *hue = 4.0f + (r - g) / delta;
    }

    *hue *= 60.0f;
    if (*hue < 0.0f)
      *hue += 360.0f;
  }
}

void saveAnimation(bool isRGBEnabled) {
  struct passwd *pw = getpwuid(getuid());
  const std::string configPath = std::string(pw->pw_dir) + "/.config/cds4/";

  if (!std::filesystem::exists(configPath)) {
    std::filesystem::create_directories(configPath);
  }

  const std::string configFilePath = configPath + "animation";
  std::ofstream configFile(configFilePath);
  if (!configFile) {
    throw std::runtime_error("Failed to open/create file: " + configFilePath);
  }

  configFile << isRGBEnabled;
  configFile.close();
}

bool loadAnimation() {
  struct passwd *pw = getpwuid(getuid());
  const std::string configPath = std::string(pw->pw_dir) + "/.config/cds4/";

  if (!std::filesystem::exists(configPath)) {
    std::filesystem::create_directories(configPath);
  }

  const std::string configFilePath = configPath + "animation";
  std::ifstream configFile(configFilePath);
  if (!configFile) {
    saveAnimation(false);
    return false;
  }

  bool isRGBEnabled;
  configFile >> isRGBEnabled;
  configFile.close();
  return isRGBEnabled;
}

int main(void) {
  constexpr int screenWidth = 1300;
  constexpr int screenHeight = 800;
  std::string lastDetectedName = "NOT DETECTED";
  std::vector<std::future<void>> futures;

  InitWindow(screenWidth, screenHeight, "CDS4");
  SetTargetFPS(60);

  ds4Controller controller;

  // motors take 6 seconds to vibrate
  // in the beginning of the program we want lights to work, since motor isn't
  // vibrating so we change time to 6 seconds before starting
  std::chrono::time_point motorApplyTime =
      std::chrono::system_clock::now() - std::chrono::seconds(6);
  bool vibrated = false;

  Texture2D texPs4Pad = LoadTexture("resources/ps4.png");
  Texture2D texLeftTrigger = LoadTexture("resources/left-trigger.png");
  Texture2D texRightTrigger = LoadTexture("resources/right-trigger.png");
  int gamepad = 0;
  Font font =
      LoadFontEx(("resources/CodeNewRomanNerdFont-Regular.otf"), 20, NULL, 0);
  Font btnFont =
      LoadFontEx(("resources/CodeNewRomanNerdFont-Regular.otf"), 30, NULL, 0);
  Texture2D scanIcon = LoadTexture("resources/scan.png");
  Texture2D ledIcon = LoadTexture("resources/light.png");
  Texture2D motorIcon = LoadTexture("resources/vibration.png");
  Texture2D infoIcon = LoadTexture("resources/info.png");
  MENU sidebar = MENU::SCAN;
  color col = {0, 0, 0};
  button scanBtn({850, 730, 50, 50}, {40, 40, 40, 255}, BLANK,
                 {55, 55, 55, 255}, {30, 30, 30, 255}, BLANK, 0.0f, 0.3f,
                 [&sidebar]() { sidebar = MENU::SCAN; }, {});
  button ledBtn({950, 730, 50, 50}, {40, 40, 40, 255}, BLANK, {55, 55, 55, 255},
                {30, 30, 30, 255}, BLANK, 0.0f, 0.3f,
                [&sidebar]() { sidebar = MENU::LED; }, {});
  button motorBtn({1050, 730, 50, 50}, {40, 40, 40, 255}, BLANK,
                  {55, 55, 55, 255}, {30, 30, 30, 255}, BLANK, 0.0f, 0.3f,
                  [&sidebar]() { sidebar = MENU::MOTOR; }, {});
  button infoBtn({1150, 730, 50, 50}, {40, 40, 40, 255}, BLANK,
                 {55, 55, 55, 255}, {30, 30, 30, 255}, BLANK, 0.0f, 0.3f,
                 [&sidebar]() { sidebar = MENU::INFO; }, {});
  button rescanBtn({920, 180, 150, 60}, {65, 65, 65, 255}, {15, 15, 15, 255},
                   {90, 90, 90, 255}, {45, 45, 45, 255}, BLANK, 1.0f, 0.3f,
                   [&gamepad, &sidebar, &lastDetectedName, &controller]() {
                     gamepad = 0;
                     sidebar = rescan(gamepad, controller, false);
                     lastDetectedName =
                         (IsGamepadAvailable(gamepad) ? GetGamepadName(gamepad)
                                                      : "NOT DETECTED");
                   },
                   {});

  toggleSwitch rgbSwitch({950, 390, 80, 40}, 0.5f, 0.1f, loadAnimation(), GREEN,
                         GRAY, WHITE, saveAnimation);

  slider leftMotorSlider(
      0, [](int n) {}, Rectangle{800, 250, 60, 400}, 0.3f,
      Color{103, 194, 173, 255}, Color{65, 75, 92, 255}, Color{0, 0, 0, 0},
      Color{26, 26, 26, 255}, Color{45, 52, 64, 255}, Color{94, 108, 133, 255},
      0.2f, 0.2f, 0.0f, DIRECTION::HORIZONTAL);
  slider rightMotorSlider(
      0, [](int n) {}, Rectangle{1190, 250, 60, 400}, 0.3f,
      Color{103, 194, 173, 255}, Color{65, 75, 92, 255}, Color{0, 0, 0, 0},
      Color{26, 26, 26, 255}, Color{45, 52, 64, 255}, Color{94, 108, 133, 255},
      0.2f, 0.2f, 0.0f, DIRECTION::HORIZONTAL);

  button motorApplyBtn(Rectangle{965, 300, 100, 50}, {40, 40, 40, 255}, BLANK,
                       {55, 55, 55, 255}, {30, 30, 30, 255}, BLANK, 0.0f, 0.3f,
                       [&controller, &leftMotorSlider, &rightMotorSlider,
                        &motorApplyTime, &vibrated]() {
                         vibrated = true;
                         motorApplyTime = std::chrono::system_clock::now();
                         controller.vibrate(
                             uint8_t(leftMotorSlider.getValue() * 255 / 100),
                             uint8_t(rightMotorSlider.getValue() * 255 / 100));
                       },
                       {});

  Rectangle hueSlider = {800, 100, 20, 256};
  Rectangle colorPicker = {840, 100, 256, 256};
  Color selectedColor = {0, 0, 0, 255};
  float hue = 0.0f;
  Vector2 sbPosition = {0.0f, 0.0f};
  bool isSearching = false;

  futures.push_back(std::async(
      std::launch::async, [&sidebar, &gamepad, &controller, &isSearching]() {
        isSearching = true;
        sidebar = rescan(gamepad, controller, true);
        isSearching = false;
      }));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(Color{69, 69, 69, 255});

    if ((!IsGamepadAvailable(gamepad) or
         lastDetectedName != GetGamepadName(gamepad)) and
        !isSearching) {
      gamepad = 0;
      futures.push_back(
          std::async(std::launch::async,
                     [&sidebar, &gamepad, &controller, &isSearching]() {
                       isSearching = true;
                       sidebar = rescan(gamepad, controller, true);
                       isSearching = false;
                     }));

      lastDetectedName = (IsGamepadAvailable(gamepad) ? GetGamepadName(gamepad)
                                                      : "NOT DETECTED");
      if (IsGamepadAvailable(gamepad) and
          (strcmp(GetGamepadName(gamepad), gamepadName) == 0 or
           strcmp(GetGamepadName(gamepad), gamepadName2) == 0)) {
        controller.repair();
      } else {
        controller.reset();
      }
    }

    if (IsGamepadAvailable(gamepad) and
        (lastDetectedName == gamepadName or lastDetectedName == gamepadName2)) {

      DrawTexture(texPs4Pad, 0, 150, Color{222, 222, 222, 255});

      // Draw buttons: basic
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT))
        DrawRectangleRounded({309, 280, 15, 28}, 1, 1,
                             Color{103, 194, 173, 255});

      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        DrawRectangleRounded({468, 280, 15, 28}, 1, 1,
                             Color{103, 194, 173, 255});

      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP))
        DrawCircle(557, 294, 13, LIME);
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
        DrawCircle(586, 323, 13, RED);
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        DrawCircle(557, 353, 13, BLUE);
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        DrawCircle(527, 323, 13, PINK);

      // Draw buttons: d-pad
      DrawRectangle(225, 282, 24, 84, BLACK);
      DrawRectangle(195, 311, 84, 25, BLACK);
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP))
        DrawRectangle(225, 282, 24, 29, Color{103, 194, 173, 255});
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
        DrawRectangle(225, 282 + 54, 24, 30, Color{103, 194, 173, 255});
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
        DrawRectangle(195, 311, 30, 25, Color{103, 194, 173, 255});
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
        DrawRectangle(195 + 54, 311, 30, 25, Color{103, 194, 173, 255});

      // Draw buttons: left-right back buttons
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
        DrawTexture(texLeftTrigger, 200, 220, Color{103, 194, 173, 255});
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
        DrawTexture(texRightTrigger, 508, 220, Color{103, 194, 173, 255});

      // Draw axis: left joystick
      Color leftGamepadColor = BLACK;
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_THUMB))
        leftGamepadColor = Color{103, 194, 173, 255};
      DrawCircle(319, 405, 35, leftGamepadColor);
      DrawCircle(319, 405, 31, LIGHTGRAY);
      DrawCircle(
          319 +
              (int)(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) * 20),
          405 +
              (int)(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) * 20),
          25, leftGamepadColor);

      // Draw axis: right joystick
      Color rightGamepadColor = BLACK;
      if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_THUMB))
        rightGamepadColor = Color{103, 194, 173, 255};
      DrawCircle(475, 405, 35, rightGamepadColor);
      DrawCircle(475, 405, 31, LIGHTGRAY);
      DrawCircle(
          475 +
              (int)(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X) * 20),
          405 +
              (int)(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y) * 20),
          25, rightGamepadColor);

      // Draw axis: left-right triggers
      DrawRectangle(169, 198, 15, 70, Color{103, 194, 173, 255});
      DrawRectangle(611, 198, 15, 70, Color{103, 194, 173, 255});
      DrawRectangle(169, 198, 15,
                    (int)(((1 - GetGamepadAxisMovement(
                                    gamepad, GAMEPAD_AXIS_LEFT_TRIGGER)) /
                           2) *
                          70),
                    GRAY);
      DrawRectangle(611, 198, 15,
                    (int)(((1 - GetGamepadAxisMovement(
                                    gamepad, GAMEPAD_AXIS_RIGHT_TRIGGER)) /
                           2) *
                          70),
                    GRAY);

      if (controller) {
        try {
          color ledCol = controller.getColor();
          DrawRectangleRounded({340, 275, 113, 5}, 1.0f, 1,
                               {ledCol.r, ledCol.g, ledCol.b, 255});
        } catch (...) {
        }
      }

    } else if (lastDetectedName != "NOT DETECTED" and !isSearching) {
      gamepad = 0;
      sidebar = rescan(gamepad, controller, true);
      lastDetectedName = (IsGamepadAvailable(gamepad) ? GetGamepadName(gamepad)
                                                      : "NOT DETECTED");
      if (IsGamepadAvailable(gamepad) and
          (strcmp(GetGamepadName(gamepad), gamepadName) == 0 or
           strcmp(GetGamepadName(gamepad), gamepadName2) == 0)) {
        controller.repair();
      } else {
        controller.reset();
      }
    }

    if (vibrated and std::chrono::system_clock::now() - motorApplyTime >=
                         std::chrono::seconds(6)) {
      vibrated = false;
    }

    DrawRectangle(750, 0, 550, 800, Color{50, 50, 50, 255});

    scanBtn.draw();
    DrawTexture(scanIcon, 860, 740, WHITE);

    ledBtn.draw();
    DrawTexture(ledIcon, 960, 740, WHITE);

    motorBtn.draw();
    DrawTexture(motorIcon, 1060, 740, WHITE);

    infoBtn.draw();
    DrawTexture(infoIcon, 1160, 740, WHITE);

    std::string deviceName = GetGamepadName(gamepad);
    if (!IsGamepadAvailable(gamepad) or
        (deviceName != gamepadName and deviceName != gamepadName2)) {
      deviceName = "No DualShock 4 available.";
    }

    bool changed = false;
    while (MeasureTextEx(font, deviceName.c_str(), 20, 1).x > 500) {
      deviceName.pop_back();
      changed = true;
    }
    if (changed) {
      deviceName.pop_back();
      deviceName.pop_back();
      deviceName.pop_back();
      deviceName += "...";
    }
    DrawTextEx(font, deviceName.c_str(), {760, 10}, 20, 1, WHITE);
    switch (sidebar) {
    case MENU::SCAN: {

      DrawTextEx(font, "Try rescanning if: ", {800, 50}, 20, 1, WHITE);
      DrawTextEx(font, "  1. Program doesn't work properly ", {800, 80}, 20, 1,
                 WHITE);
      DrawTextEx(font, "  2. DualShock 4 doesn't work properly ", {800, 110},
                 20, 1, WHITE);
      DrawTextEx(font, "  3. DualShock 4 doesn't get detected ", {800, 140}, 20,
                 1, WHITE);
      rescanBtn.draw();
      DrawTextEx(btnFont, "rescan",
                 {rescanBtn.getCenter().x - 40, rescanBtn.getCenter().y - 15},
                 30, 1, WHITE);
      break;
    }
    case MENU::LED: {
      DrawTextEx(btnFont, "Choose LED color: ", {800, 50}, 30, 1, WHITE);
      DrawTextEx(font, "RGB animation: ", {800, 400}, 20, 1, WHITE);
      Vector2 mousePosition = GetMousePosition();
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, hueSlider)) {
          hue = (mousePosition.y - hueSlider.y) / hueSlider.height;
        }
        if (CheckCollisionPointRec(mousePosition, colorPicker)) {
          sbPosition = (Vector2){mousePosition.x - colorPicker.x,
                                 mousePosition.y - colorPicker.y};
        }
        if (CheckCollisionPointRec(mousePosition, hueSlider) or
            CheckCollisionPointRec(mousePosition, colorPicker)) {

          float saturation = sbPosition.x / colorPicker.width;
          float brightness = 1.0f - sbPosition.y / colorPicker.height;
          selectedColor = ColorFromHSV(hue * 360.0f, saturation, brightness);

          if (!rgbSwitch and !vibrated) {
            try {
              controller.setColor(
                  {selectedColor.r, selectedColor.g, selectedColor.b});

            } catch (...) {
            }
          }
        }
      }
      for (int i = 0; i < hueSlider.height; i++) {
        float hueValue = (float)i / hueSlider.height;
        Color color = ColorFromHSV(hueValue * 360.0f, 1.0f, 1.0f);
        DrawRectangleRec(
            (Rectangle){hueSlider.x, hueSlider.y + i, hueSlider.width, 1},
            color);
      }
      for (int y = 0; y < (int)colorPicker.height; y++) {
        for (int x = 0; x < (int)colorPicker.width; x++) {
          float saturation = (float)x / colorPicker.width;
          float brightness = 1.0f - (float)y / colorPicker.height;
          Color color = ColorFromHSV(hue * 360.0f, saturation, brightness);
          DrawPixel(colorPicker.x + x, colorPicker.y + y, color);
        }
      }
      DrawRectangleRec((Rectangle){colorPicker.x + sbPosition.x - 5,
                                   colorPicker.y + sbPosition.y - 5, 10, 10},
                       BLACK);
      DrawRectangle(1100, 100, 100, 100, selectedColor);
      DrawRectangleLines(1100, 100, 100, 100, BLACK);
      if (rgbSwitch) {
        DrawLineEx({1100, 100}, {1200, 200}, 5.0f, RED);
        DrawLineEx({1200, 100}, {1100, 200}, 5.0f, RED);
      }
      rgbSwitch.draw();
      break;
    }
    case MENU::MOTOR:
      leftMotorSlider.render(leftMotorSlider.getValue());
      rightMotorSlider.render(rightMotorSlider.getValue());
      motorApplyBtn.draw();
      DrawTextEx(
          btnFont, "apply",
          {motorApplyBtn.getCenter().x - 37, motorApplyBtn.getCenter().y - 15},
          30, 1, WHITE);
      DrawTextEx(btnFont, "L", {820, 200}, 30, 1, WHITE);
      DrawTextEx(btnFont, "R", {1210, 200}, 30, 1, WHITE);
      DrawTextEx(btnFont,
                 (std::to_string(leftMotorSlider.getValue()) + "%").c_str(),
                 {810, 680}, 30, 1, WHITE);
      DrawTextEx(btnFont,
                 (std::to_string(rightMotorSlider.getValue()) + "%").c_str(),
                 {1200, 680}, 30, 1, WHITE);
      DrawTextEx(btnFont, "Choose motor strength.", {850, 50}, 30, 1, WHITE);
      break;
    case MENU::INFO: {
      if ((deviceName == gamepadName or deviceName == gamepadName2) and
          IsGamepadAvailable(gamepad)) {
        try {
          DrawTextEx(font,
                     ("Device HID file: " + controller.getHIDFile()).c_str(),
                     {760, 60}, 20, 1, WHITE);
          DrawTextEx(
              font,
              (std::string("Device HID number: ") + std::to_string(gamepad))
                  .c_str(),
              {760, 90}, 20, 1, WHITE);
          DrawTextEx(font, ("LED color: " + controller.getHEXColor()).c_str(),
                     {760, 120}, 20, 1, WHITE);
          color ledCol = controller.getColor();
          DrawRectangle(947, 117, 26, 26, BLACK);
          DrawRectangle(950, 120, 20, 20, {ledCol.r, ledCol.g, ledCol.b, 255});
          color batteryCol = controller.getBatteryColor();
          DrawTextEx(font,
                     ("Battery level: " +
                      std::to_string(controller.getBatteryPercentage()) + "%" +
                      " (" + controller.getBatteryLevel() + ")")
                         .c_str(),
                     {760, 150}, 20, 1,
                     {batteryCol.r, batteryCol.g, batteryCol.b, 255});
        } catch (...) {
        }
      }
      if (controller) {

        float saturation, brightness;

        RGBtoHSV(selectedColor, &hue, &saturation, &brightness);
        hue /= 360.0f; // Normalize hue to [0, 1]
        sbPosition.x = saturation * colorPicker.width;
        sbPosition.y = (1.0f - brightness) * colorPicker.height;

        color ledCol = controller.getColor();
        selectedColor = {ledCol.r, ledCol.g, ledCol.b, 255};
      }
      break;
    }
    default:
      throw std::logic_error("Invalid menu selection\n");
      break;
    }

    if (rgbSwitch and controller and !vibrated) {

      if (col.r < 255 && col.g == 0 && col.b == 0) {
        col.r++;
      } else if (col.r == 255 && col.g < 255 && col.b == 0) {
        col.g++;
      } else if (col.r > 0 && col.g == 255 && col.b == 0) {
        col.r--;
      } else if (col.r == 0 && col.g == 255 && col.b < 255) {
        col.b++;
      } else if (col.r == 0 && col.g > 0 && col.b == 255) {
        col.g--;
      } else if (col.r < 255 && col.g == 0 && col.b == 255) {
        col.r++;
      } else if (col.r == 255 && col.g == 0 && col.b > 0) {
        col.b--;
      }

      try {
        controller.setColor(col);
      } catch (...) {
      }
    }

    EndDrawing();
  }

  UnloadTexture(texPs4Pad);
  CloseWindow();
  return 0;
}
