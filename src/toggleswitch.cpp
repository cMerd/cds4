#include <algorithm>
#include <toggleswitch.hpp>

toggleSwitch::toggleSwitch() {}

toggleSwitch::toggleSwitch(const Rectangle &rect, float radius,
                           float animationSpeed, bool defaultValue,
                           const Color &activeColor, const Color &inactiveColor,
                           const Color &switchColor,
                           const std::function<void(bool)> &onChange)
    : box(rect), rad(radius), animSpeed(animationSpeed), activeCol(activeColor),
      inactiveCol(inactiveColor), switchCol(switchColor), val(defaultValue),
      changeFunc(onChange) {}

bool toggleSwitch::getVal() const { return val; }

void toggleSwitch::setVal(bool value) { val = value; }

bool toggleSwitch::isClicked() const {
  return CheckCollisionPointRec(GetMousePosition(), box) and
         IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

toggleSwitch::operator bool() const { return val; }

void toggleSwitch::draw() {

  static float xPos = (val ? 0.75f : 0.25f);

  if (isClicked()) {
    val = !val;
    changeFunc(val);
  }

  if (xPos > 0.25f and !val) {
    xPos -= animSpeed;
  } else if (xPos < 0.75f and val) {
    xPos += animSpeed;
  }
  xPos = (val ? std::min(xPos, 0.75f) : std::max(xPos, 0.25f));

  DrawRectangleRounded(box, rad, 1, (val ? activeCol : inactiveCol));

  if (val) {
    DrawCircle(box.x + box.width * xPos, box.y + box.height / 2.0f,
               box.height * 0.4f, switchCol);
    return;
  }

  DrawCircle(box.x + box.width * xPos, box.y + box.height / 2.0f,
             box.height * 0.4f, switchCol);
}
