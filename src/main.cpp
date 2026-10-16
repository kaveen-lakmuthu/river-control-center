#include "settingscontroller.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("river-control-center"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("River Control Center"));
    QCoreApplication::setOrganizationName(QStringLiteral("River Classic"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Settings and theme editor for this River Classic session"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption checkOption(QStringLiteral("check"),
                                         QStringLiteral("Validate the current settings, then exit."));
    const QCommandLineOption regenerateOption(QStringLiteral("regenerate"),
                                              QStringLiteral("Regenerate application theme fragments, then exit."));
    parser.addOption(checkOption);
    parser.addOption(regenerateOption);
    parser.process(app);

    SettingsController settings;
    if (parser.isSet(checkOption)) {
        const QString error = settings.validationError();
        if (!error.isEmpty()) {
            qCritical().noquote() << error;
            return 1;
        }
        qInfo().noquote() << "Configuration is valid:" << settings.settingsPath();
        return 0;
    }
    if (parser.isSet(regenerateOption)) {
        QString error;
        if (!settings.regenerate(&error)) {
            qCritical().noquote() << error;
            return 1;
        }
        return 0;
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), &settings);
    engine.loadFromModule(QStringLiteral("org.kaveen.rivercontrol"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
