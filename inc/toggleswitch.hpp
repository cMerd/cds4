#pragma once

#include <functional>
#include <raylib.h>

class toggleSwitch {
public:
  toggleSwitch();
  toggleSwitch(const Rectangle &rect, float radius, float animationSpeed,
               bool defaultValue, const Color &activeColor,
               const Color &inactiveColor, const Color &switchColor,
               const std::function<void(bool)> &onChange);

  bool getVal() const;
  void setVal(bool value);
  operator bool() const;

  void draw();
  bool isClicked() const;

private:
  Rectangle box;
  float rad;
  float animSpeed;
  Color activeCol;
  Color inactiveCol;
  Color switchCol;
  bool val;
  std::function<void(bool)> changeFunc;
};
