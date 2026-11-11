#include "mainwindow.h"
#include "windowchrome.h"

#include <QApplication>
#include <QFont>
#include <QLibraryInfo>
#include <QStyleHints>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // 系统是深色主题，文件对话框统一用 Qt 自绘的浅色版本，和主界面保持一致
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("简单文本编辑器"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    // 本机是深色主题，实验截图统一用浅色，界面和参考图保持一致
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);

    QTranslator translator;
    if (translator.load(QStringLiteral("qt_zh_CN"),
                        QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&translator);
    }

    QFont font = app.font();
    font.setPointSize(10);
    app.setFont(font);

    MainWindow window;
    window.show();
    applyLightTitleBar(&window);

    const QStringList args = QCoreApplication::arguments();
    if (args.size() > 1)
        window.openPath(args.at(1));

    return app.exec();
}
