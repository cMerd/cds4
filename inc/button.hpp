#pragma once

#include <functional>
#include <optional>
#include <raylib.h>

class button {
public:
  button(const Rectangle &rect, const Color &color, const Color &outlineColor,
         const Color &hoverColor, const Color &clickColor,
         const Color &shadowColor, float outlineWidth, float radius,
         const std::function<void()> onClick, const std::optional<int> &key);
  bool isPressed() const;
  bool isDown() const;
  bool isHovered() const;
  void apply() const;
  void draw() const;
  void render() const;
  void renderIdle() const;
  void renderActive() const;
  Vector2 getCenter() const;

private:
  Rectangle btn;
  Rectangle outline;
  float rad;
  Color col;
  Color outlineCol;
  Color hoverCol;
  Color clickCol;
  Color shadowCol;
  std::function<void()> func;
  std::optional<int> key;
};
