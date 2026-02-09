#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCoreApplication>

#include "app/AppController.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Link2BOM"));
    QCoreApplication::setApplicationName(QStringLiteral("Link2BOM"));

    AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.loadFromModule(QStringLiteral("Link2BOM"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
