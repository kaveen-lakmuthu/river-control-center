#include "settingscontroller.h"

#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>

namespace {
constexpr auto kPurple = "#9A78B4";
}

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
{
    const QString configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QString appRoot = QDir(configRoot).filePath(QStringLiteral("river-control-center"));
    m_settingsPath = QDir(appRoot).filePath(QStringLiteral("settings.json"));
    m_generatedPath = QDir(appRoot).filePath(QStringLiteral("generated"));

    setDefaults();
    QString error;
    if (!loadFromDisk(&error))
        setStatus(error, true);
}

void SettingsController::setDefaults()
{
    m_loading = true;
    m_base = QStringLiteral("#08111F");
    m_surface = QStringLiteral("#101C2C");
    m_secondarySurface = QStringLiteral("#17283D");
    m_text = QStringLiteral("#EEF2F6");
    m_mutedText = QStringLiteral("#A8B4C3");
    m_accent = QStringLiteral("#D6AE4A");
    m_secondaryAccent = QStringLiteral("#4C83C3");
    m_highlight = QStringLiteral("#79AFC7");
    m_urgent = QStringLiteral("#B94747");
    m_success = QStringLiteral("#5F9C83");
    m_borderUnfocused = QStringLiteral("#27374A");
    m_borderWidth = 2;
    m_viewPadding = 0;
    m_outerPadding = 0;
    m_barHeight = 26;
    m_barFontSize = 11;
    m_modulePadding = 7;
    m_tagPadding = 6;
    m_clockFormat = QStringLiteral("{:%a %d/%m/%Y (%b)  %H:%M:%S}");
    m_batteryWarning = 25;
    m_batteryCritical = 12;
    m_showTags = true;
    m_showWindowTitle = true;
    m_showClock = true;
    m_showVolume = true;
    m_showBattery = true;
    m_showKeyboardLayout = true;
    m_showClipboard = true;
    m_showWindowControls = true;
    m_showTray = true;
    m_showSystemMenu = true;
    m_clipboardHistoryLimit = 40;
    m_notificationHistoryLimit = 25;
    m_panelWidth = 430;
    m_panelHeight = 420;
    m_panelPadding = 16;
    m_panelTopMargin = 4;
    m_panelRightMargin = 6;
    m_loading = false;
}

QString SettingsController::normalizedColor(const QString &value) const
{
    const QColor color(value.trimmed());
    if (!color.isValid())
        return {};
    return color.name(QColor::HexRgb).toUpper();
}

QString SettingsController::colorWithoutHash(const QString &value) const
{
    QString result = normalizedColor(value);
    if (result.startsWith(QLatin1Char('#')))
        result.remove(0, 1);
    return result.toLower();
}

QString SettingsController::lighter(const QString &value, int factor) const
{
    return QColor(value).lighter(factor).name(QColor::HexRgb).remove(QLatin1Char('#')).toLower();
}

QString SettingsController::colorValue(const QString &key) const
{
    if (key == QLatin1String("base")) return m_base;
    if (key == QLatin1String("surface")) return m_surface;
    if (key == QLatin1String("secondarySurface")) return m_secondarySurface;
    if (key == QLatin1String("text")) return m_text;
    if (key == QLatin1String("mutedText")) return m_mutedText;
    if (key == QLatin1String("accent")) return m_accent;
    if (key == QLatin1String("secondaryAccent")) return m_secondaryAccent;
    if (key == QLatin1String("highlight")) return m_highlight;
    if (key == QLatin1String("urgent")) return m_urgent;
    if (key == QLatin1String("success")) return m_success;
    if (key == QLatin1String("borderUnfocused")) return m_borderUnfocused;
    return {};
}

bool SettingsController::setColor(const QString &key, const QString &value)
{
    const QString normalized = normalizedColor(value);
    if (normalized.isEmpty()) {
        setStatus(QStringLiteral("%1 is not a valid colour.").arg(value), true);
        return false;
    }

    QString *target = nullptr;
    if (key == QLatin1String("base")) target = &m_base;
    else if (key == QLatin1String("surface")) target = &m_surface;
    else if (key == QLatin1String("secondarySurface")) target = &m_secondarySurface;
    else if (key == QLatin1String("text")) target = &m_text;
    else if (key == QLatin1String("mutedText")) target = &m_mutedText;
    else if (key == QLatin1String("accent")) target = &m_accent;
    else if (key == QLatin1String("secondaryAccent")) target = &m_secondaryAccent;
    else if (key == QLatin1String("highlight")) target = &m_highlight;
    else if (key == QLatin1String("urgent")) target = &m_urgent;
    else if (key == QLatin1String("success")) target = &m_success;
    else if (key == QLatin1String("borderUnfocused")) target = &m_borderUnfocused;

    if (!target) {
        setStatus(QStringLiteral("Unknown colour role: %1").arg(key), true);
        return false;
    }
    if (*target == normalized)
        return true;

    *target = normalized;
    markChanged();
    return true;
}

void SettingsController::setBorderWidth(int value)
{
    value = qBound(0, value, 12);
    if (m_borderWidth == value) return;
    m_borderWidth = value;
    markChanged();
}

void SettingsController::setViewPadding(int value)
{
    value = qBound(0, value, 32);
    if (m_viewPadding == value) return;
    m_viewPadding = value;
    markChanged();
}

void SettingsController::setOuterPadding(int value)
{
    value = qBound(0, value, 32);
    if (m_outerPadding == value) return;
    m_outerPadding = value;
    markChanged();
}

#define DEFINE_BOUNDED_SETTER(Name, Member, Minimum, Maximum) \
    void SettingsController::Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) return; \
        Member = value; \
        markChanged(); \
    }

DEFINE_BOUNDED_SETTER(setBarHeight, m_barHeight, 20, 64)
DEFINE_BOUNDED_SETTER(setBarFontSize, m_barFontSize, 8, 24)
DEFINE_BOUNDED_SETTER(setModulePadding, m_modulePadding, 0, 24)
DEFINE_BOUNDED_SETTER(setTagPadding, m_tagPadding, 0, 24)
DEFINE_BOUNDED_SETTER(setBatteryWarning, m_batteryWarning, 2, 80)
DEFINE_BOUNDED_SETTER(setBatteryCritical, m_batteryCritical, 1, 50)
DEFINE_BOUNDED_SETTER(setClipboardHistoryLimit, m_clipboardHistoryLimit, 5, 200)
DEFINE_BOUNDED_SETTER(setNotificationHistoryLimit, m_notificationHistoryLimit, 5, 100)
DEFINE_BOUNDED_SETTER(setPanelWidth, m_panelWidth, 300, 900)
DEFINE_BOUNDED_SETTER(setPanelHeight, m_panelHeight, 240, 900)
DEFINE_BOUNDED_SETTER(setPanelPadding, m_panelPadding, 4, 32)
DEFINE_BOUNDED_SETTER(setPanelTopMargin, m_panelTopMargin, 0, 48)
DEFINE_BOUNDED_SETTER(setPanelRightMargin, m_panelRightMargin, 0, 48)

#undef DEFINE_BOUNDED_SETTER

#define DEFINE_BOOL_SETTER(Name, Member) \
    void SettingsController::Name(bool value) \
    { \
        if (Member == value) return; \
        Member = value; \
        markChanged(); \
    }

DEFINE_BOOL_SETTER(setShowTags, m_showTags)
DEFINE_BOOL_SETTER(setShowWindowTitle, m_showWindowTitle)
DEFINE_BOOL_SETTER(setShowClock, m_showClock)
DEFINE_BOOL_SETTER(setShowVolume, m_showVolume)
DEFINE_BOOL_SETTER(setShowBattery, m_showBattery)
DEFINE_BOOL_SETTER(setShowKeyboardLayout, m_showKeyboardLayout)
DEFINE_BOOL_SETTER(setShowClipboard, m_showClipboard)
DEFINE_BOOL_SETTER(setShowWindowControls, m_showWindowControls)
DEFINE_BOOL_SETTER(setShowTray, m_showTray)
DEFINE_BOOL_SETTER(setShowSystemMenu, m_showSystemMenu)

#undef DEFINE_BOOL_SETTER

void SettingsController::setClockFormat(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (m_clockFormat == trimmed) return;
    m_clockFormat = trimmed;
    markChanged();
}

void SettingsController::markChanged()
{
    if (m_loading) return;
    ++m_revision;
    setDirty(true);
    emit settingsChanged();
}

void SettingsController::setDirty(bool value)
{
    if (m_dirty == value) return;
    m_dirty = value;
    emit dirtyChanged();
}

void SettingsController::setStatus(const QString &message, bool error)
{
    m_statusMessage = message;
    m_statusError = error;
    emit statusChanged();
}

void SettingsController::applyPreset(const QString &name)
{
    m_loading = true;
    if (name.compare(QStringLiteral("Ocean"), Qt::CaseInsensitive) == 0) {
        m_base = QStringLiteral("#08131C");
        m_surface = QStringLiteral("#102432");
        m_secondarySurface = QStringLiteral("#173747");
        m_text = QStringLiteral("#ECF5F7");
        m_mutedText = QStringLiteral("#9CB6BD");
        m_accent = QStringLiteral("#58A6C7");
        m_secondaryAccent = QStringLiteral("#6E91D7");
        m_highlight = QStringLiteral("#88C0D0");
        m_urgent = QStringLiteral("#C65C66");
        m_success = QStringLiteral("#6FB596");
        m_borderUnfocused = QStringLiteral("#2B4654");
    } else if (name.compare(QStringLiteral("Graphite"), Qt::CaseInsensitive) == 0) {
        m_base = QStringLiteral("#111318");
        m_surface = QStringLiteral("#191C22");
        m_secondarySurface = QStringLiteral("#252932");
        m_text = QStringLiteral("#F0F1F3");
        m_mutedText = QStringLiteral("#A5A9B2");
        m_accent = QStringLiteral("#C9A85B");
        m_secondaryAccent = QStringLiteral("#7E9AC7");
        m_highlight = QStringLiteral("#9CB3C8");
        m_urgent = QStringLiteral("#C75D63");
        m_success = QStringLiteral("#72A889");
        m_borderUnfocused = QStringLiteral("#353A45");
    } else {
        setDefaults();
    }
    m_loading = false;
    markChanged();
    setStatus(QStringLiteral("%1 preset loaded. Apply to save it.").arg(name));
}

QString SettingsController::validationError() const
{
    const struct { const char *name; const QString *value; } colors[] = {
        {"Base", &m_base}, {"Surface", &m_surface},
        {"Secondary surface", &m_secondarySurface}, {"Text", &m_text},
        {"Muted text", &m_mutedText}, {"Accent", &m_accent},
        {"Secondary accent", &m_secondaryAccent}, {"Highlight", &m_highlight},
        {"Urgent", &m_urgent}, {"Success", &m_success},
        {"Unfocused border", &m_borderUnfocused}
    };
    for (const auto &entry : colors) {
        if (!QColor(*entry.value).isValid())
            return QStringLiteral("%1 has an invalid colour value: %2").arg(QLatin1String(entry.name), *entry.value);
    }
    if (m_borderWidth < 0 || m_borderWidth > 12)
        return QStringLiteral("Border width must be between 0 and 12.");
    if (m_viewPadding < 0 || m_viewPadding > 32 || m_outerPadding < 0 || m_outerPadding > 32)
        return QStringLiteral("Layout padding must be between 0 and 32.");
    if (m_barHeight < 20 || m_barHeight > 64)
        return QStringLiteral("Waybar height must be between 20 and 64 pixels.");
    if (m_barFontSize < 8 || m_barFontSize > 24)
        return QStringLiteral("Waybar font size must be between 8 and 24 pixels.");
    if (m_clockFormat.isEmpty() || m_clockFormat.size() > 160)
        return QStringLiteral("Clock format must contain between 1 and 160 characters.");
    if (m_batteryCritical >= m_batteryWarning)
        return QStringLiteral("Critical battery level must be lower than the warning level.");
    if (m_panelWidth < 300 || m_panelWidth > 900 || m_panelHeight < 240 || m_panelHeight > 900)
        return QStringLiteral("Panel dimensions are outside the supported range.");
    return {};
}

bool SettingsController::loadFromDisk(QString *error)
{
    QFile file(m_settingsPath);
    if (!file.exists()) {
        setDirty(true);
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("Could not read %1: %2").arg(m_settingsPath, file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) *error = QStringLiteral("Invalid settings JSON: %1").arg(parseError.errorString());
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonObject appearance = root.value(QStringLiteral("appearance")).toObject();
    const QJsonObject river = root.value(QStringLiteral("river")).toObject();
    const QJsonObject bar = root.value(QStringLiteral("bar")).toObject();
    const QJsonObject modules = bar.value(QStringLiteral("modules")).toObject();
    const QJsonObject panels = root.value(QStringLiteral("panels")).toObject();
    auto readColor = [&appearance](const QString &key, const QString &fallback) {
        const QColor value(appearance.value(key).toString(fallback));
        return value.isValid() ? value.name(QColor::HexRgb).toUpper() : fallback;
    };

    m_loading = true;
    m_base = readColor(QStringLiteral("base"), m_base);
    m_surface = readColor(QStringLiteral("surface"), m_surface);
    m_secondarySurface = readColor(QStringLiteral("secondarySurface"), m_secondarySurface);
    m_text = readColor(QStringLiteral("text"), m_text);
    m_mutedText = readColor(QStringLiteral("mutedText"), m_mutedText);
    m_accent = readColor(QStringLiteral("accent"), m_accent);
    m_secondaryAccent = readColor(QStringLiteral("secondaryAccent"), m_secondaryAccent);
    m_highlight = readColor(QStringLiteral("highlight"), m_highlight);
    m_urgent = readColor(QStringLiteral("urgent"), m_urgent);
    m_success = readColor(QStringLiteral("success"), m_success);
    m_borderUnfocused = readColor(QStringLiteral("borderUnfocused"), m_borderUnfocused);
    m_borderWidth = qBound(0, river.value(QStringLiteral("borderWidth")).toInt(m_borderWidth), 12);
    m_viewPadding = qBound(0, river.value(QStringLiteral("viewPadding")).toInt(m_viewPadding), 32);
    m_outerPadding = qBound(0, river.value(QStringLiteral("outerPadding")).toInt(m_outerPadding), 32);
    m_barHeight = qBound(20, bar.value(QStringLiteral("height")).toInt(m_barHeight), 64);
    m_barFontSize = qBound(8, bar.value(QStringLiteral("fontSize")).toInt(m_barFontSize), 24);
    m_modulePadding = qBound(0, bar.value(QStringLiteral("modulePadding")).toInt(m_modulePadding), 24);
    m_tagPadding = qBound(0, bar.value(QStringLiteral("tagPadding")).toInt(m_tagPadding), 24);
    m_clockFormat = bar.value(QStringLiteral("clockFormat")).toString(m_clockFormat);
    m_batteryWarning = qBound(2, bar.value(QStringLiteral("batteryWarning")).toInt(m_batteryWarning), 80);
    m_batteryCritical = qBound(1, bar.value(QStringLiteral("batteryCritical")).toInt(m_batteryCritical), 50);
    m_showTags = modules.value(QStringLiteral("tags")).toBool(m_showTags);
    m_showWindowTitle = modules.value(QStringLiteral("windowTitle")).toBool(m_showWindowTitle);
    m_showClock = modules.value(QStringLiteral("clock")).toBool(m_showClock);
    m_showVolume = modules.value(QStringLiteral("volume")).toBool(m_showVolume);
    m_showBattery = modules.value(QStringLiteral("battery")).toBool(m_showBattery);
    m_showKeyboardLayout = modules.value(QStringLiteral("keyboardLayout")).toBool(m_showKeyboardLayout);
    m_showClipboard = modules.value(QStringLiteral("clipboard")).toBool(m_showClipboard);
    m_showWindowControls = modules.value(QStringLiteral("windowControls")).toBool(m_showWindowControls);
    m_showTray = modules.value(QStringLiteral("tray")).toBool(m_showTray);
    m_showSystemMenu = modules.value(QStringLiteral("systemMenu")).toBool(m_showSystemMenu);
    m_clipboardHistoryLimit = qBound(5, panels.value(QStringLiteral("clipboardHistoryLimit")).toInt(m_clipboardHistoryLimit), 200);
    m_notificationHistoryLimit = qBound(5, panels.value(QStringLiteral("notificationHistoryLimit")).toInt(m_notificationHistoryLimit), 100);
    m_panelWidth = qBound(300, panels.value(QStringLiteral("width")).toInt(m_panelWidth), 900);
    m_panelHeight = qBound(240, panels.value(QStringLiteral("height")).toInt(m_panelHeight), 900);
    m_panelPadding = qBound(4, panels.value(QStringLiteral("padding")).toInt(m_panelPadding), 32);
    m_panelTopMargin = qBound(0, panels.value(QStringLiteral("topMargin")).toInt(m_panelTopMargin), 48);
    m_panelRightMargin = qBound(0, panels.value(QStringLiteral("rightMargin")).toInt(m_panelRightMargin), 48);
    m_loading = false;
    ++m_revision;
    setDirty(false);
    emit settingsChanged();
    return true;
}

bool SettingsController::writeFileAtomically(const QString &path, const QByteArray &contents, QString *error) const
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = QStringLiteral("Could not write %1: %2").arg(path, file.errorString());
        return false;
    }
    if (file.write(contents) != contents.size() || !file.commit()) {
        if (error) *error = QStringLiteral("Could not commit %1: %2").arg(path, file.errorString());
        return false;
    }
    return true;
}

bool SettingsController::writeSettings(QString *error)
{
    const QFileInfo current(m_settingsPath);
    if (current.exists()) {
        const QString backupDir = QDir(current.absolutePath()).filePath(QStringLiteral("backups"));
        QDir().mkpath(backupDir);
        const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
        QFile::copy(m_settingsPath, QDir(backupDir).filePath(QStringLiteral("settings-%1.json").arg(stamp)));

        QDir backups(backupDir);
        const QFileInfoList entries = backups.entryInfoList({QStringLiteral("settings-*.json")}, QDir::Files, QDir::Time);
        for (qsizetype index = 10; index < entries.size(); ++index)
            QFile::remove(entries.at(index).absoluteFilePath());
    }

    QJsonObject appearance;
    appearance.insert(QStringLiteral("base"), m_base);
    appearance.insert(QStringLiteral("surface"), m_surface);
    appearance.insert(QStringLiteral("secondarySurface"), m_secondarySurface);
    appearance.insert(QStringLiteral("text"), m_text);
    appearance.insert(QStringLiteral("mutedText"), m_mutedText);
    appearance.insert(QStringLiteral("accent"), m_accent);
    appearance.insert(QStringLiteral("secondaryAccent"), m_secondaryAccent);
    appearance.insert(QStringLiteral("highlight"), m_highlight);
    appearance.insert(QStringLiteral("urgent"), m_urgent);
    appearance.insert(QStringLiteral("success"), m_success);
    appearance.insert(QStringLiteral("borderUnfocused"), m_borderUnfocused);

    QJsonObject river;
    river.insert(QStringLiteral("borderWidth"), m_borderWidth);
    river.insert(QStringLiteral("viewPadding"), m_viewPadding);
    river.insert(QStringLiteral("outerPadding"), m_outerPadding);

    QJsonObject modules;
    modules.insert(QStringLiteral("tags"), m_showTags);
    modules.insert(QStringLiteral("windowTitle"), m_showWindowTitle);
    modules.insert(QStringLiteral("clock"), m_showClock);
    modules.insert(QStringLiteral("volume"), m_showVolume);
    modules.insert(QStringLiteral("battery"), m_showBattery);
    modules.insert(QStringLiteral("keyboardLayout"), m_showKeyboardLayout);
    modules.insert(QStringLiteral("clipboard"), m_showClipboard);
    modules.insert(QStringLiteral("windowControls"), m_showWindowControls);
    modules.insert(QStringLiteral("tray"), m_showTray);
    modules.insert(QStringLiteral("systemMenu"), m_showSystemMenu);

    QJsonObject bar;
    bar.insert(QStringLiteral("height"), m_barHeight);
    bar.insert(QStringLiteral("fontSize"), m_barFontSize);
    bar.insert(QStringLiteral("modulePadding"), m_modulePadding);
    bar.insert(QStringLiteral("tagPadding"), m_tagPadding);
    bar.insert(QStringLiteral("clockFormat"), m_clockFormat);
    bar.insert(QStringLiteral("batteryWarning"), m_batteryWarning);
    bar.insert(QStringLiteral("batteryCritical"), m_batteryCritical);
    bar.insert(QStringLiteral("modules"), modules);

    QJsonObject panels;
    panels.insert(QStringLiteral("clipboardHistoryLimit"), m_clipboardHistoryLimit);
    panels.insert(QStringLiteral("notificationHistoryLimit"), m_notificationHistoryLimit);
    panels.insert(QStringLiteral("width"), m_panelWidth);
    panels.insert(QStringLiteral("height"), m_panelHeight);
    panels.insert(QStringLiteral("padding"), m_panelPadding);
    panels.insert(QStringLiteral("topMargin"), m_panelTopMargin);
    panels.insert(QStringLiteral("rightMargin"), m_panelRightMargin);

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 2);
    root.insert(QStringLiteral("appearance"), appearance);
    root.insert(QStringLiteral("river"), river);
    root.insert(QStringLiteral("bar"), bar);
    root.insert(QStringLiteral("panels"), panels);
    return writeFileAtomically(m_settingsPath, QJsonDocument(root).toJson(QJsonDocument::Indented), error);
}

bool SettingsController::generateFiles(QString *error)
{
    const QString river = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_BASE='%1'\nRCC_SURFACE='%2'\nRCC_SECONDARY_SURFACE='%3'\n"
        "RCC_TEXT='%4'\nRCC_MUTED_TEXT='%5'\nRCC_ACCENT='%6'\n"
        "RCC_SECONDARY_ACCENT='%7'\nRCC_HIGHLIGHT='%8'\nRCC_URGENT='%9'\n"
        "RCC_SUCCESS='%10'\nRCC_BORDER_UNFOCUSED='%11'\nRCC_BORDER_WIDTH=%12\n"
        "RCC_VIEW_PADDING=%13\nRCC_OUTER_PADDING=%14\n")
        .arg(m_base, m_surface, m_secondarySurface, m_text, m_mutedText, m_accent,
             m_secondaryAccent, m_highlight, m_urgent, m_success, m_borderUnfocused)
        .arg(m_borderWidth).arg(m_viewPadding).arg(m_outerPadding);

    const QString css = QStringLiteral(
        "/* Generated by River Control Center. */\n"
        "@define-color rcc_base %1;\n@define-color rcc_surface %2;\n"
        "@define-color rcc_secondary_surface %3;\n@define-color rcc_text %4;\n"
        "@define-color rcc_muted_text %5;\n@define-color rcc_accent %6;\n"
        "@define-color rcc_secondary_accent %7;\n@define-color rcc_highlight %8;\n"
        "@define-color rcc_urgent %9;\n@define-color rcc_success %10;\n"
        "@define-color rcc_border_unfocused %11;\n@define-color rcc_subtle_text mix(%5, %2, 0.68);\n"
        "@define-color rcc_occupied mix(%4, %5, 0.36);\n")
        .arg(m_base, m_surface, m_secondarySurface, m_text, m_mutedText, m_accent,
             m_secondaryAccent, m_highlight, m_urgent, m_success, m_borderUnfocused);

    const QString foot = QStringLiteral(
        "# Generated by River Control Center.\n[colors-dark]\n"
        "background=%1\nforeground=%2\nregular0=%1\nregular1=%3\nregular2=%4\n"
        "regular3=%5\nregular4=%6\nregular5=%7\nregular6=%8\nregular7=%9\n"
        "bright0=%10\nbright1=%11\nbright2=%12\nbright3=%13\nbright4=%14\n"
        "bright5=%15\nbright6=%16\nbright7=%2\nselection-foreground=%2\n"
        "selection-background=%17\ncursor=%1 %5\njump-labels=%1 %5\n"
        "scrollback-indicator=%1 %8\nsearch-box-no-match=%2 %3\n"
        "search-box-match=%1 %5\nurls=%8\n")
        .arg(colorWithoutHash(m_base), colorWithoutHash(m_text), colorWithoutHash(m_urgent),
             colorWithoutHash(m_success), colorWithoutHash(m_accent), colorWithoutHash(m_secondaryAccent),
             colorWithoutHash(QString::fromLatin1(kPurple)), colorWithoutHash(m_highlight),
             colorWithoutHash(m_mutedText), colorWithoutHash(m_borderUnfocused), lighter(m_urgent),
             lighter(m_success), lighter(m_accent), lighter(m_secondaryAccent),
             lighter(QString::fromLatin1(kPurple)), lighter(m_highlight), colorWithoutHash(m_secondarySurface));

    auto rgba = [this](const QString &value) { return colorWithoutHash(value) + QStringLiteral("ff"); };
    const QString fuzzel = QStringLiteral(
        "# Generated by River Control Center.\n[colors]\n"
        "background=%1\ntext=%2\nmessage=%3\nprompt=%4\nplaceholder=%3\n"
        "input=%2\nmatch=%4\nselection=%5\nselection-text=%2\n"
        "selection-match=%4\ncounter=%3\nborder=%4\n")
        .arg(rgba(m_surface), rgba(m_text), rgba(m_mutedText), rgba(m_accent), rgba(m_secondarySurface));

    const QString mako = QStringLiteral(
        "# Generated by River Control Center.\nbackground-color=%1\ntext-color=%2\n"
        "border-color=%3\nmax-history=%4\n")
        .arg(m_surface, m_text, m_accent).arg(m_notificationHistoryLimit);

    QJsonArray modulesLeft;
    if (m_showTags) modulesLeft.append(QStringLiteral("river/tags"));
    if (m_showWindowTitle) modulesLeft.append(QStringLiteral("river/window"));

    QJsonArray modulesCenter;
    if (m_showClock) modulesCenter.append(QStringLiteral("clock"));

    QJsonArray essentials;
    if (m_showVolume) essentials.append(QStringLiteral("pulseaudio"));
    if (m_showBattery) essentials.append(QStringLiteral("battery"));
    if (m_showKeyboardLayout) essentials.append(QStringLiteral("custom/keyboard-layout"));
    if (m_showClipboard) essentials.append(QStringLiteral("custom/clipboard"));

    QJsonArray modulesRight;
    if (!essentials.isEmpty()) modulesRight.append(QStringLiteral("group/essentials"));
    if (m_showWindowControls) modulesRight.append(QStringLiteral("custom/window-controls"));
    if (m_showTray) modulesRight.append(QStringLiteral("tray"));
    if (m_showSystemMenu) modulesRight.append(QStringLiteral("custom/system-menu"));

    QJsonObject waybarSettings;
    waybarSettings.insert(QStringLiteral("height"), m_barHeight);
    waybarSettings.insert(QStringLiteral("spacing"), 0);
    waybarSettings.insert(QStringLiteral("margin-top"), 0);
    waybarSettings.insert(QStringLiteral("margin-right"), 0);
    waybarSettings.insert(QStringLiteral("margin-bottom"), 0);
    waybarSettings.insert(QStringLiteral("margin-left"), 0);
    waybarSettings.insert(QStringLiteral("reload_style_on_change"), true);
    waybarSettings.insert(QStringLiteral("modules-left"), modulesLeft);
    waybarSettings.insert(QStringLiteral("modules-center"), modulesCenter);
    waybarSettings.insert(QStringLiteral("modules-right"), modulesRight);
    waybarSettings.insert(QStringLiteral("group/essentials"),
                          QJsonObject{{QStringLiteral("orientation"), QStringLiteral("horizontal")},
                                      {QStringLiteral("modules"), essentials}});
    waybarSettings.insert(QStringLiteral("clock"),
                          QJsonObject{{QStringLiteral("format"), m_clockFormat},
                                      {QStringLiteral("interval"), m_clockFormat.contains(QStringLiteral("%S")) ? 1 : 60}});
    waybarSettings.insert(QStringLiteral("battery"),
                          QJsonObject{
                              {QStringLiteral("interval"), 30},
                              {QStringLiteral("format"), QStringLiteral("BAT {capacity}%")},
                              {QStringLiteral("format-charging"), QStringLiteral("BAT {capacity}% +")},
                              {QStringLiteral("format-full"), QStringLiteral("BAT 100%")},
                              {QStringLiteral("states"),
                               QJsonObject{{QStringLiteral("warning"), m_batteryWarning},
                                           {QStringLiteral("critical"), m_batteryCritical}}},
                              {QStringLiteral("events"),
                               QJsonObject{
                                   {QStringLiteral("on-discharging-warning"),
                                    QStringLiteral("sh -c 'level=$(cat /sys/class/power_supply/BAT0/capacity); notify-send -a Battery -u normal \"Low battery\" \"Battery is at ${level}%.\"'")},
                                   {QStringLiteral("on-discharging-critical"),
                                    QStringLiteral("sh -c 'level=$(cat /sys/class/power_supply/BAT0/capacity); notify-send -a Battery -u critical \"Critical battery\" \"Battery is at ${level}%. Connect a charger.\"'")}}}});
    waybarSettings.insert(QStringLiteral("tray"),
                          QJsonObject{{QStringLiteral("icon-size"), 14},
                                      {QStringLiteral("spacing"), 5}});

    const QByteArray waybar = QJsonDocument(waybarSettings).toJson(QJsonDocument::Indented);
    const QString waybarLayout = QStringLiteral(
        "/* Generated by River Control Center. */\n"
        "* { font-size: %1px; }\n"
        "#tags button { padding: 0 %2px; }\n"
        "#window, #clock, #custom-clipboard, #custom-keyboard-layout,\n"
        "#custom-window-controls, #pulseaudio, #battery { padding: 0 %3px; }\n"
        "#tray { padding: 0 %4px; }\n"
        "#custom-system-menu { padding: 0 %3px; }\n")
        .arg(m_barFontSize).arg(m_tagPadding).arg(m_modulePadding).arg(qMax(0, m_modulePadding - 1));

    QJsonObject panelSettings;
    panelSettings.insert(QStringLiteral("clipboardHistoryLimit"), m_clipboardHistoryLimit);
    panelSettings.insert(QStringLiteral("notificationHistoryLimit"), m_notificationHistoryLimit);
    panelSettings.insert(QStringLiteral("width"), m_panelWidth);
    panelSettings.insert(QStringLiteral("height"), m_panelHeight);
    panelSettings.insert(QStringLiteral("padding"), m_panelPadding);
    panelSettings.insert(QStringLiteral("topMargin"), m_panelTopMargin);
    panelSettings.insert(QStringLiteral("rightMargin"), m_panelRightMargin);
    const QByteArray panelJson = QJsonDocument(panelSettings).toJson(QJsonDocument::Indented);

    return writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("river-theme.sh")), river.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("waybar-colors.css")), css.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("waybar-layout.css")), waybarLayout.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("waybar-settings.json")), waybar, error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("panel-settings.json")), panelJson, error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("foot-theme.ini")), foot.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("fuzzel-theme.ini")), fuzzel.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("mako-theme.conf")), mako.toUtf8(), error);
}

bool SettingsController::regenerate(QString *error)
{
    const QString invalid = validationError();
    if (!invalid.isEmpty()) {
        if (error) *error = invalid;
        return false;
    }
    return generateFiles(error);
}

bool SettingsController::run(const QString &program, const QStringList &arguments, int timeoutMs) const
{
    QProcess process;
    process.start(program, arguments);
    return process.waitForStarted(timeoutMs) && process.waitForFinished(timeoutMs)
        && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void SettingsController::applyLive()
{
    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    if (!desktop.contains(QStringLiteral("river"), Qt::CaseInsensitive)
        || qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
        setStatus(QStringLiteral("Saved. The theme will be applied when the next River session starts."));
        return;
    }

    const bool riverAvailable = run(QStringLiteral("riverctl"),
                                    {QStringLiteral("background-color"), QStringLiteral("0x") + colorWithoutHash(m_base)});
    if (riverAvailable) {
        run(QStringLiteral("riverctl"), {QStringLiteral("border-color-focused"),
                                          QStringLiteral("0x") + colorWithoutHash(m_accent)});
        run(QStringLiteral("riverctl"), {QStringLiteral("border-color-unfocused"),
                                          QStringLiteral("0x") + colorWithoutHash(m_borderUnfocused)});
        run(QStringLiteral("riverctl"), {QStringLiteral("border-width"), QString::number(m_borderWidth)});
    }

    run(QStringLiteral("pkill"), {QStringLiteral("-USR2"), QStringLiteral("waybar")});
    run(QStringLiteral("makoctl"), {QStringLiteral("reload")});

    if (run(QStringLiteral("pgrep"), {QStringLiteral("-x"), QStringLiteral("swaybg")})) {
        run(QStringLiteral("pkill"), {QStringLiteral("-x"), QStringLiteral("swaybg")});
        QProcess::startDetached(QStringLiteral("swaybg"), {QStringLiteral("-c"), m_base});
    }

    if (riverAvailable)
        setStatus(QStringLiteral("Settings applied. Layout padding will update at the next River login."));
    else
        setStatus(QStringLiteral("Files saved, but River did not accept the live update."), true);
}

bool SettingsController::saveAndApply()
{
    const QString invalid = validationError();
    if (!invalid.isEmpty()) {
        setStatus(invalid, true);
        return false;
    }

    QString error;
    if (!writeSettings(&error) || !generateFiles(&error)) {
        setStatus(error, true);
        return false;
    }

    setDirty(false);
    applyLive();
    return true;
}

void SettingsController::reload()
{
    setDefaults();
    QString error;
    if (!loadFromDisk(&error)) {
        setStatus(error, true);
        return;
    }
    setStatus(QStringLiteral("Discarded unsaved changes."));
}
