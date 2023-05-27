#include <chrono>
#include <iostream>
#include <cstring>
#include <cwchar>

#include <windows.h>

// Hacky MINGW Header Fix 
typedef struct _CONSOLE_FONT_INFOEX
{
    ULONG cbSize;
    DWORD nFont;
    COORD dwFontSize;
    UINT  FontFamily;
    UINT  FontWeight;
    WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI SetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX
lpConsoleCurrentFontEx);
#ifdef __cplusplus
}
#endif

#define PI                 3.1415926535897932384f

#define BLOCK_EMPTY L' '
#define BLOCK_FULL L'█'

#define COLOR_BLACK	       0x0000
#define COLOR_DARK_BLUE    0x0001
#define COLOR_DARK_GREEN   0x0002
#define COLOR_DARK_CYAN    0x0003
#define COLOR_DARK_RED     0x0004
#define COLOR_DARK_MAGENTA 0x0005
#define COLOR_DARK_YELLOW  0x0006
#define COLOR_GREY		   0x0007
#define COLOR_DARK_GREY    0x0008
#define COLOR_BLUE		   0x0009
#define COLOR_GREEN	       0x000A
#define COLOR_CYAN		   0x000B
#define COLOR_RED		   0x000C
#define COLOR_MAGENTA	   0x000D
#define COLOR_YELLOW	   0x000E
#define COLOR_WHITE	       0x000F

class Engine {
public:
    Engine(std::wstring name, int width, int height) : _name(name), _width(width), _height(height),
            _screen(nullptr), _console(nullptr), _window(nullptr), _bytes(0), _time(0.0f) {
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 8;
        cfi.dwFontSize.X = 8;
        cfi.dwFontSize.Y = 8;
        cfi.FontFamily = 8;
        cfi.FontWeight = 8;
        wcscpy(cfi.FaceName, L"Terminal");
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
        _screen = new CHAR_INFO[_width * _height + 1];
        _window = GetConsoleWindow();
        _console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        _window_rect = {0, 0, (short)(_width - 1), (short)(_height - 1)};
        COORD buf_size = {(short)_width, (short)_height};
        SetConsoleScreenBufferSize(_console, buf_size);
        SetConsoleActiveScreenBuffer(_console);
        SetConsoleWindowInfo(_console, TRUE, &_window_rect);
        SetWindowLong(_window, GWL_STYLE, GetWindowLong(_window, GWL_STYLE) & ~(WS_HSCROLL | WS_VSCROLL));
        SetWindowPos(_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    ~Engine() {
        delete _screen;
    }

    void run() {
        auto time0 = std::chrono::steady_clock::now();
        auto time1 = std::chrono::steady_clock::now();
        bool is_running = true;
        float delta_time = 0;
        wchar_t wbuf[128];
        swprintf(wbuf, L"%s", _name.c_str());
        start();
        while(update(delta_time)) {
            time1 = std::chrono::steady_clock::now();
            delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(time1 - time0).count();
            _time += delta_time;
            time0 = time1;
            swprintf(wbuf, L"%s FPS: %3.2f", _name.c_str(), 1.0f / delta_time);
            SetConsoleTitleW(wbuf);
        }
    }

protected:
    std::wstring _name;
    CHAR_INFO* _screen;
    int _width;
    int _height;
    HWND _window;
    SMALL_RECT _window_rect;
    HANDLE _console;
    DWORD _bytes;
    float _time;

    virtual bool start() = 0;
    virtual bool update(float time_delta) = 0;

    inline void clear_screen(short val = BLOCK_EMPTY, short color = COLOR_WHITE) {
        for (int i = 0; i < _width * _height; i++) {
            _screen[i].Char.UnicodeChar = val;
            _screen[i].Attributes = color;
        }
    }

    inline void set_pixel(int row, int col, short val = BLOCK_FULL, short color = COLOR_WHITE) {
        _screen[row * _width + col].Char.UnicodeChar = val;
        _screen[row * _width + col].Attributes = color;
    }

    inline void set_pixel_s(int row, int col, short val = BLOCK_FULL, short color = COLOR_WHITE) {
        if (row >= 0 && row < _height && col >= 0 && col < _width) {
            _screen[row * _width + col].Char.UnicodeChar = val;
            _screen[row * _width + col].Attributes = color;
        }
    }

    inline void render() {
		WriteConsoleOutputW(_console, _screen, {(short)_width, (short)_height}, {0, 0}, &_window_rect);
    }

    inline void draw_ellipse(int rx, int ry, int xc, int yc, short block = BLOCK_FULL, short color = COLOR_WHITE) {
        float dx, dy, d1, d2, x, y;
        x = 0;
        y = ry;
        d1 = (ry * ry)
            - (rx * rx * ry)
            + (0.25 * rx * rx);
        dx = 2 * ry * ry * x;
        dy = 2 * rx * rx * y;
        while (dx < dy) {
            set_pixel_s(x + xc, y + yc, block, color);
            set_pixel_s(-x + xc, y + yc, block, color);
            set_pixel_s(x + xc, -y + yc, block, color);
            set_pixel_s(-x + xc, -y + yc, block, color);
            if (d1 < 0) {
                x++;
                dx = dx + (2 * ry * ry);
                d1 = d1 + dx + (ry * ry);
            }
            else {
                x++;
                y--;
                dx = dx + (2 * ry * ry);
                dy = dy - (2 * rx * rx);
                d1 = d1 + dx - dy + (ry * ry);
            }
        }
        d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5)))
            + ((rx * rx) * ((y - 1) * (y - 1)))
            - (rx * rx * ry * ry);
        while (y >= 0) {
            set_pixel_s(x + xc, y + yc, block, color);
            set_pixel_s(-x + xc, y + yc, block, color);
            set_pixel_s(x + xc, -y + yc, block, color);
            set_pixel_s(-x + xc, -y + yc, block, color);
            if (d2 > 0) {
                y--;
                dy = dy - (2 * rx * rx);
                d2 = d2 + (rx * rx) - dy;
            }
            else {
                y--;
                x++;
                dx = dx + (2 * ry * ry);
                dy = dy - (2 * rx * rx);
                d2 = d2 + dx - dy + (rx * rx);
            }
        }
    }

    inline WORD RGBToCharInfoColor(int r, int g, int b) {
        WORD color = 0;
        color |= (r & 0xF) << 8;
        color |= (g & 0xF) << 4;
        color |= (b & 0xF);
        return color;
    }
};
