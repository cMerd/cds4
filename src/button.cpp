#include <button.hpp>

button::button(const Rectangle &rect, const Color &color,
               const Color &outlineColor, const Color &hoverColor,
               const Color &clickColor, const Color &shadowColor,
               float outlineWidth, float radius,
               const std::function<void()> onClick,
               const std::optional<int> &key)
    : btn(rect),
      outline({rect.x - outlineWidth, rect.y - outlineWidth,
               rect.width + outlineWidth * 2, rect.height + outlineWidth * 2}),
      rad(radius), col(color), outlineCol(outlineColor), hoverCol(hoverColor),
      clickCol(clickColor), shadowCol(shadowColor), func(onClick), key(key) {}

bool button::isHovered() const {
  return CheckCollisionPointRec(GetMousePosition(), btn);
}

bool button::isDown() const {
  return (isHovered() and IsMouseButtonDown(MOUSE_BUTTON_LEFT)) or
         (key ? IsKeyDown(key.value()) : false);
}

bool button::isPressed() const {
  return (isHovered() and IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) or
         (key ? IsKeyPressed(key.value()) : false);
}

Vector2 button::getCenter() const {
  return {btn.x + btn.width / 2, btn.y + btn.height / 2};
}

void button::apply() const { this->func(); }

void button::draw() const {
  render();

  if (isPressed()) {
    apply();
  }
}

void button::render() const {
  DrawRectangleRounded(outline, rad, 1, outlineCol);
  DrawRectangleRounded(btn, rad, 1,
                       (isDown() ? clickCol : (isHovered() ? hoverCol : col)));
  if (!isDown()) {
    DrawRectangleRounded(
        {btn.x, btn.y + btn.height * 0.9f, btn.width, btn.height * 0.1f}, rad,
        1, shadowCol);
    return;
  }

  DrawRectangleRounded(
      {btn.x, btn.y + btn.height * 0.95f, btn.width, btn.height * 0.05f}, rad,
      1, shadowCol);
}

void button::renderIdle() const {

  DrawRectangleRounded(outline, rad, 1, outlineCol);
  DrawRectangleRounded(btn, rad, 1, col);
  DrawRectangleRounded(
      {btn.x, btn.y + btn.height * 0.9f, btn.width, btn.height * 0.1f}, rad, 1,
      shadowCol);
}

void button::renderActive() const {

  DrawRectangleRounded(outline, rad, 1, outlineCol);
  DrawRectangleRounded(btn, rad, 1, clickCol);
  DrawRectangleRounded(
      {btn.x, btn.y + btn.height * 0.95f, btn.width, btn.height * 0.05f}, rad,
      1, shadowCol);
}
