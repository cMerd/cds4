#pragma once

#include <functional>

enum DIRECTION { VERTICAL, HORIZONTAL };

#include <raylib.h>

class slider {
public:
  slider();
  slider(int val);
  slider(int val, const std::function<void(int)> &update_fn);
  slider(int val, const std::function<void(int)> &update_fn,
         const Rectangle &bar, float radius, const Color &color,
         const Color &button_color, const Color &button_line_color,
         const Color &unused_color, const Color &clicked_button_color,
         const Color &hovered_button_color, float anim_speed, float anim_scale,
         float seperator_width, DIRECTION slider_direction);

  void render(int val, const Rectangle &bar, float radius, const Color &color,
              const Color &button_color, const Color &button_line_color,
              const Color &unused_color, const Color &clicked_button_color,
              const Color &hovered_button_color, float anim_speed,
              float anim_scale, float seperator_width,
              DIRECTION slider_direction);

  void render(int val);

  int getValue() const;
  void setValue(int);

private:
  void updateValue(const Rectangle &bar, DIRECTION slider_direction);
  void draw(const Rectangle &bar, float radius, const Color &color,
            const Color &button_color, const Color &button_line_color,
            const Color &unused_color, const Color &clicked_button_color,
            const Color &hovered_button_color, float anim_speed,
            float anim_scale, float seperator_width,
            DIRECTION slider_direction) const;

  void playCursorAnimation(Rectangle &cursor, float val) const;

  static bool isHovered(const Rectangle &cursor);
  static bool isClicked(const Rectangle &cursor);

  std::function<void(int)> update_func;
  Rectangle slider_bar;
  Rectangle cursor;
  Color bg;
  Color bg_enabled;
  Color btn_bg;
  Color btn_bg_hovered;
  Color btn_bg_clicked;
  Color btn_seperator_bg;
  DIRECTION slider_direction;
  float anim_speed;
  float anim_scale;
  float seperator_width;
  float slider_radius;
  int var;
  float animation_value = 0.0f;
};
