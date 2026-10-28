#include "settingscontroller.h"
#include "keybindingscontroller.h"

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
    const QCommandLineOption listWindowsOption(QStringLiteral("list-windows"),
                                               QStringLiteral("List the current Wayland windows and their application IDs."));
    const QCommandLineOption exportBindingsOption(QStringLiteral("export-keybindings"),
                                                  QStringLiteral("Export a printable keybinding map to Documents."));
    parser.addOption(checkOption);
    parser.addOption(regenerateOption);
    parser.addOption(listWindowsOption);
    parser.addOption(exportBindingsOption);
    parser.process(app);

    SettingsController settings;
    KeybindingsController keybindings;
    if (parser.isSet(checkOption)) {
        const QString error = settings.validationError();
        if (!error.isEmpty()) {
            qCritical().noquote() << error;
            return 1;
        }
        const QString bindingError = keybindings.validationError();
        if (!bindingError.isEmpty()) {
            qCritical().noquote() << bindingError;
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
        if (!keybindings.regenerate(&error)) {
            qCritical().noquote() << error;
            return 1;
        }
        return 0;
    }
    if (parser.isSet(listWindowsOption)) {
        settings.refreshOpenWindows();
        for (const QVariant &entry : settings.openWindows()) {
            const QVariantMap window = entry.toMap();
            qInfo().noquote() << window.value(QStringLiteral("appId")).toString()
                              << "\t" << window.value(QStringLiteral("title")).toString();
        }
        if (!settings.ruleTestResult().isEmpty()) {
            qCritical().noquote() << settings.ruleTestResult();
            return 1;
        }
        return 0;
    }
    if (parser.isSet(exportBindingsOption))
        return keybindings.exportPrintableMap() ? 0 : 1;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("keybindingsController"), &keybindings);
    engine.loadFromModule(QStringLiteral("org.kaveen.rivercontrol"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
