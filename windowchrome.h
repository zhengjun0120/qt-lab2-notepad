#ifndef WINDOWCHROME_H
#define WINDOWCHROME_H

class QWidget;

// 统一窗口标题栏配色，避免跟随系统深色主题和强调色
void applyLightTitleBar(QWidget *widget);

#endif // WINDOWCHROME_H
