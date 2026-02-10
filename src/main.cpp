#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QWindow>

#include "app/AppController.h"

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
    engine.loadFromModule(QStringLiteral("Link2BOM"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (auto *window = qobject_cast<QWindow *>(engine.rootObjects().constFirst())) {
        window->setIcon(app.windowIcon());
    }

    return app.exec();
}
