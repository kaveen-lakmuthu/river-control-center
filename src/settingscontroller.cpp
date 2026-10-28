#include "settingscontroller.h"
#include "windowenumerator.h"

#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>
#include <cmath>

namespace {
constexpr auto kPurple = "#9A78B4";

QVariantMap makeRule(const QString &appId, const QString &titleContains,
                     const QString &action)
{
    return {{QStringLiteral("appId"), appId},
            {QStringLiteral("titleContains"), titleContains},
            {QStringLiteral("action"), action}};
}

QVariantList defaultWindowRules()
{
    QVariantList rules;
    rules << makeRule(QStringLiteral("emacs"), {}, QStringLiteral("ssd"));
    const QStringList floatingAppIds = {
        QStringLiteral("pavucontrol"),
        QStringLiteral("blueman-manager"),
        QStringLiteral("nm-connection-editor"),
        QStringLiteral("org.kde.kcalc"),
        QStringLiteral("org.kde.haruna"),
        QStringLiteral("haruna"),
        QStringLiteral("vlc"),
        QStringLiteral("org.videolan.VLC"),
        QStringLiteral("xdg-desktop-portal-gtk"),
        QStringLiteral("org.freedesktop.impl.portal.desktop.gtk")};
    for (const QString &appId : floatingAppIds)
        rules << makeRule(appId, {}, QStringLiteral("float"));

    const QStringList dialogTitles = {
        QStringLiteral("Open File"), QStringLiteral("Save File"),
        QStringLiteral("Save As"), QStringLiteral("Choose File"),
        QStringLiteral("Select File"), QStringLiteral("File Upload"),
        QStringLiteral("wants to open")};
    for (const QString &title : dialogTitles)
        rules << makeRule({}, title, QStringLiteral("float"));
    const QStringList browserAppIds = {
        QStringLiteral("firefox"), QStringLiteral("org.mozilla.firefox"),
        QStringLiteral("brave-origin"), QStringLiteral("com.brave.Origin")};
    for (const QString &appId : browserAppIds) {
        rules << makeRule(appId, QStringLiteral("Download"), QStringLiteral("float"));
        rules << makeRule(appId, QStringLiteral("Opening "), QStringLiteral("float"));
    }
    return rules;
}

bool globMatches(const QString &glob, const QString &value)
{
    if (glob.isEmpty())
        return true;
    return QRegularExpression(QRegularExpression::wildcardToRegularExpression(glob)).match(value).hasMatch();
}

double relativeLuminance(const QColor &color)
{
    auto channel = [](double value) {
        value /= 255.0;
        return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(color.red()) + 0.7152 * channel(color.green())
        + 0.0722 * channel(color.blue());
}

double contrastRatio(const QColor &a, const QColor &b)
{
    const double first = relativeLuminance(a);
    const double second = relativeLuminance(b);
    return (qMax(first, second) + 0.05) / (qMin(first, second) + 0.05);
}
}

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
{
    m_configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QString appRoot = QDir(m_configRoot).filePath(QStringLiteral("river-control-center"));
    m_settingsPath = QDir(appRoot).filePath(QStringLiteral("settings.json"));
    m_generatedPath = QDir(appRoot).filePath(QStringLiteral("generated"));

    setDefaults();
    QString error;
    if (!loadFromDisk(&error))
        setStatus(error, true);
    m_lidClosePolicy = queryLidClosePolicy();
    refreshSessionStatus();
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
    m_waybarFontFamily = QStringLiteral("Noto Sans");
    m_makoFontFamily = QStringLiteral("Noto Sans");
    m_makoFontSize = 10;
    m_fuzzelFontFamily = QStringLiteral("Noto Sans");
    m_fuzzelFontSize = 11;
    m_footFontFamily = QStringLiteral("Hack");
    m_footFontSize = 10.5;
    m_panelFontFamily = QStringLiteral("Noto Sans");
    m_panelFontSize = 10;
    m_backgroundMode = QStringLiteral("solid");
    m_wallpaperPath.clear();
    m_interfaceRadius = 8;
    m_panelSpacing = 10;
    m_themeWaybar = true;
    m_themeMako = true;
    m_themeFuzzel = true;
    m_themeFoot = true;
    m_themePanels = true;
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
    m_lockTimeoutMinutes = 10;
    m_displayOffTimeoutMinutes = 15;
    m_batterySuspendTimeoutMinutes = 30;
    m_acSuspendTimeoutMinutes = 60;
    m_lockOnIdle = true;
    m_displayOffOnIdle = true;
    m_suspendOnBattery = true;
    m_suspendOnAc = true;
    m_lockBeforeSuspend = true;
    m_defaultPowerProfile = QStringLiteral("balanced");
    m_nightLightStart = QStringLiteral("18:00");
    m_nightLightStop = QStringLiteral("06:00");
    m_touchpadTap = true;
    m_touchpadNaturalScroll = true;
    m_touchpadDisableWhileTyping = true;
    m_touchpadPointerAccel = 0.1;
    m_touchpadClickMethod = QStringLiteral("clickfinger");
    m_keyboardRepeatRate = 50;
    m_keyboardRepeatDelay = 300;
    m_sinhalaLayoutEnabled = true;
    m_composeKey = QStringLiteral("compose:rctrl");
    m_gestureFingerCount = 3;
    m_gestureDirection = QStringLiteral("left-next");
    m_gestureDeviceName = QStringLiteral("MSFT0001:00 04F3:31BE Touchpad");
    m_mainWindowRatio = 0.6;
    m_mainWindowCount = 1;
    m_mainPosition = QStringLiteral("left");
    m_defaultAttachMode = QStringLiteral("bottom");
    m_focusPolicy = QStringLiteral("mode-aware");
    m_defaultLayoutMode = QStringLiteral("tiled");
    m_tagCount = 9;
    m_autostartWaybar = true;
    m_autostartMako = true;
    m_autostartKanshi = true;
    m_autostartFusuma = true;
    m_autostartSwayidle = true;
    m_windowRules = defaultWindowRules();
    m_savedWindowRules = m_windowRules;
    m_powerDirty = false;
    m_inputDirty = false;
    m_windowDirty = false;
    m_loading = false;
}

#define DEFINE_AUTOSTART_SETTER(Name, Member) \
    void SettingsController::Name(bool value) \
    { \
        if (Member == value) return; \
        Member = value; \
        markChanged(); \
    }

DEFINE_AUTOSTART_SETTER(setAutostartWaybar, m_autostartWaybar)
DEFINE_AUTOSTART_SETTER(setAutostartMako, m_autostartMako)
DEFINE_AUTOSTART_SETTER(setAutostartKanshi, m_autostartKanshi)
DEFINE_AUTOSTART_SETTER(setAutostartFusuma, m_autostartFusuma)
DEFINE_AUTOSTART_SETTER(setAutostartSwayidle, m_autostartSwayidle)

#undef DEFINE_AUTOSTART_SETTER

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

#define DEFINE_APPEARANCE_STRING_SETTER(Name, Member) \
    void SettingsController::Name(const QString &value) \
    { \
        const QString trimmed = value.trimmed(); \
        if (Member == trimmed) return; \
        Member = trimmed; \
        markChanged(); \
    }

DEFINE_APPEARANCE_STRING_SETTER(setWaybarFontFamily, m_waybarFontFamily)
DEFINE_APPEARANCE_STRING_SETTER(setMakoFontFamily, m_makoFontFamily)
DEFINE_APPEARANCE_STRING_SETTER(setFuzzelFontFamily, m_fuzzelFontFamily)
DEFINE_APPEARANCE_STRING_SETTER(setFootFontFamily, m_footFontFamily)
DEFINE_APPEARANCE_STRING_SETTER(setPanelFontFamily, m_panelFontFamily)
DEFINE_APPEARANCE_STRING_SETTER(setBackgroundMode, m_backgroundMode)

#undef DEFINE_APPEARANCE_STRING_SETTER

void SettingsController::setWallpaperPath(const QString &value)
{
    const QUrl url(value);
    const QString path = url.isLocalFile() ? url.toLocalFile() : value.trimmed();
    if (m_wallpaperPath == path) return;
    m_wallpaperPath = path;
    markChanged();
}

#define DEFINE_APPEARANCE_INT_SETTER(Name, Member, Minimum, Maximum) \
    void SettingsController::Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) return; \
        Member = value; \
        markChanged(); \
    }

DEFINE_APPEARANCE_INT_SETTER(setMakoFontSize, m_makoFontSize, 7, 28)
DEFINE_APPEARANCE_INT_SETTER(setFuzzelFontSize, m_fuzzelFontSize, 7, 28)
DEFINE_APPEARANCE_INT_SETTER(setPanelFontSize, m_panelFontSize, 7, 28)
DEFINE_APPEARANCE_INT_SETTER(setInterfaceRadius, m_interfaceRadius, 0, 30)
DEFINE_APPEARANCE_INT_SETTER(setPanelSpacing, m_panelSpacing, 0, 30)

#undef DEFINE_APPEARANCE_INT_SETTER

void SettingsController::setFootFontSize(double value)
{
    value = qBound(6.0, value, 32.0);
    if (qFuzzyCompare(m_footFontSize, value)) return;
    m_footFontSize = value;
    markChanged();
}

#define DEFINE_THEME_BOOL_SETTER(Name, Member) \
    void SettingsController::Name(bool value) \
    { \
        if (Member == value) return; \
        Member = value; \
        markChanged(); \
    }

DEFINE_THEME_BOOL_SETTER(setThemeWaybar, m_themeWaybar)
DEFINE_THEME_BOOL_SETTER(setThemeMako, m_themeMako)
DEFINE_THEME_BOOL_SETTER(setThemeFuzzel, m_themeFuzzel)
DEFINE_THEME_BOOL_SETTER(setThemeFoot, m_themeFoot)
DEFINE_THEME_BOOL_SETTER(setThemePanels, m_themePanels)

#undef DEFINE_THEME_BOOL_SETTER

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

#define DEFINE_POWER_BOUNDED_SETTER(Name, Member) \
    void SettingsController::Name(int value) \
    { \
        value = qBound(1, value, 360); \
        if (Member == value) return; \
        Member = value; \
        m_powerDirty = true; \
        markChanged(); \
    }

DEFINE_POWER_BOUNDED_SETTER(setLockTimeoutMinutes, m_lockTimeoutMinutes)
DEFINE_POWER_BOUNDED_SETTER(setDisplayOffTimeoutMinutes, m_displayOffTimeoutMinutes)
DEFINE_POWER_BOUNDED_SETTER(setBatterySuspendTimeoutMinutes, m_batterySuspendTimeoutMinutes)
DEFINE_POWER_BOUNDED_SETTER(setAcSuspendTimeoutMinutes, m_acSuspendTimeoutMinutes)

#undef DEFINE_POWER_BOUNDED_SETTER

#define DEFINE_POWER_BOOL_SETTER(Name, Member) \
    void SettingsController::Name(bool value) \
    { \
        if (Member == value) return; \
        Member = value; \
        m_powerDirty = true; \
        markChanged(); \
    }

DEFINE_POWER_BOOL_SETTER(setLockOnIdle, m_lockOnIdle)
DEFINE_POWER_BOOL_SETTER(setDisplayOffOnIdle, m_displayOffOnIdle)
DEFINE_POWER_BOOL_SETTER(setSuspendOnBattery, m_suspendOnBattery)
DEFINE_POWER_BOOL_SETTER(setSuspendOnAc, m_suspendOnAc)
DEFINE_POWER_BOOL_SETTER(setLockBeforeSuspend, m_lockBeforeSuspend)

#undef DEFINE_POWER_BOOL_SETTER

#define DEFINE_INPUT_BOOL_SETTER(Name, Member) \
    void SettingsController::Name(bool value) \
    { \
        if (Member == value) return; \
        Member = value; \
        m_inputDirty = true; \
        markChanged(); \
    }

DEFINE_INPUT_BOOL_SETTER(setTouchpadTap, m_touchpadTap)
DEFINE_INPUT_BOOL_SETTER(setTouchpadNaturalScroll, m_touchpadNaturalScroll)
DEFINE_INPUT_BOOL_SETTER(setTouchpadDisableWhileTyping, m_touchpadDisableWhileTyping)
DEFINE_INPUT_BOOL_SETTER(setSinhalaLayoutEnabled, m_sinhalaLayoutEnabled)

#undef DEFINE_INPUT_BOOL_SETTER

void SettingsController::setTouchpadPointerAccel(double value)
{
    value = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_touchpadPointerAccel, value)) return;
    m_touchpadPointerAccel = value;
    m_inputDirty = true;
    markChanged();
}

#define DEFINE_INPUT_BOUNDED_SETTER(Name, Member, Minimum, Maximum) \
    void SettingsController::Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) return; \
        Member = value; \
        m_inputDirty = true; \
        markChanged(); \
    }

DEFINE_INPUT_BOUNDED_SETTER(setKeyboardRepeatRate, m_keyboardRepeatRate, 1, 100)
DEFINE_INPUT_BOUNDED_SETTER(setKeyboardRepeatDelay, m_keyboardRepeatDelay, 100, 2000)
DEFINE_INPUT_BOUNDED_SETTER(setGestureFingerCount, m_gestureFingerCount, 3, 5)

#undef DEFINE_INPUT_BOUNDED_SETTER

#define DEFINE_INPUT_STRING_SETTER(Name, Member) \
    void SettingsController::Name(const QString &value) \
    { \
        const QString trimmed = value.trimmed(); \
        if (Member == trimmed) return; \
        Member = trimmed; \
        m_inputDirty = true; \
        markChanged(); \
    }

DEFINE_INPUT_STRING_SETTER(setTouchpadClickMethod, m_touchpadClickMethod)
DEFINE_INPUT_STRING_SETTER(setComposeKey, m_composeKey)
DEFINE_INPUT_STRING_SETTER(setGestureDirection, m_gestureDirection)
DEFINE_INPUT_STRING_SETTER(setGestureDeviceName, m_gestureDeviceName)

#undef DEFINE_INPUT_STRING_SETTER

void SettingsController::setMainWindowRatio(double value)
{
    value = qBound(0.1, value, 0.9);
    if (qFuzzyCompare(m_mainWindowRatio, value)) return;
    m_mainWindowRatio = value;
    m_windowDirty = true;
    markChanged();
}

#define DEFINE_WINDOW_BOUNDED_SETTER(Name, Member, Minimum, Maximum) \
    void SettingsController::Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) return; \
        Member = value; \
        m_windowDirty = true; \
        markChanged(); \
    }

DEFINE_WINDOW_BOUNDED_SETTER(setMainWindowCount, m_mainWindowCount, 1, 16)
DEFINE_WINDOW_BOUNDED_SETTER(setTagCount, m_tagCount, 1, 9)

#undef DEFINE_WINDOW_BOUNDED_SETTER

#define DEFINE_WINDOW_STRING_SETTER(Name, Member) \
    void SettingsController::Name(const QString &value) \
    { \
        const QString trimmed = value.trimmed(); \
        if (Member == trimmed) return; \
        Member = trimmed; \
        m_windowDirty = true; \
        markChanged(); \
    }

DEFINE_WINDOW_STRING_SETTER(setMainPosition, m_mainPosition)
DEFINE_WINDOW_STRING_SETTER(setDefaultAttachMode, m_defaultAttachMode)
DEFINE_WINDOW_STRING_SETTER(setFocusPolicy, m_focusPolicy)
DEFINE_WINDOW_STRING_SETTER(setDefaultLayoutMode, m_defaultLayoutMode)

#undef DEFINE_WINDOW_STRING_SETTER

void SettingsController::addWindowRule()
{
    m_windowRules.append(makeRule({}, {}, QStringLiteral("float")));
    m_windowDirty = true;
    emit windowRulesChanged();
    markChanged();
}

void SettingsController::removeWindowRule(int index)
{
    if (index < 0 || index >= m_windowRules.size()) return;
    m_windowRules.removeAt(index);
    m_windowDirty = true;
    emit windowRulesChanged();
    markChanged();
}

void SettingsController::updateWindowRule(int index, const QString &appId,
                                          const QString &titleContains,
                                          const QString &action)
{
    if (index < 0 || index >= m_windowRules.size()) return;
    const QVariantMap updated = makeRule(appId.trimmed(), titleContains.trimmed(),
                                         action.trimmed().toLower());
    if (m_windowRules.at(index).toMap() == updated) return;
    m_windowRules[index] = updated;
    m_windowDirty = true;
    emit windowRulesChanged();
    markChanged();
}

void SettingsController::refreshOpenWindows()
{
    QString error;
    m_openWindows = enumerateOpenWindows(&error);
    emit openWindowsChanged();
    if (!error.isEmpty()) {
        m_ruleTestResult = error;
        emit ruleTestResultChanged();
    }
}

void SettingsController::testWindowRule(int index)
{
    if (index < 0 || index >= m_windowRules.size()) return;
    refreshOpenWindows();

    const QVariantMap rule = m_windowRules.at(index).toMap();
    const QString appId = rule.value(QStringLiteral("appId")).toString();
    const QString titleContains = rule.value(QStringLiteral("titleContains")).toString();
    QStringList matches;
    for (const QVariant &entry : m_openWindows) {
        const QVariantMap window = entry.toMap();
        const QString windowAppId = window.value(QStringLiteral("appId")).toString();
        const QString windowTitle = window.value(QStringLiteral("title")).toString();
        if (globMatches(appId, windowAppId)
            && (titleContains.isEmpty() || windowTitle.contains(titleContains))) {
            matches.append(QStringLiteral("%1 — %2").arg(
                windowAppId.isEmpty() ? QStringLiteral("(no app ID)") : windowAppId,
                windowTitle.isEmpty() ? QStringLiteral("(untitled)") : windowTitle));
        }
    }

    if (matches.isEmpty())
        m_ruleTestResult = QStringLiteral("No currently open windows match this rule.");
    else
        m_ruleTestResult = QStringLiteral("Matched %1 window(s):\n%2")
            .arg(matches.size()).arg(matches.join(QLatin1Char('\n')));
    emit ruleTestResultChanged();
}

void SettingsController::setClockFormat(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (m_clockFormat == trimmed) return;
    m_clockFormat = trimmed;
    markChanged();
}

void SettingsController::setDefaultPowerProfile(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (m_defaultPowerProfile == trimmed) return;
    m_defaultPowerProfile = trimmed;
    m_powerDirty = true;
    markChanged();
}

void SettingsController::setNightLightStart(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (m_nightLightStart == trimmed) return;
    m_nightLightStart = trimmed;
    m_powerDirty = true;
    markChanged();
}

void SettingsController::setNightLightStop(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (m_nightLightStop == trimmed) return;
    m_nightLightStop = trimmed;
    m_powerDirty = true;
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

QJsonObject SettingsController::appearanceJson() const
{
    QJsonObject fonts;
    fonts.insert(QStringLiteral("waybarFamily"), m_waybarFontFamily);
    fonts.insert(QStringLiteral("makoFamily"), m_makoFontFamily);
    fonts.insert(QStringLiteral("makoSize"), m_makoFontSize);
    fonts.insert(QStringLiteral("fuzzelFamily"), m_fuzzelFontFamily);
    fonts.insert(QStringLiteral("fuzzelSize"), m_fuzzelFontSize);
    fonts.insert(QStringLiteral("footFamily"), m_footFontFamily);
    fonts.insert(QStringLiteral("footSize"), m_footFontSize);
    fonts.insert(QStringLiteral("panelFamily"), m_panelFontFamily);
    fonts.insert(QStringLiteral("panelSize"), m_panelFontSize);
    QJsonObject targets;
    targets.insert(QStringLiteral("waybar"), m_themeWaybar);
    targets.insert(QStringLiteral("mako"), m_themeMako);
    targets.insert(QStringLiteral("fuzzel"), m_themeFuzzel);
    targets.insert(QStringLiteral("foot"), m_themeFoot);
    targets.insert(QStringLiteral("panels"), m_themePanels);
    return QJsonObject{
        {QStringLiteral("base"), m_base}, {QStringLiteral("surface"), m_surface},
        {QStringLiteral("secondarySurface"), m_secondarySurface}, {QStringLiteral("text"), m_text},
        {QStringLiteral("mutedText"), m_mutedText}, {QStringLiteral("accent"), m_accent},
        {QStringLiteral("secondaryAccent"), m_secondaryAccent}, {QStringLiteral("highlight"), m_highlight},
        {QStringLiteral("urgent"), m_urgent}, {QStringLiteral("success"), m_success},
        {QStringLiteral("borderUnfocused"), m_borderUnfocused}, {QStringLiteral("fonts"), fonts},
        {QStringLiteral("backgroundMode"), m_backgroundMode}, {QStringLiteral("wallpaperPath"), m_wallpaperPath},
        {QStringLiteral("interfaceRadius"), m_interfaceRadius}, {QStringLiteral("panelSpacing"), m_panelSpacing},
        {QStringLiteral("themeTargets"), targets}};
}

bool SettingsController::loadAppearanceJson(const QJsonObject &appearance, QString *error)
{
    auto readColor = [&appearance, error](const QString &key, const QString &fallback) {
        const QColor value(appearance.value(key).toString(fallback));
        if (!value.isValid() && error && error->isEmpty())
            *error = QStringLiteral("Theme contains an invalid %1 colour.").arg(key);
        return value.isValid() ? value.name(QColor::HexRgb).toUpper() : fallback;
    };
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
    const QJsonObject fonts = appearance.value(QStringLiteral("fonts")).toObject();
    m_waybarFontFamily = fonts.value(QStringLiteral("waybarFamily")).toString(m_waybarFontFamily);
    m_makoFontFamily = fonts.value(QStringLiteral("makoFamily")).toString(m_makoFontFamily);
    m_makoFontSize = qBound(7, fonts.value(QStringLiteral("makoSize")).toInt(m_makoFontSize), 28);
    m_fuzzelFontFamily = fonts.value(QStringLiteral("fuzzelFamily")).toString(m_fuzzelFontFamily);
    m_fuzzelFontSize = qBound(7, fonts.value(QStringLiteral("fuzzelSize")).toInt(m_fuzzelFontSize), 28);
    m_footFontFamily = fonts.value(QStringLiteral("footFamily")).toString(m_footFontFamily);
    m_footFontSize = qBound(6.0, fonts.value(QStringLiteral("footSize")).toDouble(m_footFontSize), 32.0);
    m_panelFontFamily = fonts.value(QStringLiteral("panelFamily")).toString(m_panelFontFamily);
    m_panelFontSize = qBound(7, fonts.value(QStringLiteral("panelSize")).toInt(m_panelFontSize), 28);
    m_backgroundMode = appearance.value(QStringLiteral("backgroundMode")).toString(m_backgroundMode);
    m_wallpaperPath = appearance.value(QStringLiteral("wallpaperPath")).toString(m_wallpaperPath);
    m_interfaceRadius = qBound(0, appearance.value(QStringLiteral("interfaceRadius")).toInt(m_interfaceRadius), 30);
    m_panelSpacing = qBound(0, appearance.value(QStringLiteral("panelSpacing")).toInt(m_panelSpacing), 30);
    const QJsonObject targets = appearance.value(QStringLiteral("themeTargets")).toObject();
    m_themeWaybar = targets.value(QStringLiteral("waybar")).toBool(m_themeWaybar);
    m_themeMako = targets.value(QStringLiteral("mako")).toBool(m_themeMako);
    m_themeFuzzel = targets.value(QStringLiteral("fuzzel")).toBool(m_themeFuzzel);
    m_themeFoot = targets.value(QStringLiteral("foot")).toBool(m_themeFoot);
    m_themePanels = targets.value(QStringLiteral("panels")).toBool(m_themePanels);
    return !error || error->isEmpty();
}

QString SettingsController::validThemeName(const QString &name) const
{
    QString result = name.trimmed();
    result.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9 _.-]")), QStringLiteral("_"));
    while (result.startsWith(QLatin1Char('.'))) result.remove(0, 1);
    return result.left(80).trimmed();
}

QString SettingsController::themeFilePath(const QString &name) const
{
    return QDir(QDir(QFileInfo(m_settingsPath).absolutePath()).filePath(QStringLiteral("themes")))
        .filePath(validThemeName(name) + QStringLiteral(".json"));
}

QStringList SettingsController::customThemes() const
{
    QDir directory(QDir(QFileInfo(m_settingsPath).absolutePath()).filePath(QStringLiteral("themes")));
    QStringList result;
    for (const QString &file : directory.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name))
        result.append(QFileInfo(file).completeBaseName());
    return result;
}

QStringList SettingsController::contrastWarnings() const
{
    QStringList warnings;
    auto check = [&warnings](const QString &label, const QString &foreground,
                             const QString &background, double minimum) {
        const double ratio = contrastRatio(QColor(foreground), QColor(background));
        if (ratio < minimum)
            warnings.append(QStringLiteral("%1 contrast is %2:1; aim for at least %3:1.")
                                .arg(label).arg(ratio, 0, 'f', 2).arg(minimum, 0, 'f', 1));
    };
    check(QStringLiteral("Primary text on the surface"), m_text, m_surface, 4.5);
    check(QStringLiteral("Muted text on the surface"), m_mutedText, m_surface, 4.5);
    check(QStringLiteral("Focused-tag text on the accent"), m_base, m_accent, 4.5);
    check(QStringLiteral("Urgent text"), m_text, m_urgent, 3.0);
    return warnings;
}

bool SettingsController::saveCustomTheme(const QString &name)
{
    const QString safeName = validThemeName(name);
    if (safeName.isEmpty()) { setStatus(QStringLiteral("Enter a theme name."), true); return false; }
    QString error;
    const QJsonObject document{{QStringLiteral("schemaVersion"), 1}, {QStringLiteral("name"), safeName},
                               {QStringLiteral("appearance"), appearanceJson()}};
    if (!writeFileAtomically(themeFilePath(safeName), QJsonDocument(document).toJson(QJsonDocument::Indented), &error)) {
        setStatus(error, true); return false;
    }
    emit themeListChanged(); setStatus(QStringLiteral("Saved theme “%1”.").arg(safeName)); return true;
}

bool SettingsController::applyCustomTheme(const QString &name)
{
    QFile file(themeFilePath(name));
    if (!file.open(QIODevice::ReadOnly)) { setStatus(QStringLiteral("Could not open theme “%1”.").arg(name), true); return false; }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    QString error;
    if (!document.isObject() || parseError.error != QJsonParseError::NoError
        || !loadAppearanceJson(document.object().value(QStringLiteral("appearance")).toObject(), &error)) {
        setStatus(error.isEmpty() ? QStringLiteral("Theme file is invalid.") : error, true); return false;
    }
    markChanged(); setStatus(QStringLiteral("Loaded theme “%1”. Apply to use it.").arg(name)); return true;
}

bool SettingsController::duplicateTheme(const QString &name, const QString &newName)
{
    const QString safeName = validThemeName(newName);
    if (safeName.isEmpty() || !QFileInfo::exists(themeFilePath(name))) { setStatus(QStringLiteral("Choose a theme and enter a new name."), true); return false; }
    QFile source(themeFilePath(name));
    if (!source.open(QIODevice::ReadOnly)) { setStatus(source.errorString(), true); return false; }
    QJsonDocument document = QJsonDocument::fromJson(source.readAll());
    QJsonObject root = document.object(); root.insert(QStringLiteral("name"), safeName);
    QString error;
    if (!writeFileAtomically(themeFilePath(safeName), QJsonDocument(root).toJson(QJsonDocument::Indented), &error)) {
        setStatus(error, true); return false;
    }
    emit themeListChanged(); setStatus(QStringLiteral("Duplicated as “%1”.").arg(safeName)); return true;
}

bool SettingsController::renameTheme(const QString &name, const QString &newName)
{
    const QString safeName = validThemeName(newName);
    if (safeName.isEmpty() || name == safeName) { setStatus(QStringLiteral("Enter a different theme name."), true); return false; }
    const QString sourcePath = themeFilePath(name);
    const QString targetPath = themeFilePath(safeName);
    QFile source(sourcePath);
    if (QFileInfo::exists(targetPath) || !source.open(QIODevice::ReadOnly)) {
        setStatus(QStringLiteral("Could not rename the theme; the target may already exist."), true); return false;
    }
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(source.readAll(), &parseError);
    if (!document.isObject() || parseError.error != QJsonParseError::NoError) {
        setStatus(QStringLiteral("The saved theme is invalid."), true); return false;
    }
    QJsonObject root = document.object();
    root.insert(QStringLiteral("name"), safeName);
    QString error;
    if (!writeFileAtomically(targetPath, QJsonDocument(root).toJson(QJsonDocument::Indented), &error)) {
        setStatus(error, true); return false;
    }
    source.close();
    if (!QFile::remove(sourcePath)) {
        QFile::remove(targetPath);
        setStatus(QStringLiteral("Could not finish renaming the theme."), true); return false;
    }
    emit themeListChanged(); setStatus(QStringLiteral("Renamed theme to “%1”.").arg(safeName)); return true;
}

bool SettingsController::importTheme(const QString &urlOrPath)
{
    const QUrl url(urlOrPath);
    const QString path = url.isLocalFile() ? url.toLocalFile() : urlOrPath;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { setStatus(QStringLiteral("Could not open the selected theme."), true); return false; }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    const QJsonObject root = document.object();
    QString name = validThemeName(root.value(QStringLiteral("name")).toString(QFileInfo(path).completeBaseName()));
    QString error;
    QJsonObject appearance = root.value(QStringLiteral("appearance")).toObject();
    if (!document.isObject() || appearance.isEmpty() || name.isEmpty()) {
        setStatus(QStringLiteral("The selected file is not a River Control Center theme."), true); return false;
    }
    const QJsonObject normalized{{QStringLiteral("schemaVersion"), 1}, {QStringLiteral("name"), name},
                                 {QStringLiteral("appearance"), appearance}};
    if (!writeFileAtomically(themeFilePath(name), QJsonDocument(normalized).toJson(QJsonDocument::Indented), &error)) {
        setStatus(error, true); return false;
    }
    emit themeListChanged(); setStatus(QStringLiteral("Imported theme “%1”.").arg(name)); return true;
}

bool SettingsController::exportTheme(const QString &name)
{
    QFile source(themeFilePath(name));
    if (!source.open(QIODevice::ReadOnly)) { setStatus(QStringLiteral("Choose a saved theme to export."), true); return false; }
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) documents = QDir::homePath();
    QString error;
    const QString path = QDir(documents).filePath(validThemeName(name) + QStringLiteral(".river-theme.json"));
    if (!writeFileAtomically(path, source.readAll(), &error)) { setStatus(error, true); return false; }
    setStatus(QStringLiteral("Exported theme to %1").arg(path)); return true;
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
    }
    m_loading = false;
    markChanged();
    setStatus(QStringLiteral("%1 preset loaded. Apply to save it.").arg(name));
}

QString SettingsController::queryLidClosePolicy() const
{
    QProcess process;
    process.start(QStringLiteral("loginctl"),
                  {QStringLiteral("show-logind"),
                   QStringLiteral("--property=HandleLidSwitch"),
                   QStringLiteral("--property=HandleLidSwitchExternalPower"),
                   QStringLiteral("--property=HandleLidSwitchDocked")});
    if (!process.waitForStarted(1000) || !process.waitForFinished(1500)
        || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return QStringLiteral("Unavailable — could not query systemd-logind");
    }

    QString normal = QStringLiteral("not reported");
    QString external = QStringLiteral("not reported");
    QString docked = QStringLiteral("not reported");
    const QStringList lines = QString::fromUtf8(process.readAllStandardOutput()).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QString value = line.section(QLatin1Char('='), 1);
        if (line.startsWith(QStringLiteral("HandleLidSwitch=")))
            normal = value;
        else if (line.startsWith(QStringLiteral("HandleLidSwitchExternalPower=")))
            external = value;
        else if (line.startsWith(QStringLiteral("HandleLidSwitchDocked=")))
            docked = value;
    }
    return QStringLiteral("Default: %1 · External power: %2 · Docked: %3")
        .arg(normal, external, docked);
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
    const QStringList fontFamilies = {m_waybarFontFamily, m_makoFontFamily, m_fuzzelFontFamily,
                                      m_footFontFamily, m_panelFontFamily};
    for (const QString &font : fontFamilies) {
        if (font.trimmed().isEmpty() || font.contains(QLatin1Char('\n')))
            return QStringLiteral("Font family names cannot be empty or contain line breaks.");
    }
    if (m_backgroundMode != QLatin1String("solid") && m_backgroundMode != QLatin1String("wallpaper"))
        return QStringLiteral("Desktop background mode is not supported.");
    if (m_backgroundMode == QLatin1String("wallpaper")
        && (m_wallpaperPath.isEmpty() || !QFileInfo::exists(m_wallpaperPath)))
        return QStringLiteral("Choose an existing wallpaper image.");
    if (m_interfaceRadius < 0 || m_interfaceRadius > 30 || m_panelSpacing < 0 || m_panelSpacing > 30)
        return QStringLiteral("Radius and panel spacing must be between 0 and 30 pixels.");
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
    const QRegularExpression timePattern(QStringLiteral("^(?:[01][0-9]|2[0-3]):[0-5][0-9]$"));
    if (!timePattern.match(m_nightLightStart).hasMatch()
        || !timePattern.match(m_nightLightStop).hasMatch())
        return QStringLiteral("Night-light times must use 24-hour HH:MM format.");
    if (m_nightLightStart == m_nightLightStop)
        return QStringLiteral("Night-light start and stop times must be different.");
    if (m_defaultPowerProfile != QLatin1String("power-saver")
        && m_defaultPowerProfile != QLatin1String("balanced")
        && m_defaultPowerProfile != QLatin1String("performance"))
        return QStringLiteral("Default power profile is not supported.");
    if (m_touchpadPointerAccel < -1.0 || m_touchpadPointerAccel > 1.0)
        return QStringLiteral("Touchpad acceleration must be between -1.0 and 1.0.");
    if (m_touchpadClickMethod != QLatin1String("none")
        && m_touchpadClickMethod != QLatin1String("button-areas")
        && m_touchpadClickMethod != QLatin1String("clickfinger"))
        return QStringLiteral("Touchpad click method is not supported.");
    if (m_keyboardRepeatRate < 1 || m_keyboardRepeatRate > 100
        || m_keyboardRepeatDelay < 100 || m_keyboardRepeatDelay > 2000)
        return QStringLiteral("Keyboard repeat settings are outside the supported range.");
    const QStringList composeOptions = {
        QString(), QStringLiteral("compose:rctrl"), QStringLiteral("compose:lctrl"),
        QStringLiteral("compose:ralt"), QStringLiteral("compose:lalt"),
        QStringLiteral("compose:menu"), QStringLiteral("compose:caps")};
    if (!composeOptions.contains(m_composeKey))
        return QStringLiteral("Compose key is not supported.");
    if (m_gestureFingerCount < 3 || m_gestureFingerCount > 5)
        return QStringLiteral("Gesture finger count must be between 3 and 5.");
    if (m_gestureDirection != QLatin1String("left-next")
        && m_gestureDirection != QLatin1String("right-next"))
        return QStringLiteral("Gesture direction is not supported.");
    if (m_gestureDeviceName.isEmpty() || m_gestureDeviceName.contains(QLatin1Char('\n'))
        || m_gestureDeviceName.contains(QLatin1Char('"')))
        return QStringLiteral("Fusuma device name is empty or contains unsupported characters.");
    if (m_mainWindowRatio < 0.1 || m_mainWindowRatio > 0.9)
        return QStringLiteral("Main-window ratio must be between 0.10 and 0.90.");
    if (m_mainWindowCount < 1 || m_mainWindowCount > 16)
        return QStringLiteral("Main-window count must be between 1 and 16.");
    if (!QStringList{QStringLiteral("left"), QStringLiteral("right"),
                     QStringLiteral("top"), QStringLiteral("bottom")}.contains(m_mainPosition))
        return QStringLiteral("Main-window position is not supported.");
    if (!QStringList{QStringLiteral("top"), QStringLiteral("bottom"),
                     QStringLiteral("above"), QStringLiteral("below")}.contains(m_defaultAttachMode))
        return QStringLiteral("Default attach mode is not supported.");
    if (!QStringList{QStringLiteral("mode-aware"), QStringLiteral("disabled"),
                     QStringLiteral("normal"), QStringLiteral("always")}.contains(m_focusPolicy))
        return QStringLiteral("Focus-follows-pointer policy is not supported.");
    if (m_defaultLayoutMode != QLatin1String("tiled")
        && m_defaultLayoutMode != QLatin1String("floating"))
        return QStringLiteral("Default layout mode is not supported.");
    if (m_tagCount < 1 || m_tagCount > 9)
        return QStringLiteral("Tag count must be between 1 and 9.");
    const QStringList ruleActions = {QStringLiteral("float"), QStringLiteral("no-float"),
                                     QStringLiteral("ssd"), QStringLiteral("csd")};
    for (qsizetype index = 0; index < m_windowRules.size(); ++index) {
        const QVariantMap rule = m_windowRules.at(index).toMap();
        const QString appId = rule.value(QStringLiteral("appId")).toString();
        const QString title = rule.value(QStringLiteral("titleContains")).toString();
        const QString action = rule.value(QStringLiteral("action")).toString();
        if (appId.isEmpty() && title.isEmpty())
            return QStringLiteral("Window rule %1 needs an application ID or title.").arg(index + 1);
        if (appId.contains(QLatin1Char('\n')) || title.contains(QLatin1Char('\n')))
            return QStringLiteral("Window rule %1 contains a line break.").arg(index + 1);
        if (title.contains(QLatin1Char('*')))
            return QStringLiteral("Window rule %1 title should be plain text, without *.").arg(index + 1);
        if (!ruleActions.contains(action))
            return QStringLiteral("Window rule %1 has an unsupported action.").arg(index + 1);
    }
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
    const QJsonObject power = root.value(QStringLiteral("powerAndIdle")).toObject();
    const QJsonObject idleActions = power.value(QStringLiteral("actions")).toObject();
    const QJsonObject input = root.value(QStringLiteral("input")).toObject();
    const QJsonObject touchpad = input.value(QStringLiteral("touchpad")).toObject();
    const QJsonObject keyboard = input.value(QStringLiteral("keyboard")).toObject();
    const QJsonObject gestures = input.value(QStringLiteral("gestures")).toObject();
    const QJsonObject windows = root.value(QStringLiteral("windowManagement")).toObject();
    const QJsonObject startup = root.value(QStringLiteral("sessionStartup")).toObject();
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
    loadAppearanceJson(appearance);
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
    m_lockTimeoutMinutes = qBound(1, power.value(QStringLiteral("lockTimeoutMinutes")).toInt(m_lockTimeoutMinutes), 360);
    m_displayOffTimeoutMinutes = qBound(1, power.value(QStringLiteral("displayOffTimeoutMinutes")).toInt(m_displayOffTimeoutMinutes), 360);
    m_batterySuspendTimeoutMinutes = qBound(1, power.value(QStringLiteral("batterySuspendTimeoutMinutes")).toInt(m_batterySuspendTimeoutMinutes), 360);
    m_acSuspendTimeoutMinutes = qBound(1, power.value(QStringLiteral("acSuspendTimeoutMinutes")).toInt(m_acSuspendTimeoutMinutes), 360);
    m_lockOnIdle = idleActions.value(QStringLiteral("lock")).toBool(m_lockOnIdle);
    m_displayOffOnIdle = idleActions.value(QStringLiteral("displayOff")).toBool(m_displayOffOnIdle);
    m_suspendOnBattery = idleActions.value(QStringLiteral("suspendOnBattery")).toBool(m_suspendOnBattery);
    m_suspendOnAc = idleActions.value(QStringLiteral("suspendOnAc")).toBool(m_suspendOnAc);
    m_lockBeforeSuspend = power.value(QStringLiteral("lockBeforeSuspend")).toBool(m_lockBeforeSuspend);
    m_defaultPowerProfile = power.value(QStringLiteral("defaultPowerProfile")).toString(m_defaultPowerProfile);
    m_nightLightStart = power.value(QStringLiteral("nightLightStart")).toString(m_nightLightStart);
    m_nightLightStop = power.value(QStringLiteral("nightLightStop")).toString(m_nightLightStop);
    m_touchpadTap = touchpad.value(QStringLiteral("tap")).toBool(m_touchpadTap);
    m_touchpadNaturalScroll = touchpad.value(QStringLiteral("naturalScroll")).toBool(m_touchpadNaturalScroll);
    m_touchpadDisableWhileTyping = touchpad.value(QStringLiteral("disableWhileTyping")).toBool(m_touchpadDisableWhileTyping);
    m_touchpadPointerAccel = qBound(-1.0, touchpad.value(QStringLiteral("pointerAcceleration")).toDouble(m_touchpadPointerAccel), 1.0);
    m_touchpadClickMethod = touchpad.value(QStringLiteral("clickMethod")).toString(m_touchpadClickMethod);
    m_keyboardRepeatRate = qBound(1, keyboard.value(QStringLiteral("repeatRate")).toInt(m_keyboardRepeatRate), 100);
    m_keyboardRepeatDelay = qBound(100, keyboard.value(QStringLiteral("repeatDelay")).toInt(m_keyboardRepeatDelay), 2000);
    const QJsonArray layouts = keyboard.value(QStringLiteral("layouts")).toArray();
    if (!layouts.isEmpty())
        m_sinhalaLayoutEnabled = layouts.contains(QStringLiteral("lk"));
    m_composeKey = keyboard.value(QStringLiteral("composeKey")).toString(m_composeKey);
    m_gestureFingerCount = qBound(3, gestures.value(QStringLiteral("fingerCount")).toInt(m_gestureFingerCount), 5);
    m_gestureDirection = gestures.value(QStringLiteral("direction")).toString(m_gestureDirection);
    m_gestureDeviceName = gestures.value(QStringLiteral("deviceName")).toString(m_gestureDeviceName);
    m_mainWindowRatio = qBound(0.1, windows.value(QStringLiteral("mainRatio")).toDouble(m_mainWindowRatio), 0.9);
    m_mainWindowCount = qBound(1, windows.value(QStringLiteral("mainCount")).toInt(m_mainWindowCount), 16);
    m_mainPosition = windows.value(QStringLiteral("mainPosition")).toString(m_mainPosition);
    m_defaultAttachMode = windows.value(QStringLiteral("defaultAttachMode")).toString(m_defaultAttachMode);
    m_focusPolicy = windows.value(QStringLiteral("focusPolicy")).toString(m_focusPolicy);
    m_defaultLayoutMode = windows.value(QStringLiteral("defaultLayout")).toString(m_defaultLayoutMode);
    m_tagCount = qBound(1, windows.value(QStringLiteral("tagCount")).toInt(m_tagCount), 9);
    m_autostartWaybar = startup.value(QStringLiteral("waybar")).toBool(m_autostartWaybar);
    m_autostartMako = startup.value(QStringLiteral("mako")).toBool(m_autostartMako);
    m_autostartKanshi = startup.value(QStringLiteral("kanshi")).toBool(m_autostartKanshi);
    m_autostartFusuma = startup.value(QStringLiteral("fusuma")).toBool(m_autostartFusuma);
    m_autostartSwayidle = startup.value(QStringLiteral("swayidle")).toBool(m_autostartSwayidle);
    const QJsonArray rules = windows.value(QStringLiteral("rules")).toArray();
    if (windows.contains(QStringLiteral("rules"))) {
        m_windowRules.clear();
        for (const QJsonValue &value : rules) {
            const QJsonObject rule = value.toObject();
            m_windowRules.append(makeRule(rule.value(QStringLiteral("appId")).toString(),
                                          rule.value(QStringLiteral("titleContains")).toString(),
                                          rule.value(QStringLiteral("action")).toString(QStringLiteral("float"))));
        }
    }
    m_savedWindowRules = m_windowRules;
    m_windowDirty = false;
    m_loading = false;
    ++m_revision;
    setDirty(false);
    emit settingsChanged();
    emit windowRulesChanged();
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

    const QJsonObject appearance = appearanceJson();

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

    QJsonObject idleActions;
    idleActions.insert(QStringLiteral("lock"), m_lockOnIdle);
    idleActions.insert(QStringLiteral("displayOff"), m_displayOffOnIdle);
    idleActions.insert(QStringLiteral("suspendOnBattery"), m_suspendOnBattery);
    idleActions.insert(QStringLiteral("suspendOnAc"), m_suspendOnAc);

    QJsonObject power;
    power.insert(QStringLiteral("lockTimeoutMinutes"), m_lockTimeoutMinutes);
    power.insert(QStringLiteral("displayOffTimeoutMinutes"), m_displayOffTimeoutMinutes);
    power.insert(QStringLiteral("batterySuspendTimeoutMinutes"), m_batterySuspendTimeoutMinutes);
    power.insert(QStringLiteral("acSuspendTimeoutMinutes"), m_acSuspendTimeoutMinutes);
    power.insert(QStringLiteral("actions"), idleActions);
    power.insert(QStringLiteral("lockBeforeSuspend"), m_lockBeforeSuspend);
    power.insert(QStringLiteral("defaultPowerProfile"), m_defaultPowerProfile);
    power.insert(QStringLiteral("nightLightStart"), m_nightLightStart);
    power.insert(QStringLiteral("nightLightStop"), m_nightLightStop);

    QJsonObject touchpad;
    touchpad.insert(QStringLiteral("tap"), m_touchpadTap);
    touchpad.insert(QStringLiteral("naturalScroll"), m_touchpadNaturalScroll);
    touchpad.insert(QStringLiteral("disableWhileTyping"), m_touchpadDisableWhileTyping);
    touchpad.insert(QStringLiteral("pointerAcceleration"), m_touchpadPointerAccel);
    touchpad.insert(QStringLiteral("clickMethod"), m_touchpadClickMethod);

    QJsonArray layouts;
    layouts.append(QStringLiteral("us"));
    if (m_sinhalaLayoutEnabled)
        layouts.append(QStringLiteral("lk"));
    QJsonObject keyboard;
    keyboard.insert(QStringLiteral("repeatRate"), m_keyboardRepeatRate);
    keyboard.insert(QStringLiteral("repeatDelay"), m_keyboardRepeatDelay);
    keyboard.insert(QStringLiteral("layouts"), layouts);
    keyboard.insert(QStringLiteral("composeKey"), m_composeKey);

    QJsonObject gestures;
    gestures.insert(QStringLiteral("fingerCount"), m_gestureFingerCount);
    gestures.insert(QStringLiteral("direction"), m_gestureDirection);
    gestures.insert(QStringLiteral("deviceName"), m_gestureDeviceName);

    QJsonObject input;
    input.insert(QStringLiteral("touchpad"), touchpad);
    input.insert(QStringLiteral("keyboard"), keyboard);
    input.insert(QStringLiteral("gestures"), gestures);

    QJsonArray rules;
    for (const QVariant &entry : m_windowRules) {
        const QVariantMap rule = entry.toMap();
        rules.append(QJsonObject{{QStringLiteral("appId"), rule.value(QStringLiteral("appId")).toString()},
                                 {QStringLiteral("titleContains"), rule.value(QStringLiteral("titleContains")).toString()},
                                 {QStringLiteral("action"), rule.value(QStringLiteral("action")).toString()}});
    }
    QJsonObject windows;
    windows.insert(QStringLiteral("mainRatio"), m_mainWindowRatio);
    windows.insert(QStringLiteral("mainCount"), m_mainWindowCount);
    windows.insert(QStringLiteral("mainPosition"), m_mainPosition);
    windows.insert(QStringLiteral("defaultAttachMode"), m_defaultAttachMode);
    windows.insert(QStringLiteral("focusPolicy"), m_focusPolicy);
    windows.insert(QStringLiteral("defaultLayout"), m_defaultLayoutMode);
    windows.insert(QStringLiteral("tagCount"), m_tagCount);
    windows.insert(QStringLiteral("rules"), rules);

    QJsonObject startup;
    startup.insert(QStringLiteral("waybar"), m_autostartWaybar);
    startup.insert(QStringLiteral("mako"), m_autostartMako);
    startup.insert(QStringLiteral("kanshi"), m_autostartKanshi);
    startup.insert(QStringLiteral("fusuma"), m_autostartFusuma);
    startup.insert(QStringLiteral("swayidle"), m_autostartSwayidle);

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 7);
    root.insert(QStringLiteral("appearance"), appearance);
    root.insert(QStringLiteral("river"), river);
    root.insert(QStringLiteral("bar"), bar);
    root.insert(QStringLiteral("panels"), panels);
    root.insert(QStringLiteral("powerAndIdle"), power);
    root.insert(QStringLiteral("input"), input);
    root.insert(QStringLiteral("windowManagement"), windows);
    root.insert(QStringLiteral("sessionStartup"), startup);
    return writeFileAtomically(m_settingsPath, QJsonDocument(root).toJson(QJsonDocument::Indented), error);
}

bool SettingsController::generateFiles(QString *error)
{
    QString river = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_BASE='%1'\nRCC_SURFACE='%2'\nRCC_SECONDARY_SURFACE='%3'\n"
        "RCC_TEXT='%4'\nRCC_MUTED_TEXT='%5'\nRCC_ACCENT='%6'\n"
        "RCC_SECONDARY_ACCENT='%7'\nRCC_HIGHLIGHT='%8'\nRCC_URGENT='%9'\n"
        "RCC_SUCCESS='%10'\nRCC_BORDER_UNFOCUSED='%11'\nRCC_BORDER_WIDTH=%12\n"
        "RCC_VIEW_PADDING=%13\nRCC_OUTER_PADDING=%14\n")
        .arg(m_base, m_surface, m_secondarySurface, m_text, m_mutedText, m_accent,
             m_secondaryAccent, m_highlight, m_urgent, m_success, m_borderUnfocused)
        .arg(m_borderWidth).arg(m_viewPadding).arg(m_outerPadding);
    river += QStringLiteral("RCC_BACKGROUND_MODE=%1\nRCC_WALLPAPER=%2\n")
        .arg(shellQuote(m_backgroundMode), shellQuote(m_wallpaperPath));

    const QString css = QStringLiteral(
        "/* Generated by River Control Center. */\n"
        "@define-color rcc_base %1;\n@define-color rcc_surface %2;\n"
        "@define-color rcc_secondary_surface %3;\n@define-color rcc_text %4;\n"
        "@define-color rcc_muted_text %5;\n@define-color rcc_accent %6;\n"
        "@define-color rcc_secondary_accent %7;\n@define-color rcc_highlight %8;\n"
        "@define-color rcc_urgent %9;\n@define-color rcc_success %10;\n"
        "@define-color rcc_border_unfocused %11;\n@define-color rcc_subtle_text mix(%5, %2, 0.68);\n"
        "@define-color rcc_occupied mix(%4, %5, 0.36);\n"
        "* { font-family: \"%12\", sans-serif; font-size: %13px; }\n"
        "#tags, #tags button, #clock, #essentials, #tray, #custom-system-menu { border-radius: %14px; }\n")
        .arg(m_base, m_surface, m_secondarySurface, m_text, m_mutedText, m_accent,
             m_secondaryAccent, m_highlight, m_urgent, m_success, m_borderUnfocused,
             m_waybarFontFamily)
        .arg(m_barFontSize).arg(m_interfaceRadius);

    const QString foot = QStringLiteral(
        "# Generated by River Control Center.\n[main]\nfont=%18:size=%19, Noto Color Emoji:size=%19\n\n[colors-dark]\n"
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
             lighter(QString::fromLatin1(kPurple)), lighter(m_highlight), colorWithoutHash(m_secondarySurface),
             m_footFontFamily, QString::number(m_footFontSize, 'f', 1));

    auto rgba = [this](const QString &value) { return colorWithoutHash(value) + QStringLiteral("ff"); };
    const QString fuzzel = QStringLiteral(
        "# Generated by River Control Center.\n[main]\nfont=%6:size=%7\n\n[colors]\n"
        "background=%1\ntext=%2\nmessage=%3\nprompt=%4\nplaceholder=%3\n"
        "input=%2\nmatch=%4\nselection=%5\nselection-text=%2\n"
        "selection-match=%4\ncounter=%3\nborder=%4\n\n[border]\nradius=%8\n")
        .arg(rgba(m_surface), rgba(m_text), rgba(m_mutedText), rgba(m_accent), rgba(m_secondarySurface),
             m_fuzzelFontFamily).arg(m_fuzzelFontSize).arg(m_interfaceRadius);

    const QString mako = QStringLiteral(
        "# Generated by River Control Center.\nbackground-color=%1\ntext-color=%2\n"
        "border-color=%3\nmax-history=%4\nfont=%5 %6\nborder-radius=%7\n")
        .arg(m_surface, m_text, m_accent).arg(m_notificationHistoryLimit)
        .arg(m_makoFontFamily).arg(m_makoFontSize).arg(m_interfaceRadius);

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
    waybarSettings.insert(QStringLiteral("river/tags"),
                          QJsonObject{{QStringLiteral("num-tags"), m_tagCount},
                                      {QStringLiteral("smooth-scrolling-threshold"), 1.0},
                                      {QStringLiteral("on-scroll-up"),
                                       QStringLiteral("~/.config/river/bin/river-cycle-tags previous %1").arg(m_tagCount)},
                                      {QStringLiteral("on-scroll-down"),
                                       QStringLiteral("~/.config/river/bin/river-cycle-tags next %1").arg(m_tagCount)}});
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
        "#tags button { padding: 0 %1px; }\n"
        "#window, #clock, #custom-clipboard, #custom-keyboard-layout,\n"
        "#custom-window-controls, #pulseaudio, #battery { padding: 0 %3px; }\n"
        "#tray { padding: 0 %4px; }\n"
        "#custom-system-menu { padding: 0 %3px; }\n")
        .arg(m_tagPadding).arg(m_modulePadding).arg(qMax(0, m_modulePadding - 1));

    QJsonObject panelSettings;
    panelSettings.insert(QStringLiteral("clipboardHistoryLimit"), m_clipboardHistoryLimit);
    panelSettings.insert(QStringLiteral("notificationHistoryLimit"), m_notificationHistoryLimit);
    panelSettings.insert(QStringLiteral("width"), m_panelWidth);
    panelSettings.insert(QStringLiteral("height"), m_panelHeight);
    panelSettings.insert(QStringLiteral("padding"), m_panelPadding);
    panelSettings.insert(QStringLiteral("topMargin"), m_panelTopMargin);
    panelSettings.insert(QStringLiteral("rightMargin"), m_panelRightMargin);
    panelSettings.insert(QStringLiteral("spacing"), m_panelSpacing);
    panelSettings.insert(QStringLiteral("radius"), m_interfaceRadius);
    const QByteArray panelJson = QJsonDocument(panelSettings).toJson(QJsonDocument::Indented);

    const QString panelTheme = QStringLiteral(
        "/* Generated by River Control Center. */\n"
        "window { font-family: \"%1\", sans-serif; font-size: %2pt; }\n"
        "#quick-settings-card, #desktop-panel-card, #brightness-panel-card, #window-controls-card {\n"
        "  background-color: %3; color: %4; border-color: %5; border-radius: %6px;\n"
        "}\n"
        ".settings-section, .notification-card { background-color: %7; border-color: %8; border-radius: %6px; }\n"
        "label.title, label.section { color: %5; }\nlabel.muted { color: %9; }\n"
        "button { color: %4; background-color: %7; border-color: %8; border-radius: %10px; }\n"
        "button:hover, button:active, button.active { color: %11; background-color: %5; border-color: %5; }\n"
        "button.alert, button.destructive { color: %4; background-color: %12; border-color: %12; }\n"
        "scale trough { background-color: %3; }\nscale highlight, switch:checked { background-color: %5; }\n"
        "scale slider { background-color: %4; }\n")
        .arg(m_panelFontFamily).arg(m_panelFontSize).arg(m_surface, m_text, m_accent)
        .arg(m_interfaceRadius).arg(m_secondarySurface, m_borderUnfocused, m_mutedText)
        .arg(qMax(0, m_interfaceRadius - 3)).arg(m_base, m_urgent);

    auto shellBool = [](bool value) {
        return value ? QStringLiteral("true") : QStringLiteral("false");
    };
    const QString sessionStartup = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_AUTOSTART_WAYBAR=%1\n"
        "RCC_AUTOSTART_MAKO=%2\n"
        "RCC_AUTOSTART_KANSHI=%3\n"
        "RCC_AUTOSTART_FUSUMA=%4\n"
        "RCC_AUTOSTART_SWAYIDLE=%5\n")
        .arg(shellBool(m_autostartWaybar), shellBool(m_autostartMako),
             shellBool(m_autostartKanshi), shellBool(m_autostartFusuma),
             shellBool(m_autostartSwayidle));
    const QString powerIdle = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_LOCK_TIMEOUT=%1\n"
        "RCC_DISPLAY_OFF_TIMEOUT=%2\n"
        "RCC_BATTERY_SUSPEND_TIMEOUT=%3\n"
        "RCC_AC_SUSPEND_TIMEOUT=%4\n"
        "RCC_LOCK_ON_IDLE=%5\n"
        "RCC_DISPLAY_OFF_ON_IDLE=%6\n"
        "RCC_SUSPEND_ON_BATTERY=%7\n"
        "RCC_SUSPEND_ON_AC=%8\n"
        "RCC_LOCK_BEFORE_SUSPEND=%9\n"
        "RCC_DEFAULT_POWER_PROFILE='%10'\n"
        "RCC_NIGHT_LIGHT_START='%11'\n"
        "RCC_NIGHT_LIGHT_STOP='%12'\n")
        .arg(m_lockTimeoutMinutes * 60)
        .arg(m_displayOffTimeoutMinutes * 60)
        .arg(m_batterySuspendTimeoutMinutes * 60)
        .arg(m_acSuspendTimeoutMinutes * 60)
        .arg(shellBool(m_lockOnIdle))
        .arg(shellBool(m_displayOffOnIdle))
        .arg(shellBool(m_suspendOnBattery))
        .arg(shellBool(m_suspendOnAc))
        .arg(shellBool(m_lockBeforeSuspend))
        .arg(m_defaultPowerProfile)
        .arg(m_nightLightStart)
        .arg(m_nightLightStop);

    const QString inputSettings = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_TOUCHPAD_TAP=%1\n"
        "RCC_TOUCHPAD_NATURAL_SCROLL=%2\n"
        "RCC_TOUCHPAD_DISABLE_WHILE_TYPING=%3\n"
        "RCC_TOUCHPAD_POINTER_ACCEL=%4\n"
        "RCC_TOUCHPAD_CLICK_METHOD='%5'\n"
        "RCC_KEYBOARD_REPEAT_RATE=%6\n"
        "RCC_KEYBOARD_REPEAT_DELAY=%7\n"
        "RCC_KEYBOARD_LAYOUTS='%8'\n"
        "RCC_COMPOSE_KEY='%9'\n")
        .arg(shellBool(m_touchpadTap))
        .arg(shellBool(m_touchpadNaturalScroll))
        .arg(shellBool(m_touchpadDisableWhileTyping))
        .arg(QString::number(m_touchpadPointerAccel, 'f', 2))
        .arg(m_touchpadClickMethod)
        .arg(m_keyboardRepeatRate)
        .arg(m_keyboardRepeatDelay)
        .arg(m_sinhalaLayoutEnabled ? QStringLiteral("us lk") : QStringLiteral("us"))
        .arg(m_composeKey);

    const QString nextOnLeft = m_gestureDirection == QLatin1String("left-next")
        ? QStringLiteral("next") : QStringLiteral("previous");
    const QString nextOnRight = m_gestureDirection == QLatin1String("left-next")
        ? QStringLiteral("previous") : QStringLiteral("next");
    const QString cycleTags = QDir(QDir::homePath()).filePath(
        QStringLiteral(".config/river/bin/river-cycle-tags"));
    const QString fusuma = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "swipe:\n"
        "  %1:\n"
        "    left:\n"
        "      command: \"%2 %3 %6\"\n"
        "    right:\n"
        "      command: \"%2 %4 %6\"\n\n"
        "threshold:\n"
        "  swipe: 0.5\n\n"
        "interval:\n"
        "  swipe: 0.6\n\n"
        "plugin:\n"
        "  filters:\n"
        "    libinput_device_filter:\n"
        "      keep_device_names:\n"
        "        - \"%5\"\n")
        .arg(m_gestureFingerCount)
        .arg(cycleTags, nextOnLeft, nextOnRight, m_gestureDeviceName)
        .arg(m_tagCount);

    const QString windowSettings = QStringLiteral(
        "# Generated by River Control Center. Do not edit this file directly.\n"
        "RCC_MAIN_RATIO=%1\n"
        "RCC_MAIN_COUNT=%2\n"
        "RCC_MAIN_POSITION=%3\n"
        "RCC_ATTACH_MODE=%4\n"
        "RCC_FOCUS_POLICY=%5\n"
        "RCC_DEFAULT_LAYOUT_MODE=%6\n"
        "RCC_TAG_COUNT=%7\n")
        .arg(QString::number(m_mainWindowRatio, 'f', 2))
        .arg(m_mainWindowCount)
        .arg(shellQuote(m_mainPosition), shellQuote(m_defaultAttachMode),
             shellQuote(m_focusPolicy), shellQuote(m_defaultLayoutMode))
        .arg(m_tagCount);

    QString windowRules = QStringLiteral(
        "#!/bin/sh\n# Generated by River Control Center. Do not edit this file directly.\n");
    for (const QVariant &entry : m_windowRules) {
        const QVariantMap rule = entry.toMap();
        const QString appId = rule.value(QStringLiteral("appId")).toString();
        const QString title = rule.value(QStringLiteral("titleContains")).toString();
        const QString action = rule.value(QStringLiteral("action")).toString();
        windowRules += QStringLiteral("riverctl rule-add");
        if (!appId.isEmpty())
            windowRules += QStringLiteral(" -app-id ") + shellQuote(appId);
        if (!title.isEmpty())
            windowRules += QStringLiteral(" -title ") + shellQuote(QStringLiteral("*") + title + QStringLiteral("*"));
        windowRules += QLatin1Char(' ') + action + QLatin1Char('\n');
    }

    auto writeManaged = [this, error](bool enabled, const QString &name, const QByteArray &contents) {
        return !enabled || writeFileAtomically(QDir(m_generatedPath).filePath(name), contents, error);
    };
    return writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("river-theme.sh")), river.toUtf8(), error)
        && writeManaged(m_themeWaybar, QStringLiteral("waybar-colors.css"), css.toUtf8())
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("waybar-layout.css")), waybarLayout.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("waybar-settings.json")), waybar, error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("panel-settings.json")), panelJson, error)
        && writeManaged(m_themePanels, QStringLiteral("panel-theme.css"), panelTheme.toUtf8())
        && writeManaged(m_themeFoot, QStringLiteral("foot-theme.ini"), foot.toUtf8())
        && writeManaged(m_themeFuzzel, QStringLiteral("fuzzel-theme.ini"), fuzzel.toUtf8())
        && writeManaged(m_themeMako, QStringLiteral("mako-theme.conf"), mako.toUtf8())
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("session-startup.conf")), sessionStartup.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("power-idle.conf")), powerIdle.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("input.conf")), inputSettings.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("window-management.conf")), windowSettings.toUtf8(), error)
        && writeFileAtomically(QDir(m_generatedPath).filePath(QStringLiteral("window-rules.sh")), windowRules.toUtf8(), error)
        && writeFileAtomically(QDir(m_configRoot).filePath(QStringLiteral("fusuma/config.yml")), fusuma.toUtf8(), error);
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

QString SettingsController::commandOutput(const QString &program, const QStringList &arguments,
                                          int timeoutMs) const
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(timeoutMs) || !process.waitForFinished(timeoutMs))
        return {};
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

QString SettingsController::userUnitState(const QString &unit) const
{
    const QString state = commandOutput(QStringLiteral("systemctl"),
                                        {QStringLiteral("--user"), QStringLiteral("is-active"), unit});
    return state.isEmpty() ? QStringLiteral("unavailable") : state;
}

int SettingsController::processCount(const QString &name) const
{
    const QStringList arguments = name == QLatin1String("fusuma")
        ? QStringList{QStringLiteral("-f"), QStringLiteral("fusuma.*--config")}
        : QStringList{QStringLiteral("-x"), name};
    const QString output = commandOutput(QStringLiteral("pgrep"), arguments);
    if (output.isEmpty()) return 0;
    return output.split(QLatin1Char('\n'), Qt::SkipEmptyParts).size();
}

bool SettingsController::inRiverSession() const
{
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP").contains(
        QStringLiteral("river"), Qt::CaseInsensitive);
}

void SettingsController::refreshSessionStatus()
{
    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    if (desktop.contains(QStringLiteral("river"), Qt::CaseInsensitive))
        m_currentSessionOwner = QStringLiteral("River");
    else if (desktop.contains(QStringLiteral("plasma"), Qt::CaseInsensitive)
             || desktop.contains(QStringLiteral("kde"), Qt::CaseInsensitive))
        m_currentSessionOwner = QStringLiteral("Plasma");
    else
        m_currentSessionOwner = desktop.isEmpty() ? QStringLiteral("Unknown") : desktop;

    auto component = [this](const QString &key, const QString &label,
                            const QString &process, bool enabled) {
        const int count = processCount(process);
        const bool duplicate = count > 1;
        const QString state = duplicate ? QStringLiteral("Duplicate")
            : count == 1 ? QStringLiteral("Running") : QStringLiteral("Stopped");
        QString detail = count == 0 ? QStringLiteral("No process found")
            : count == 1 ? QStringLiteral("1 process")
                         : QStringLiteral("%1 processes detected").arg(count);
        if (!enabled) detail += QStringLiteral(" · River autostart disabled");
        return QVariantMap{{QStringLiteral("key"), key}, {QStringLiteral("name"), label},
                           {QStringLiteral("state"), state}, {QStringLiteral("detail"), detail},
                           {QStringLiteral("owner"), QStringLiteral("River only")},
                           {QStringLiteral("count"), count}, {QStringLiteral("duplicate"), duplicate},
                           {QStringLiteral("autostart"), enabled}};
    };
    m_sessionComponents = {
        component(QStringLiteral("waybar"), QStringLiteral("Waybar"), QStringLiteral("waybar"), m_autostartWaybar),
        component(QStringLiteral("mako"), QStringLiteral("Mako"), QStringLiteral("mako"), m_autostartMako),
        component(QStringLiteral("kanshi"), QStringLiteral("Kanshi"), QStringLiteral("kanshi"), m_autostartKanshi),
        component(QStringLiteral("fusuma"), QStringLiteral("Fusuma"), QStringLiteral("fusuma"), m_autostartFusuma),
        component(QStringLiteral("swayidle"), QStringLiteral("swayidle"), QStringLiteral("swayidle"), m_autostartSwayidle)};

    auto portal = [this](const QString &name, const QString &unit, const QString &role,
                         const QString &owner) {
        const QString state = userUnitState(unit);
        return QVariantMap{{QStringLiteral("name"), name},
                           {QStringLiteral("state"), state == QLatin1String("active") ? QStringLiteral("Active") : state},
                           {QStringLiteral("detail"), role + QStringLiteral(" · ") + unit},
                           {QStringLiteral("owner"), owner},
                           {QStringLiteral("active"), state == QLatin1String("active")}};
    };
    m_portalBackends = {
        portal(QStringLiteral("Portal broker"), QStringLiteral("xdg-desktop-portal.service"),
               QStringLiteral("D-Bus portal broker"), QStringLiteral("River + Plasma")),
        portal(QStringLiteral("wlr backend"), QStringLiteral("xdg-desktop-portal-wlr.service"),
               QStringLiteral("Screenshots and screen casting in River"), QStringLiteral("River only")),
        portal(QStringLiteral("GTK backend"), QStringLiteral("xdg-desktop-portal-gtk.service"),
               QStringLiteral("File chooser and generic interfaces"), QStringLiteral("River + Plasma")),
        portal(QStringLiteral("KDE backend"), QStringLiteral("xdg-desktop-portal-kde.service"),
               QStringLiteral("Plasma-native portal interfaces"), QStringLiteral("Plasma only"))};

    m_riverSessionTargetStatus = userUnitState(QStringLiteral("river-session.target"));
    const QString portalPath = QDir(m_configRoot).filePath(
        QStringLiteral("xdg-desktop-portal/river-portals.conf"));
    QFile portalFile(portalPath);
    if (!portalFile.open(QIODevice::ReadOnly)) {
        m_portalConfigurationStatus = QStringLiteral("Missing: %1").arg(portalPath);
    } else {
        const QString contents = QString::fromUtf8(portalFile.readAll());
        const bool gtkDefault = contents.contains(QRegularExpression(
            QStringLiteral("(?m)^default\\s*=\\s*gtk\\s*$")));
        const bool castWlr = contents.contains(QRegularExpression(
            QStringLiteral("(?m)^org\\.freedesktop\\.impl\\.portal\\.ScreenCast\\s*=\\s*wlr\\s*$")));
        const bool shotWlr = contents.contains(QRegularExpression(
            QStringLiteral("(?m)^org\\.freedesktop\\.impl\\.portal\\.Screenshot\\s*=\\s*wlr\\s*$")));
        m_portalConfigurationStatus = gtkDefault && castWlr && shotWlr
            ? QStringLiteral("Configured correctly: GTK default; ScreenCast and Screenshot use wlr")
            : QStringLiteral("Configuration found, but its preferred backends do not match the River setup");
    }
    emit sessionStatusChanged();
}

bool SettingsController::restartSessionComponent(const QString &component)
{
    if (!inRiverSession() || qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
        setStatus(QStringLiteral("Component restarts are available only inside the River session."), true);
        return false;
    }

    bool started = false;
    if (component == QLatin1String("fusuma") || component == QLatin1String("swayidle")) {
        const QString helper = QDir(m_configRoot).filePath(
            QStringLiteral("river/scripts/start-") + component);
        started = run(helper, {QStringLiteral("restart")}, 4000);
    } else if (component == QLatin1String("waybar")
               || component == QLatin1String("mako")
               || component == QLatin1String("kanshi")) {
        run(QStringLiteral("pkill"), {QStringLiteral("-x"), component});
        started = QProcess::startDetached(component, {});
    }

    if (started)
        setStatus(QStringLiteral("Restarted %1.").arg(component));
    else
        setStatus(QStringLiteral("Could not restart %1.").arg(component), true);
    refreshSessionStatus();
    return started;
}

QString SettingsController::shellQuote(const QString &value) const
{
    QString quoted = value;
    quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + quoted + QLatin1Char('\'');
}

bool SettingsController::applyWindowRules()
{
    auto argumentsFor = [](const QVariantMap &rule, const QString &verb) {
        QStringList arguments{verb};
        const QString appId = rule.value(QStringLiteral("appId")).toString();
        const QString title = rule.value(QStringLiteral("titleContains")).toString();
        if (!appId.isEmpty()) arguments << QStringLiteral("-app-id") << appId;
        if (!title.isEmpty()) arguments << QStringLiteral("-title") << (QStringLiteral("*") + title + QStringLiteral("*"));
        arguments << rule.value(QStringLiteral("action")).toString();
        return arguments;
    };

    bool success = true;
    for (const QVariant &entry : m_savedWindowRules)
        success = run(QStringLiteral("riverctl"), argumentsFor(entry.toMap(), QStringLiteral("rule-del"))) && success;
    for (const QVariant &entry : m_windowRules)
        success = run(QStringLiteral("riverctl"), argumentsFor(entry.toMap(), QStringLiteral("rule-add"))) && success;
    return success;
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

    if (m_powerDirty) {
        const QString riverScripts = QDir(m_configRoot).filePath(QStringLiteral("river/scripts"));
        run(QDir(riverScripts).filePath(QStringLiteral("start-swayidle")),
            {QStringLiteral("restart")}, 3000);
        run(QDir(m_configRoot).filePath(QStringLiteral("waybar/scripts/power-profile")),
            {QStringLiteral("set"), m_defaultPowerProfile}, 5000);
        run(QDir(m_configRoot).filePath(QStringLiteral("waybar/scripts/night-light")),
            {QStringLiteral("reload")}, 3000);
    }
    if (m_inputDirty) {
        const QString riverScripts = QDir(m_configRoot).filePath(QStringLiteral("river/scripts"));
        run(QDir(riverScripts).filePath(QStringLiteral("apply-input")), {}, 3000);
        run(QDir(riverScripts).filePath(QStringLiteral("start-fusuma")),
            {QStringLiteral("restart")}, 3000);
    }
    if (m_windowDirty) {
        applyWindowRules();
        const QString riverScripts = QDir(m_configRoot).filePath(QStringLiteral("river/scripts"));
        run(QDir(riverScripts).filePath(QStringLiteral("apply-window-settings")), {}, 5000);
        run(QDir(riverScripts).filePath(QStringLiteral("start-fusuma")),
            {QStringLiteral("restart")}, 3000);
    }

    run(QDir(m_configRoot).filePath(QStringLiteral("river/scripts/start-background")), {}, 3000);

    if (riverAvailable)
        setStatus(QStringLiteral("Settings applied. Window rules affect newly opened windows."));
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
    refreshSessionStatus();
    m_powerDirty = false;
    m_inputDirty = false;
    m_windowDirty = false;
    m_savedWindowRules = m_windowRules;
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
    m_lidClosePolicy = queryLidClosePolicy();
    refreshSessionStatus();
    emit settingsChanged();
    setStatus(QStringLiteral("Discarded unsaved changes."));
}
