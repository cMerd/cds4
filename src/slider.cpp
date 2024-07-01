#include <raylib.h>
#include <slider.hpp>
#include <stdexcept>

slider::slider() {}

slider::slider(int val) : var(val) {}

slider::slider(int val, const std::function<void(int)> &update_fn)
    : update_func(update_fn), var(val) {}

slider::slider(int val, const std::function<void(int)> &update_fn,
               const Rectangle &bar, float radius, const Color &color,
               const Color &button_color, const Color &button_line_color,
               const Color &unused_color, const Color &clicked_button_color,
               const Color &hovered_button_color, float anim_speed,
               float anim_scale, float seperator_width,
               DIRECTION slider_direction) {
  this->var = val;
  this->update_func = update_fn;
  this->slider_bar = bar;
  this->bg = unused_color;
  this->bg_enabled = color;
  this->slider_radius = radius;
  this->btn_bg = button_color;
  this->btn_bg_hovered = hovered_button_color;
  this->btn_bg_clicked = clicked_button_color;
  this->anim_speed = anim_speed;
  this->anim_scale = anim_scale;
  this->slider_direction = slider_direction;
  this->seperator_width = seperator_width;
  this->btn_seperator_bg = button_line_color;
}

void slider::render(int val) {

  this->var = val;

  switch (slider_direction) {
  case VERTICAL:
    cursor.width = this->slider_bar.width / 10.0f;
    cursor.height = this->slider_bar.height + 5.0f;
    cursor.x = (this->slider_bar.x + (this->slider_bar.width * var / 100.0f)) -
               cursor.width / 2.0f;
    cursor.y = this->slider_bar.y - 2.0f;
    break;
  case HORIZONTAL:
    cursor.width = this->slider_bar.width + 5.0f;
    cursor.height = this->slider_bar.height / 10.0f;
    cursor.x = this->slider_bar.x - 2.0f;
    cursor.y =
        (this->slider_bar.y + this->slider_bar.height -
         (this->slider_bar.height * var / 100.0f) - cursor.height / 2.0f);
    break;
  default:
    throw std::logic_error("Invalid direction.");
    break;
  }
  float original_height = cursor.height;
  float animation_target = 0.0f;

  // unused (background) bar
  DrawRectangleRounded(this->slider_bar, this->slider_radius, 1, this->bg);

  // used area
  switch (slider_direction) {
  case VERTICAL:
    DrawRectangleRounded({this->slider_bar.x, this->slider_bar.y,
                          this->cursor.x - this->slider_bar.x + cursor.width,
                          this->slider_bar.height},
                         this->slider_radius, 1, this->bg_enabled);
    break;
  case HORIZONTAL:
    DrawRectangleRounded(
        {this->slider_bar.x, this->cursor.y, this->slider_bar.width,
         (this->slider_bar.y + this->slider_bar.height) - this->cursor.y + 1},
        this->slider_radius, 1, this->bg_enabled);
    break;
  default:
    throw std::logic_error("Invalid direction.");
    break;
  }

  Color cursor_color;
  if (this->isClicked(cursor)) {
    cursor_color = this->btn_bg_clicked;
    animation_target = anim_scale;
  } else if (this->isHovered(cursor)) {
    cursor_color = this->btn_bg_hovered;
    animation_target = anim_scale;
  } else {
    cursor_color = this->btn_bg;
    animation_target = 0.0f;
  }

  // Smooth the animation by interpolating the animation_value
  // Smoothing hover animation
  // this doesn't get out of scope, so it
  // gets larger every animation frame
  animation_value += (animation_target - animation_value) * this->anim_speed;

  this->playCursorAnimation(cursor, animation_value);

  DrawRectangleRounded(cursor, this->slider_radius, 1, cursor_color);

  // Calculate the number of separators based on the original cursor height
  float separator_start = cursor.y + (20.0f);
  float separator_end = cursor.y + original_height - 10.0f;

  for (float i = separator_start; i <= separator_end; i += 10.0f) {
    DrawLine(cursor.x + ((cursor.width - seperator_width) / 2), i,
             cursor.x + cursor.width - ((cursor.width - seperator_width) / 2),
             i, this->btn_seperator_bg);
  }

  this->updateValue(this->slider_bar, slider_direction);
}

void slider::playCursorAnimation(Rectangle &cursor, float val) const {
  float deltaWidth = cursor.width * val;
  float deltaHeight = cursor.height * val;
  cursor.width *= 1.0f + val;
  cursor.height *= 1.0f + val;
  cursor.x -= deltaWidth / 2.0f;
  cursor.y -= deltaHeight / 2.0f;
}

void slider::updateValue(const Rectangle &bar, DIRECTION slider_direction) {

  if (!this->isClicked(bar)) {
    return;
  }

  // bar.x + (bar.width * var / 100) = GetMouseX();
  // bar.width * var / 100 = GetMouseX() - bar.x;
  // bar.width * var = (GetMouseX() - bar.x) * 100;
  // var = (GetMouseX() - bar.x) * 100 / bar.width;
  switch (slider_direction) {
  case VERTICAL: {
    int mouseX = GetMouseX();
    var = (mouseX - bar.x) * 100 / bar.width;
    break;
  }
  case HORIZONTAL: {
    int mouseY = GetMouseY();
    var = (bar.y + bar.height - mouseY) * 100 / bar.height;
    break;
  }
  default:
    throw std::logic_error("Invalid direction");
    break;
  }

  if (var <= 2) {
    var = 0;
  }
  if (var >= 98) {
    var = 100;
  }

  this->update_func(var);
}

int slider::getValue() const { return this->var; }

bool slider::isHovered(const Rectangle &cursor) {
  return CheckCollisionPointRec(GetMousePosition(), cursor);
}

bool slider::isClicked(const Rectangle &cursor) {
  return (CheckCollisionPointRec(GetMousePosition(), cursor) and
          IsMouseButtonDown(MOUSE_BUTTON_LEFT));
}

void slider::setValue(int val) { var = val; }
