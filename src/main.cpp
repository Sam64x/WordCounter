#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QIcon>

#include "controllers/controller.h"
#include "models/wordfrequencymodel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/application/AppIcon.ico"));

    QQmlApplicationEngine engine;

    QQuickStyle::setStyle("Fusion");
    QQmlContext* rootContext = engine.rootContext();

    Controller controller;
    WordFrequencyModel model;
    controller.setModel(&model);

    rootContext->setContextProperty("controller", &controller);
    rootContext->setContextProperty("wordModel", &model);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []()
    {
        QCoreApplication::exit(-1);
    },
    Qt::QueuedConnection);
    engine.loadFromModule("WordCounters", "Main");

    return app.exec();
}
