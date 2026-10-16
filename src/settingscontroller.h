#pragma once

#include <QObject>
#include <QString>

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

    bool dirty() const { return m_dirty; }
    int revision() const { return m_revision; }
    QString statusMessage() const { return m_statusMessage; }
    bool statusError() const { return m_statusError; }
    QString settingsPath() const { return m_settingsPath; }

    void setBorderWidth(int value);
    void setViewPadding(int value);
    void setOuterPadding(int value);
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

    Q_INVOKABLE QString colorValue(const QString &key) const;
    Q_INVOKABLE bool setColor(const QString &key, const QString &value);
    Q_INVOKABLE void applyPreset(const QString &name);
    Q_INVOKABLE bool saveAndApply();
    Q_INVOKABLE void reload();
    Q_INVOKABLE QString validationError() const;
    bool regenerate(QString *error = nullptr);

signals:
    void settingsChanged();
    void dirtyChanged();
    void statusChanged();

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
    bool writeFileAtomically(const QString &path, const QByteArray &contents, QString *error) const;
    bool run(const QString &program, const QStringList &arguments, int timeoutMs = 1500) const;

    QString m_settingsPath;
    QString m_generatedPath;

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
    bool m_dirty = false;
    bool m_loading = false;
    int m_revision = 0;
    QString m_statusMessage;
    bool m_statusError = false;
};
