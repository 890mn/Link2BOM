#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QWindow>
#include <QVariant>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

#include "app/AppController.h"

#ifdef Q_OS_WIN
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

static void applyWindowsTitleBarTheme(QWindow *window, bool dark)
{
    if (!window) {
        return;
    }

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return;
    }

    const BOOL immersiveDark = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &immersiveDark, sizeof(immersiveDark));

    const COLORREF captionColor = dark ? RGB(0x14, 0x16, 0x22) : RGB(0xF8, 0xFA, 0xFC);
    const COLORREF textColor = dark ? RGB(0xE6, 0xE1, 0xE8) : RGB(0x0F, 0x17, 0x2A);

    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
}
#endif

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/icon_100.png")));
    QCoreApplication::setOrganizationName(QStringLiteral("Link2BOM"));
    QCoreApplication::setApplicationName(QStringLiteral("Link2BOM"));

    AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.setInitialProperties({{QStringLiteral("appCtx"), QVariant::fromValue(&controller)}});
    engine.loadFromModule(QStringLiteral("Link2BOM"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    auto *window = qobject_cast<QWindow *>(engine.rootObjects().constFirst());
    if (window) {
        window->setIcon(app.windowIcon());
#ifdef Q_OS_WIN
        auto syncTitleBar = [window, &controller]() {
            const bool darkTheme = controller.theme()->currentThemeName() == QStringLiteral("Dark");
            applyWindowsTitleBarTheme(window, darkTheme);
        };
        syncTitleBar();
        QObject::connect(controller.theme(), &ThemeController::currentIndexChanged, window, syncTitleBar);
#endif
    }

    return app.exec();
}



