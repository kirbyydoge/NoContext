#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>

#include "Engine.h"

#define SCREEN_WIDTH    120
#define SCREEN_HEIGHT   120

namespace kirbyy {
    struct Ellipse {
        float anch_x {};
        float anch_y {};
        float period {};
        float scale {};
        float offset {};
        int rx {};
        int ry {};
    };
};

class CircleArt : public Engine {
public:
    CircleArt(std::wstring name, int width, int height) : Engine(name, width, height) {}
protected:
    using clock = std::chrono::steady_clock;

    virtual bool start() {
        for (int i = 0; i < 7; i++) {
            ellipses.push_back({
                10 + _width * 0.025f * i, _height * 0.5f,  4.0f, 4.5f, 0.06f * i,
                (int)(_width * 0.05f * i), (int)(_height * 0.01f * i)
            });
            ellipses.push_back({
                _width - (10 + _width * 0.025f * i), _height * 0.5f,  4.0f, 4.5f, 0.06f * i + PI * 0.5f * 4.5f,
                (int)(_width * 0.05f * i), (int)(_height * 0.01f * i)
            });
        }
        balls.push_back({
            _width * 0.5f, _height * 0.5f,
            6.0f, 25.0f, 0.06f, 4, 4
        });
        balls.push_back({
            _width * 0.5f, _height * 0.5f,
            6.0f, 25.0f, 0.00f, 2, 2
        });
        color_duration = 0.55;
        color_order = {
            COLOR_YELLOW,  COLOR_DARK_YELLOW,
            COLOR_RED, COLOR_DARK_RED,
            COLOR_DARK_MAGENTA, COLOR_MAGENTA,
            COLOR_DARK_BLUE, COLOR_BLUE,
            COLOR_DARK_CYAN, COLOR_CYAN,
            COLOR_GREEN, COLOR_DARK_GREEN
        };
    }

    virtual bool update(float time_delta) {
        clear_screen();
        // Handle Ellipses
        for (int i = 0; i < ellipses.size(); i++) {
            kirbyy::Ellipse e = ellipses[i];
            int pos_x = e.anch_x + sinf((_time + e.offset) * e.period) * e.scale;
            int pos_y = e.anch_y;
            kirbyy::Ellipse mirror = i % 2 == 0 ? e : ellipses[i - 1];
            float cur_step = (_time + mirror.offset) * mirror.period;
            short color = color_order[(int)(cur_step / color_duration) % color_order.size()];
            draw_ellipse(e.rx, e.ry, pos_y, pos_x, BLOCK_FULL, color);
        }
        // Handle Balls
        for (int i = 0; i < balls.size(); i++) {
            kirbyy::Ellipse b = balls[i];
            float cur_step = (_time + b.offset) * b.period;
            int pos_x = sinf((_time + b.offset) * b.period) * b.scale;
            int pos_y = b.anch_y;
            pos_x += pos_x >= 0 ? ellipses[ellipses.size() - 2].anch_x : ellipses[ellipses.size() - 1].anch_x;
            short color = color_order[(int)(cur_step / color_duration) % color_order.size()];
            draw_ellipse(b.rx, b.ry, pos_y, pos_x, BLOCK_FULL, color);
        }
        render();
        return true;
    }
private:
    std::vector<kirbyy::Ellipse> ellipses;
    std::vector<kirbyy::Ellipse> balls;
    std::vector<short> color_order;
    float color_duration;
};

int main(void) {
    CircleArt game(L"CircleArt", SCREEN_WIDTH, SCREEN_HEIGHT);
    game.run();
    return 0;
}