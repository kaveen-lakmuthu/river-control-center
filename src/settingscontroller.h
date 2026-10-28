#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class SettingsController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString base READ base NOTIFY settingsChanged)
    Q_PROPERTY(QString surface READ surface NOTIFY settingsChanged)
    Q_PROPERTY(QString secondarySurface READ secondarySurface NOTIFY settingsChanged)
    Q_PROPERTY(QString text READ text NOTIFY settingsChanged)
    Q_PROPERTY(QString mutedText READ mutedText NOTIFY settingsChanged)
    Q_PROPERTY(QString accent READ accent NOTIFY settingsChanged)
    Q_PROPERTY(QString secondaryAccent READ secondaryAccent NOTIFY settingsChanged)
    Q_PROPERTY(QString highlight READ highlight NOTIFY settingsChanged)
    Q_PROPERTY(QString urgent READ urgent NOTIFY settingsChanged)
    Q_PROPERTY(QString success READ success NOTIFY settingsChanged)
    Q_PROPERTY(QString borderUnfocused READ borderUnfocused NOTIFY settingsChanged)
    Q_PROPERTY(QString waybarFontFamily READ waybarFontFamily WRITE setWaybarFontFamily NOTIFY settingsChanged)
    Q_PROPERTY(QString makoFontFamily READ makoFontFamily WRITE setMakoFontFamily NOTIFY settingsChanged)
    Q_PROPERTY(int makoFontSize READ makoFontSize WRITE setMakoFontSize NOTIFY settingsChanged)
    Q_PROPERTY(QString fuzzelFontFamily READ fuzzelFontFamily WRITE setFuzzelFontFamily NOTIFY settingsChanged)
    Q_PROPERTY(int fuzzelFontSize READ fuzzelFontSize WRITE setFuzzelFontSize NOTIFY settingsChanged)
    Q_PROPERTY(QString footFontFamily READ footFontFamily WRITE setFootFontFamily NOTIFY settingsChanged)
    Q_PROPERTY(double footFontSize READ footFontSize WRITE setFootFontSize NOTIFY settingsChanged)
    Q_PROPERTY(QString panelFontFamily READ panelFontFamily WRITE setPanelFontFamily NOTIFY settingsChanged)
    Q_PROPERTY(int panelFontSize READ panelFontSize WRITE setPanelFontSize NOTIFY settingsChanged)
    Q_PROPERTY(QString backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath WRITE setWallpaperPath NOTIFY settingsChanged)
    Q_PROPERTY(int interfaceRadius READ interfaceRadius WRITE setInterfaceRadius NOTIFY settingsChanged)
    Q_PROPERTY(int panelSpacing READ panelSpacing WRITE setPanelSpacing NOTIFY settingsChanged)
    Q_PROPERTY(bool themeWaybar READ themeWaybar WRITE setThemeWaybar NOTIFY settingsChanged)
    Q_PROPERTY(bool themeMako READ themeMako WRITE setThemeMako NOTIFY settingsChanged)
    Q_PROPERTY(bool themeFuzzel READ themeFuzzel WRITE setThemeFuzzel NOTIFY settingsChanged)
    Q_PROPERTY(bool themeFoot READ themeFoot WRITE setThemeFoot NOTIFY settingsChanged)
    Q_PROPERTY(bool themePanels READ themePanels WRITE setThemePanels NOTIFY settingsChanged)
    Q_PROPERTY(QStringList customThemes READ customThemes NOTIFY themeListChanged)
    Q_PROPERTY(QStringList contrastWarnings READ contrastWarnings NOTIFY settingsChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY settingsChanged)
    Q_PROPERTY(int viewPadding READ viewPadding WRITE setViewPadding NOTIFY settingsChanged)
    Q_PROPERTY(int outerPadding READ outerPadding WRITE setOuterPadding NOTIFY settingsChanged)

    Q_PROPERTY(int barHeight READ barHeight WRITE setBarHeight NOTIFY settingsChanged)
    Q_PROPERTY(int barFontSize READ barFontSize WRITE setBarFontSize NOTIFY settingsChanged)
    Q_PROPERTY(int modulePadding READ modulePadding WRITE setModulePadding NOTIFY settingsChanged)
    Q_PROPERTY(int tagPadding READ tagPadding WRITE setTagPadding NOTIFY settingsChanged)
    Q_PROPERTY(QString clockFormat READ clockFormat WRITE setClockFormat NOTIFY settingsChanged)
    Q_PROPERTY(int batteryWarning READ batteryWarning WRITE setBatteryWarning NOTIFY settingsChanged)
    Q_PROPERTY(int batteryCritical READ batteryCritical WRITE setBatteryCritical NOTIFY settingsChanged)
    Q_PROPERTY(bool showTags READ showTags WRITE setShowTags NOTIFY settingsChanged)
    Q_PROPERTY(bool showWindowTitle READ showWindowTitle WRITE setShowWindowTitle NOTIFY settingsChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY settingsChanged)
    Q_PROPERTY(bool showVolume READ showVolume WRITE setShowVolume NOTIFY settingsChanged)
    Q_PROPERTY(bool showBattery READ showBattery WRITE setShowBattery NOTIFY settingsChanged)
    Q_PROPERTY(bool showKeyboardLayout READ showKeyboardLayout WRITE setShowKeyboardLayout NOTIFY settingsChanged)
    Q_PROPERTY(bool showClipboard READ showClipboard WRITE setShowClipboard NOTIFY settingsChanged)
    Q_PROPERTY(bool showWindowControls READ showWindowControls WRITE setShowWindowControls NOTIFY settingsChanged)
    Q_PROPERTY(bool showTray READ showTray WRITE setShowTray NOTIFY settingsChanged)
    Q_PROPERTY(bool showSystemMenu READ showSystemMenu WRITE setShowSystemMenu NOTIFY settingsChanged)

    Q_PROPERTY(int clipboardHistoryLimit READ clipboardHistoryLimit WRITE setClipboardHistoryLimit NOTIFY settingsChanged)
    Q_PROPERTY(int notificationHistoryLimit READ notificationHistoryLimit WRITE setNotificationHistoryLimit NOTIFY settingsChanged)
    Q_PROPERTY(int panelWidth READ panelWidth WRITE setPanelWidth NOTIFY settingsChanged)
    Q_PROPERTY(int panelHeight READ panelHeight WRITE setPanelHeight NOTIFY settingsChanged)
    Q_PROPERTY(int panelPadding READ panelPadding WRITE setPanelPadding NOTIFY settingsChanged)
    Q_PROPERTY(int panelTopMargin READ panelTopMargin WRITE setPanelTopMargin NOTIFY settingsChanged)
    Q_PROPERTY(int panelRightMargin READ panelRightMargin WRITE setPanelRightMargin NOTIFY settingsChanged)

    Q_PROPERTY(int lockTimeoutMinutes READ lockTimeoutMinutes WRITE setLockTimeoutMinutes NOTIFY settingsChanged)
    Q_PROPERTY(int displayOffTimeoutMinutes READ displayOffTimeoutMinutes WRITE setDisplayOffTimeoutMinutes NOTIFY settingsChanged)
    Q_PROPERTY(int batterySuspendTimeoutMinutes READ batterySuspendTimeoutMinutes WRITE setBatterySuspendTimeoutMinutes NOTIFY settingsChanged)
    Q_PROPERTY(int acSuspendTimeoutMinutes READ acSuspendTimeoutMinutes WRITE setAcSuspendTimeoutMinutes NOTIFY settingsChanged)
    Q_PROPERTY(bool lockOnIdle READ lockOnIdle WRITE setLockOnIdle NOTIFY settingsChanged)
    Q_PROPERTY(bool displayOffOnIdle READ displayOffOnIdle WRITE setDisplayOffOnIdle NOTIFY settingsChanged)
    Q_PROPERTY(bool suspendOnBattery READ suspendOnBattery WRITE setSuspendOnBattery NOTIFY settingsChanged)
    Q_PROPERTY(bool suspendOnAc READ suspendOnAc WRITE setSuspendOnAc NOTIFY settingsChanged)
    Q_PROPERTY(bool lockBeforeSuspend READ lockBeforeSuspend WRITE setLockBeforeSuspend NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultPowerProfile READ defaultPowerProfile WRITE setDefaultPowerProfile NOTIFY settingsChanged)
    Q_PROPERTY(QString nightLightStart READ nightLightStart WRITE setNightLightStart NOTIFY settingsChanged)
    Q_PROPERTY(QString nightLightStop READ nightLightStop WRITE setNightLightStop NOTIFY settingsChanged)
    Q_PROPERTY(QString lidClosePolicy READ lidClosePolicy NOTIFY settingsChanged)

    Q_PROPERTY(bool touchpadTap READ touchpadTap WRITE setTouchpadTap NOTIFY settingsChanged)
    Q_PROPERTY(bool touchpadNaturalScroll READ touchpadNaturalScroll WRITE setTouchpadNaturalScroll NOTIFY settingsChanged)
    Q_PROPERTY(bool touchpadDisableWhileTyping READ touchpadDisableWhileTyping WRITE setTouchpadDisableWhileTyping NOTIFY settingsChanged)
    Q_PROPERTY(double touchpadPointerAccel READ touchpadPointerAccel WRITE setTouchpadPointerAccel NOTIFY settingsChanged)
    Q_PROPERTY(QString touchpadClickMethod READ touchpadClickMethod WRITE setTouchpadClickMethod NOTIFY settingsChanged)
    Q_PROPERTY(int keyboardRepeatRate READ keyboardRepeatRate WRITE setKeyboardRepeatRate NOTIFY settingsChanged)
    Q_PROPERTY(int keyboardRepeatDelay READ keyboardRepeatDelay WRITE setKeyboardRepeatDelay NOTIFY settingsChanged)
    Q_PROPERTY(bool sinhalaLayoutEnabled READ sinhalaLayoutEnabled WRITE setSinhalaLayoutEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QString composeKey READ composeKey WRITE setComposeKey NOTIFY settingsChanged)
    Q_PROPERTY(int gestureFingerCount READ gestureFingerCount WRITE setGestureFingerCount NOTIFY settingsChanged)
    Q_PROPERTY(QString gestureDirection READ gestureDirection WRITE setGestureDirection NOTIFY settingsChanged)
    Q_PROPERTY(QString gestureDeviceName READ gestureDeviceName WRITE setGestureDeviceName NOTIFY settingsChanged)

    Q_PROPERTY(double mainWindowRatio READ mainWindowRatio WRITE setMainWindowRatio NOTIFY settingsChanged)
    Q_PROPERTY(int mainWindowCount READ mainWindowCount WRITE setMainWindowCount NOTIFY settingsChanged)
    Q_PROPERTY(QString mainPosition READ mainPosition WRITE setMainPosition NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultAttachMode READ defaultAttachMode WRITE setDefaultAttachMode NOTIFY settingsChanged)
    Q_PROPERTY(QString focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultLayoutMode READ defaultLayoutMode WRITE setDefaultLayoutMode NOTIFY settingsChanged)
    Q_PROPERTY(int tagCount READ tagCount WRITE setTagCount NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList windowRules READ windowRules NOTIFY windowRulesChanged)
    Q_PROPERTY(QVariantList openWindows READ openWindows NOTIFY openWindowsChanged)
    Q_PROPERTY(QString ruleTestResult READ ruleTestResult NOTIFY ruleTestResultChanged)

    Q_PROPERTY(bool autostartWaybar READ autostartWaybar WRITE setAutostartWaybar NOTIFY settingsChanged)
    Q_PROPERTY(bool autostartMako READ autostartMako WRITE setAutostartMako NOTIFY settingsChanged)
    Q_PROPERTY(bool autostartKanshi READ autostartKanshi WRITE setAutostartKanshi NOTIFY settingsChanged)
    Q_PROPERTY(bool autostartFusuma READ autostartFusuma WRITE setAutostartFusuma NOTIFY settingsChanged)
    Q_PROPERTY(bool autostartSwayidle READ autostartSwayidle WRITE setAutostartSwayidle NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList sessionComponents READ sessionComponents NOTIFY sessionStatusChanged)
    Q_PROPERTY(QVariantList portalBackends READ portalBackends NOTIFY sessionStatusChanged)
    Q_PROPERTY(QString riverSessionTargetStatus READ riverSessionTargetStatus NOTIFY sessionStatusChanged)
    Q_PROPERTY(QString portalConfigurationStatus READ portalConfigurationStatus NOTIFY sessionStatusChanged)
    Q_PROPERTY(QString currentSessionOwner READ currentSessionOwner NOTIFY sessionStatusChanged)
    Q_PROPERTY(bool inRiverSession READ inRiverSession NOTIFY sessionStatusChanged)

    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(int revision READ revision NOTIFY settingsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
    Q_PROPERTY(bool statusError READ statusError NOTIFY statusChanged)
    Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)

public:
    explicit SettingsController(QObject *parent = nullptr);

    QString base() const { return m_base; }
    QString surface() const { return m_surface; }
    QString secondarySurface() const { return m_secondarySurface; }
    QString text() const { return m_text; }
    QString mutedText() const { return m_mutedText; }
    QString accent() const { return m_accent; }
    QString secondaryAccent() const { return m_secondaryAccent; }
    QString highlight() const { return m_highlight; }
    QString urgent() const { return m_urgent; }
    QString success() const { return m_success; }
    QString borderUnfocused() const { return m_borderUnfocused; }
    QString waybarFontFamily() const { return m_waybarFontFamily; }
    QString makoFontFamily() const { return m_makoFontFamily; }
    int makoFontSize() const { return m_makoFontSize; }
    QString fuzzelFontFamily() const { return m_fuzzelFontFamily; }
    int fuzzelFontSize() const { return m_fuzzelFontSize; }
    QString footFontFamily() const { return m_footFontFamily; }
    double footFontSize() const { return m_footFontSize; }
    QString panelFontFamily() const { return m_panelFontFamily; }
    int panelFontSize() const { return m_panelFontSize; }
    QString backgroundMode() const { return m_backgroundMode; }
    QString wallpaperPath() const { return m_wallpaperPath; }
    int interfaceRadius() const { return m_interfaceRadius; }
    int panelSpacing() const { return m_panelSpacing; }
    bool themeWaybar() const { return m_themeWaybar; }
    bool themeMako() const { return m_themeMako; }
    bool themeFuzzel() const { return m_themeFuzzel; }
    bool themeFoot() const { return m_themeFoot; }
    bool themePanels() const { return m_themePanels; }
    QStringList customThemes() const;
    QStringList contrastWarnings() const;

    int borderWidth() const { return m_borderWidth; }
    int viewPadding() const { return m_viewPadding; }
    int outerPadding() const { return m_outerPadding; }

    int barHeight() const { return m_barHeight; }
    int barFontSize() const { return m_barFontSize; }
    int modulePadding() const { return m_modulePadding; }
    int tagPadding() const { return m_tagPadding; }
    QString clockFormat() const { return m_clockFormat; }
    int batteryWarning() const { return m_batteryWarning; }
    int batteryCritical() const { return m_batteryCritical; }
    bool showTags() const { return m_showTags; }
    bool showWindowTitle() const { return m_showWindowTitle; }
    bool showClock() const { return m_showClock; }
    bool showVolume() const { return m_showVolume; }
    bool showBattery() const { return m_showBattery; }
    bool showKeyboardLayout() const { return m_showKeyboardLayout; }
    bool showClipboard() const { return m_showClipboard; }
    bool showWindowControls() const { return m_showWindowControls; }
    bool showTray() const { return m_showTray; }
    bool showSystemMenu() const { return m_showSystemMenu; }

    int clipboardHistoryLimit() const { return m_clipboardHistoryLimit; }
    int notificationHistoryLimit() const { return m_notificationHistoryLimit; }
    int panelWidth() const { return m_panelWidth; }
    int panelHeight() const { return m_panelHeight; }
    int panelPadding() const { return m_panelPadding; }
    int panelTopMargin() const { return m_panelTopMargin; }
    int panelRightMargin() const { return m_panelRightMargin; }

    int lockTimeoutMinutes() const { return m_lockTimeoutMinutes; }
    int displayOffTimeoutMinutes() const { return m_displayOffTimeoutMinutes; }
    int batterySuspendTimeoutMinutes() const { return m_batterySuspendTimeoutMinutes; }
    int acSuspendTimeoutMinutes() const { return m_acSuspendTimeoutMinutes; }
    bool lockOnIdle() const { return m_lockOnIdle; }
    bool displayOffOnIdle() const { return m_displayOffOnIdle; }
    bool suspendOnBattery() const { return m_suspendOnBattery; }
    bool suspendOnAc() const { return m_suspendOnAc; }
    bool lockBeforeSuspend() const { return m_lockBeforeSuspend; }
    QString defaultPowerProfile() const { return m_defaultPowerProfile; }
    QString nightLightStart() const { return m_nightLightStart; }
    QString nightLightStop() const { return m_nightLightStop; }
    QString lidClosePolicy() const { return m_lidClosePolicy; }

    bool touchpadTap() const { return m_touchpadTap; }
    bool touchpadNaturalScroll() const { return m_touchpadNaturalScroll; }
    bool touchpadDisableWhileTyping() const { return m_touchpadDisableWhileTyping; }
    double touchpadPointerAccel() const { return m_touchpadPointerAccel; }
    QString touchpadClickMethod() const { return m_touchpadClickMethod; }
    int keyboardRepeatRate() const { return m_keyboardRepeatRate; }
    int keyboardRepeatDelay() const { return m_keyboardRepeatDelay; }
    bool sinhalaLayoutEnabled() const { return m_sinhalaLayoutEnabled; }
    QString composeKey() const { return m_composeKey; }
    int gestureFingerCount() const { return m_gestureFingerCount; }
    QString gestureDirection() const { return m_gestureDirection; }
    QString gestureDeviceName() const { return m_gestureDeviceName; }

    double mainWindowRatio() const { return m_mainWindowRatio; }
    int mainWindowCount() const { return m_mainWindowCount; }
    QString mainPosition() const { return m_mainPosition; }
    QString defaultAttachMode() const { return m_defaultAttachMode; }
    QString focusPolicy() const { return m_focusPolicy; }
    QString defaultLayoutMode() const { return m_defaultLayoutMode; }
    int tagCount() const { return m_tagCount; }
    QVariantList windowRules() const { return m_windowRules; }
    QVariantList openWindows() const { return m_openWindows; }
    QString ruleTestResult() const { return m_ruleTestResult; }
    bool autostartWaybar() const { return m_autostartWaybar; }
    bool autostartMako() const { return m_autostartMako; }
    bool autostartKanshi() const { return m_autostartKanshi; }
    bool autostartFusuma() const { return m_autostartFusuma; }
    bool autostartSwayidle() const { return m_autostartSwayidle; }
    QVariantList sessionComponents() const { return m_sessionComponents; }
    QVariantList portalBackends() const { return m_portalBackends; }
    QString riverSessionTargetStatus() const { return m_riverSessionTargetStatus; }
    QString portalConfigurationStatus() const { return m_portalConfigurationStatus; }
    QString currentSessionOwner() const { return m_currentSessionOwner; }
    bool inRiverSession() const;

    bool dirty() const { return m_dirty; }
    int revision() const { return m_revision; }
    QString statusMessage() const { return m_statusMessage; }
    bool statusError() const { return m_statusError; }
    QString settingsPath() const { return m_settingsPath; }

    void setBorderWidth(int value);
    void setViewPadding(int value);
    void setOuterPadding(int value);
    void setWaybarFontFamily(const QString &value);
    void setMakoFontFamily(const QString &value);
    void setMakoFontSize(int value);
    void setFuzzelFontFamily(const QString &value);
    void setFuzzelFontSize(int value);
    void setFootFontFamily(const QString &value);
    void setFootFontSize(double value);
    void setPanelFontFamily(const QString &value);
    void setPanelFontSize(int value);
    void setBackgroundMode(const QString &value);
    void setWallpaperPath(const QString &value);
    void setInterfaceRadius(int value);
    void setPanelSpacing(int value);
    void setThemeWaybar(bool value);
    void setThemeMako(bool value);
    void setThemeFuzzel(bool value);
    void setThemeFoot(bool value);
    void setThemePanels(bool value);
    void setBarHeight(int value);
    void setBarFontSize(int value);
    void setModulePadding(int value);
    void setTagPadding(int value);
    void setClockFormat(const QString &value);
    void setBatteryWarning(int value);
    void setBatteryCritical(int value);
    void setShowTags(bool value);
    void setShowWindowTitle(bool value);
    void setShowClock(bool value);
    void setShowVolume(bool value);
    void setShowBattery(bool value);
    void setShowKeyboardLayout(bool value);
    void setShowClipboard(bool value);
    void setShowWindowControls(bool value);
    void setShowTray(bool value);
    void setShowSystemMenu(bool value);
    void setClipboardHistoryLimit(int value);
    void setNotificationHistoryLimit(int value);
    void setPanelWidth(int value);
    void setPanelHeight(int value);
    void setPanelPadding(int value);
    void setPanelTopMargin(int value);
    void setPanelRightMargin(int value);
    void setLockTimeoutMinutes(int value);
    void setDisplayOffTimeoutMinutes(int value);
    void setBatterySuspendTimeoutMinutes(int value);
    void setAcSuspendTimeoutMinutes(int value);
    void setLockOnIdle(bool value);
    void setDisplayOffOnIdle(bool value);
    void setSuspendOnBattery(bool value);
    void setSuspendOnAc(bool value);
    void setLockBeforeSuspend(bool value);
    void setDefaultPowerProfile(const QString &value);
    void setNightLightStart(const QString &value);
    void setNightLightStop(const QString &value);
    void setTouchpadTap(bool value);
    void setTouchpadNaturalScroll(bool value);
    void setTouchpadDisableWhileTyping(bool value);
    void setTouchpadPointerAccel(double value);
    void setTouchpadClickMethod(const QString &value);
    void setKeyboardRepeatRate(int value);
    void setKeyboardRepeatDelay(int value);
    void setSinhalaLayoutEnabled(bool value);
    void setComposeKey(const QString &value);
    void setGestureFingerCount(int value);
    void setGestureDirection(const QString &value);
    void setGestureDeviceName(const QString &value);
    void setMainWindowRatio(double value);
    void setMainWindowCount(int value);
    void setMainPosition(const QString &value);
    void setDefaultAttachMode(const QString &value);
    void setFocusPolicy(const QString &value);
    void setDefaultLayoutMode(const QString &value);
    void setTagCount(int value);
    void setAutostartWaybar(bool value);
    void setAutostartMako(bool value);
    void setAutostartKanshi(bool value);
    void setAutostartFusuma(bool value);
    void setAutostartSwayidle(bool value);

    Q_INVOKABLE QString colorValue(const QString &key) const;
    Q_INVOKABLE bool setColor(const QString &key, const QString &value);
    Q_INVOKABLE void applyPreset(const QString &name);
    Q_INVOKABLE bool saveCustomTheme(const QString &name);
    Q_INVOKABLE bool applyCustomTheme(const QString &name);
    Q_INVOKABLE bool duplicateTheme(const QString &name, const QString &newName);
    Q_INVOKABLE bool renameTheme(const QString &name, const QString &newName);
    Q_INVOKABLE bool importTheme(const QString &urlOrPath);
    Q_INVOKABLE bool exportTheme(const QString &name);
    Q_INVOKABLE bool saveAndApply();
    Q_INVOKABLE void reload();
    Q_INVOKABLE QString validationError() const;
    Q_INVOKABLE void addWindowRule();
    Q_INVOKABLE void removeWindowRule(int index);
    Q_INVOKABLE void updateWindowRule(int index, const QString &appId,
                                      const QString &titleContains,
                                      const QString &action);
    Q_INVOKABLE void refreshOpenWindows();
    Q_INVOKABLE void testWindowRule(int index);
    Q_INVOKABLE void refreshSessionStatus();
    Q_INVOKABLE bool restartSessionComponent(const QString &component);
    bool regenerate(QString *error = nullptr);

signals:
    void settingsChanged();
    void dirtyChanged();
    void statusChanged();
    void windowRulesChanged();
    void openWindowsChanged();
    void ruleTestResultChanged();
    void themeListChanged();
    void sessionStatusChanged();

private:
    void setDefaults();
    bool loadFromDisk(QString *error = nullptr);
    bool writeSettings(QString *error);
    bool generateFiles(QString *error);
    void applyLive();
    void markChanged();
    void setDirty(bool value);
    void setStatus(const QString &message, bool error = false);
    QString normalizedColor(const QString &value) const;
    QString colorWithoutHash(const QString &value) const;
    QString lighter(const QString &value, int factor = 120) const;
    QString queryLidClosePolicy() const;
    QJsonObject appearanceJson() const;
    bool loadAppearanceJson(const QJsonObject &appearance, QString *error = nullptr);
    QString themeFilePath(const QString &name) const;
    QString validThemeName(const QString &name) const;
    bool applyWindowRules();
    QString shellQuote(const QString &value) const;
    bool writeFileAtomically(const QString &path, const QByteArray &contents, QString *error) const;
    bool run(const QString &program, const QStringList &arguments, int timeoutMs = 1500) const;
    QString commandOutput(const QString &program, const QStringList &arguments,
                          int timeoutMs = 1500) const;
    QString userUnitState(const QString &unit) const;
    int processCount(const QString &name) const;

    QString m_settingsPath;
    QString m_generatedPath;
    QString m_configRoot;

    QString m_base;
    QString m_surface;
    QString m_secondarySurface;
    QString m_text;
    QString m_mutedText;
    QString m_accent;
    QString m_secondaryAccent;
    QString m_highlight;
    QString m_urgent;
    QString m_success;
    QString m_borderUnfocused;
    QString m_waybarFontFamily;
    QString m_makoFontFamily;
    int m_makoFontSize = 10;
    QString m_fuzzelFontFamily;
    int m_fuzzelFontSize = 11;
    QString m_footFontFamily;
    double m_footFontSize = 10.5;
    QString m_panelFontFamily;
    int m_panelFontSize = 10;
    QString m_backgroundMode;
    QString m_wallpaperPath;
    int m_interfaceRadius = 8;
    int m_panelSpacing = 10;
    bool m_themeWaybar = true;
    bool m_themeMako = true;
    bool m_themeFuzzel = true;
    bool m_themeFoot = true;
    bool m_themePanels = true;

    int m_borderWidth = 2;
    int m_viewPadding = 0;
    int m_outerPadding = 0;

    int m_barHeight = 26;
    int m_barFontSize = 11;
    int m_modulePadding = 7;
    int m_tagPadding = 6;
    QString m_clockFormat;
    int m_batteryWarning = 25;
    int m_batteryCritical = 12;
    bool m_showTags = true;
    bool m_showWindowTitle = true;
    bool m_showClock = true;
    bool m_showVolume = true;
    bool m_showBattery = true;
    bool m_showKeyboardLayout = true;
    bool m_showClipboard = true;
    bool m_showWindowControls = true;
    bool m_showTray = true;
    bool m_showSystemMenu = true;

    int m_clipboardHistoryLimit = 40;
    int m_notificationHistoryLimit = 25;
    int m_panelWidth = 430;
    int m_panelHeight = 420;
    int m_panelPadding = 16;
    int m_panelTopMargin = 4;
    int m_panelRightMargin = 6;

    int m_lockTimeoutMinutes = 10;
    int m_displayOffTimeoutMinutes = 15;
    int m_batterySuspendTimeoutMinutes = 30;
    int m_acSuspendTimeoutMinutes = 60;
    bool m_lockOnIdle = true;
    bool m_displayOffOnIdle = true;
    bool m_suspendOnBattery = true;
    bool m_suspendOnAc = true;
    bool m_lockBeforeSuspend = true;
    QString m_defaultPowerProfile;
    QString m_nightLightStart;
    QString m_nightLightStop;
    QString m_lidClosePolicy;
    bool m_touchpadTap = true;
    bool m_touchpadNaturalScroll = true;
    bool m_touchpadDisableWhileTyping = true;
    double m_touchpadPointerAccel = 0.1;
    QString m_touchpadClickMethod;
    int m_keyboardRepeatRate = 50;
    int m_keyboardRepeatDelay = 300;
    bool m_sinhalaLayoutEnabled = true;
    QString m_composeKey;
    int m_gestureFingerCount = 3;
    QString m_gestureDirection;
    QString m_gestureDeviceName;
    double m_mainWindowRatio = 0.6;
    int m_mainWindowCount = 1;
    QString m_mainPosition;
    QString m_defaultAttachMode;
    QString m_focusPolicy;
    QString m_defaultLayoutMode;
    int m_tagCount = 9;
    QVariantList m_windowRules;
    QVariantList m_savedWindowRules;
    QVariantList m_openWindows;
    QString m_ruleTestResult;
    bool m_autostartWaybar = true;
    bool m_autostartMako = true;
    bool m_autostartKanshi = true;
    bool m_autostartFusuma = true;
    bool m_autostartSwayidle = true;
    QVariantList m_sessionComponents;
    QVariantList m_portalBackends;
    QString m_riverSessionTargetStatus;
    QString m_portalConfigurationStatus;
    QString m_currentSessionOwner;
    bool m_dirty = false;
    bool m_powerDirty = false;
    bool m_inputDirty = false;
    bool m_windowDirty = false;
    bool m_loading = false;
    int m_revision = 0;
    QString m_statusMessage;
    bool m_statusError = false;
};
