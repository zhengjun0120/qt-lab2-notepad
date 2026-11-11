#include "windowchrome.h"

#include <QWidget>

#ifdef Q_OS_WIN
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include <dwmapi.h>
#endif

void applyLightTitleBar(QWidget *widget)
{
#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    const COLORREF caption = RGB(0xFA, 0xFB, 0xFC);
    const COLORREF text = RGB(0x1F, 0x23, 0x28);
    const COLORREF border = RGB(0xD8, 0xDD, 0xE3);
    DwmSetWindowAttribute(hwnd, 35, &caption, sizeof(caption));
    DwmSetWindowAttribute(hwnd, 36, &text, sizeof(text));
    DwmSetWindowAttribute(hwnd, 34, &border, sizeof(border));
#else
    Q_UNUSED(widget)
#endif
}
