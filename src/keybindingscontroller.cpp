#include "keybindingscontroller.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>

namespace {
QVariantMap binding(const QString &id, const QString &label, const QString &category,
                    const QString &mode, const QString &modifiers, const QString &key,
                    const QString &command)
{
    return {{QStringLiteral("id"), id}, {QStringLiteral("label"), label},
            {QStringLiteral("category"), category}, {QStringLiteral("mode"), mode},
            {QStringLiteral("modifiers"), modifiers}, {QStringLiteral("key"), key},
            {QStringLiteral("command"), command}};
}

QString shownChord(const QVariantMap &item)
{
    const QString modifiers = item.value(QStringLiteral("modifiers")).toString();
    return (modifiers == QLatin1String("None") || modifiers.isEmpty())
        ? item.value(QStringLiteral("key")).toString()
        : modifiers + QLatin1Char('+') + item.value(QStringLiteral("key")).toString();
}
}

QVariantList KeybindingsController::defaultBindings()
{
    QVariantList b;
    auto add = [&b](const char *id, const char *label, const char *category,
                    const char *mode, const char *mods, const char *key, const char *command) {
        b.append(binding(QString::fromLatin1(id), QString::fromLatin1(label), QString::fromLatin1(category),
                         QString::fromLatin1(mode), QString::fromLatin1(mods),
                         QString::fromLatin1(key), QString::fromLatin1(command)));
    };
    add("layout-left", "Toggle keyboard layout (left)", "keyboard", "normal", "Alt", "Shift_L", "spawn '$HOME/.config/waybar/scripts/keyboard-layout toggle'");
    add("layout-right", "Toggle keyboard layout (right)", "keyboard", "normal", "Alt", "Shift_R", "spawn '$HOME/.config/waybar/scripts/keyboard-layout toggle'");
    add("layout-left-reverse", "Toggle keyboard layout (left reverse)", "keyboard", "normal", "Shift", "Alt_L", "spawn '$HOME/.config/waybar/scripts/keyboard-layout toggle'");
    add("layout-right-reverse", "Toggle keyboard layout (right reverse)", "keyboard", "normal", "Shift", "Alt_R", "spawn '$HOME/.config/waybar/scripts/keyboard-layout toggle'");
    add("terminal", "Terminal", "keyboard", "normal", "Super", "Return", "spawn foot");
    add("launcher", "Application launcher", "keyboard", "normal", "Super", "D", "spawn fuzzel");
    add("files", "File manager", "keyboard", "normal", "Super", "T", "spawn dolphin");
    add("emacs", "Emacs client", "keyboard", "normal", "Super", "E", "spawn 'systemctl --user start emacs.service && emacsclient --create-frame --no-wait'");
    add("browser", "Brave Origin", "keyboard", "normal", "Super", "B", "spawn '$HOME/.local/bin/brave-kwallet'");
    add("close", "Close focused window", "keyboard", "normal", "Super", "Q", "close");
    add("exit", "Exit River", "keyboard", "normal", "Super+Shift", "E", "exit");
    add("focus-next", "Focus next window", "keyboard", "normal", "Super", "J", "focus-view next");
    add("focus-previous", "Focus previous window", "keyboard", "normal", "Super", "K", "focus-view previous");
    add("swap-next", "Swap with next window", "keyboard", "normal", "Super+Shift", "J", "swap next");
    add("swap-previous", "Swap with previous window", "keyboard", "normal", "Super+Shift", "K", "swap previous");
    add("output-next", "Focus next monitor", "keyboard", "normal", "Super", "Period", "focus-output next");
    add("output-previous", "Focus previous monitor", "keyboard", "normal", "Super", "Comma", "focus-output previous");
    add("send-output-next", "Send window to next monitor", "keyboard", "normal", "Super+Shift", "Period", "send-to-output next");
    add("send-output-previous", "Send window to previous monitor", "keyboard", "normal", "Super+Shift", "Comma", "send-to-output previous");
    add("zoom", "Promote window to main", "keyboard", "normal", "Super", "Z", "zoom");
    add("ratio-down", "Decrease main ratio", "keyboard", "normal", "Super", "H", "send-layout-cmd rivertile 'main-ratio -0.05'");
    add("ratio-up", "Increase main ratio", "keyboard", "normal", "Super", "L", "send-layout-cmd rivertile 'main-ratio +0.05'");
    add("main-count-up", "Increase main count", "keyboard", "normal", "Super+Shift", "H", "send-layout-cmd rivertile 'main-count +1'");
    add("main-count-down", "Decrease main count", "keyboard", "normal", "Super+Shift", "L", "send-layout-cmd rivertile 'main-count -1'");
    add("main-top", "Put main area at top", "keyboard", "normal", "Super", "Up", "send-layout-cmd rivertile 'main-location top'");
    add("main-right", "Put main area at right", "keyboard", "normal", "Super", "Right", "send-layout-cmd rivertile 'main-location right'");
    add("main-bottom", "Put main area at bottom", "keyboard", "normal", "Super", "Down", "send-layout-cmd rivertile 'main-location bottom'");
    add("main-left", "Put main area at left", "keyboard", "normal", "Super", "Left", "send-layout-cmd rivertile 'main-location left'");
    add("toggle-float", "Toggle window floating", "keyboard", "normal", "Super", "Space", "toggle-float");
    add("global-layout", "Toggle tiled/floating mode", "keyboard", "normal", "Super+Shift", "Space", "spawn '$HOME/.config/waybar/scripts/layout-mode toggle'");
    add("fullscreen", "Toggle fullscreen", "keyboard", "normal", "Super", "F", "toggle-fullscreen");
    add("clipboard", "Clipboard history", "keyboard", "normal", "Super", "V", "spawn '$HOME/.config/waybar/scripts/desktop-panel.py clipboard'");
    add("screenshot-full", "Screenshot all outputs", "keyboard", "normal", "None", "Print", "spawn '$HOME/.config/river/scripts/screenshot full'");
    add("screenshot-region", "Screenshot region", "keyboard", "normal", "Super", "Print", "spawn '$HOME/.config/river/scripts/screenshot region'");
    add("screenshot-annotate", "Screenshot and annotate", "keyboard", "normal", "Super+Shift", "Print", "spawn '$HOME/.config/river/scripts/screenshot annotate'");
    add("lock", "Lock screen", "keyboard", "normal", "Super+Control", "L", "spawn '$HOME/.config/river/scripts/lock-screen'");
    add("pointer-move", "Move floating window", "mouse", "normal", "Super", "BTN_LEFT", "move-view");
    add("pointer-resize", "Resize floating window", "mouse", "normal", "Super", "BTN_RIGHT", "resize-view");
    add("pointer-float", "Toggle floating window", "mouse", "normal", "Super", "BTN_MIDDLE", "toggle-float");
    add("volume-up", "Volume up", "media", "both", "None", "XF86AudioRaiseVolume", "spawn 'wpctl set-volume -l 1.0 @DEFAULT_AUDIO_SINK@ 5%+'");
    add("volume-down", "Volume down", "media", "both", "None", "XF86AudioLowerVolume", "spawn 'wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-'");
    add("volume-mute", "Toggle output mute", "media", "both", "None", "XF86AudioMute", "spawn 'wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle'");
    add("mic-mute", "Toggle microphone mute", "media", "both", "None", "XF86AudioMicMute", "spawn 'wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle'");
    add("media-play", "Play or pause media", "media", "both", "None", "XF86AudioPlay", "spawn 'playerctl play-pause'");
    add("media-next", "Next media track", "media", "both", "None", "XF86AudioNext", "spawn 'playerctl next'");
    add("media-previous", "Previous media track", "media", "both", "None", "XF86AudioPrev", "spawn 'playerctl previous'");
    add("brightness-up", "Brightness up", "media", "both", "None", "XF86MonBrightnessUp", "spawn '$HOME/.config/river/scripts/brightness up'");
    add("brightness-down", "Brightness down", "media", "both", "None", "XF86MonBrightnessDown", "spawn '$HOME/.config/river/scripts/brightness down'");
    return b;
}

KeybindingsController::KeybindingsController(QObject *parent) : QObject(parent)
{
    m_configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QString appRoot = QDir(m_configRoot).filePath(QStringLiteral("river-control-center"));
    m_settingsPath = QDir(appRoot).filePath(QStringLiteral("keybindings.json"));
    m_generatedPath = QDir(appRoot).filePath(QStringLiteral("generated"));
    m_bindings = defaultBindings();
    QString error;
    if (!load(&error)) setStatus(error, true);
    m_savedBindings = m_bindings;
}

QStringList KeybindingsController::bindingModes(const QVariantMap &item) const
{
    return item.value(QStringLiteral("mode")).toString() == QLatin1String("both")
        ? QStringList{QStringLiteral("normal"), QStringLiteral("locked")}
        : QStringList{item.value(QStringLiteral("mode")).toString()};
}

QString KeybindingsController::canonicalChord(const QVariantMap &item, const QString &mode) const
{
    QStringList modifiers = item.value(QStringLiteral("modifiers")).toString().split(QLatin1Char('+'), Qt::SkipEmptyParts);
    modifiers.removeAll(QStringLiteral("None"));
    for (QString &modifier : modifiers) modifier = modifier.trimmed().toLower();
    std::sort(modifiers.begin(), modifiers.end());
    const QString nameSpace = item.value(QStringLiteral("category")).toString() == QLatin1String("mouse")
        ? QStringLiteral("pointer") : QStringLiteral("key");
    return QStringLiteral("%1|%2|%3|%4").arg(nameSpace, mode, modifiers.join(QLatin1Char('+')),
                                               item.value(QStringLiteral("key")).toString().toLower());
}

QString KeybindingsController::conflictSummary() const
{
    QHash<QString, QString> seen;
    QStringList conflicts;
    for (const QVariant &entry : m_bindings) {
        const QVariantMap item = entry.toMap();
        for (const QString &mode : bindingModes(item)) {
            const QString chord = canonicalChord(item, mode);
            if (seen.contains(chord))
                conflicts << QStringLiteral("%1 conflicts with %2 (%3)")
                    .arg(item.value(QStringLiteral("label")).toString(), seen.value(chord), shownChord(item));
            else
                seen.insert(chord, item.value(QStringLiteral("label")).toString());
        }
    }
    return conflicts.join(QLatin1Char('\n'));
}

QVariantList KeybindingsController::bindings() const
{
    QVariantList result;
    const QString needle = m_searchText.trimmed();
    const QVariantList defaults = defaultBindings();
    for (qsizetype i = 0; i < m_bindings.size(); ++i) {
        QVariantMap item = m_bindings.at(i).toMap();
        if (m_categoryFilter != QLatin1String("all")
            && item.value(QStringLiteral("category")).toString() != m_categoryFilter) continue;
        const QString haystack = item.value(QStringLiteral("label")).toString() + QLatin1Char(' ')
            + shownChord(item) + QLatin1Char(' ') + item.value(QStringLiteral("command")).toString();
        if (!needle.isEmpty() && !haystack.contains(needle, Qt::CaseInsensitive)) continue;
        item.insert(QStringLiteral("sourceIndex"), i);
        item.insert(QStringLiteral("chord"), shownChord(item));
        const QString id = item.value(QStringLiteral("id")).toString();
        bool canReset = false;
        for (const QVariant &defaultEntry : defaults)
            if (defaultEntry.toMap().value(QStringLiteral("id")).toString() == id) { canReset = true; break; }
        item.insert(QStringLiteral("canReset"), canReset);
        result.append(item);
    }
    return result;
}

void KeybindingsController::setSearchText(const QString &value)
{
    if (m_searchText == value) return;
    m_searchText = value;
    emit filterChanged(); emit bindingsChanged();
}

void KeybindingsController::setCategoryFilter(const QString &value)
{
    if (m_categoryFilter == value) return;
    m_categoryFilter = value;
    emit filterChanged(); emit bindingsChanged();
}

void KeybindingsController::setDirty(bool value)
{
    if (m_dirty == value) return;
    m_dirty = value; emit dirtyChanged();
}

void KeybindingsController::setStatus(const QString &message, bool error)
{
    m_statusMessage = message; m_statusError = error; emit statusChanged();
}

void KeybindingsController::addBinding(const QString &label, const QString &category,
                                       const QString &mode, const QString &modifiers,
                                       const QString &key, const QString &command)
{
    m_bindings.append(binding(QUuid::createUuid().toString(QUuid::WithoutBraces), label.trimmed(),
                              category, mode, modifiers.trimmed(), key.trimmed(), command.trimmed()));
    setDirty(true); emit bindingsChanged();
}

void KeybindingsController::updateBinding(int index, const QString &label, const QString &category,
                                          const QString &mode, const QString &modifiers,
                                          const QString &key, const QString &command)
{
    if (index < 0 || index >= m_bindings.size()) return;
    const QString id = m_bindings.at(index).toMap().value(QStringLiteral("id")).toString();
    m_bindings[index] = binding(id, label.trimmed(), category, mode, modifiers.trimmed(),
                                key.trimmed(), command.trimmed());
    setDirty(true); emit bindingsChanged();
}

void KeybindingsController::removeBinding(int index)
{
    if (index < 0 || index >= m_bindings.size()) return;
    m_bindings.removeAt(index); setDirty(true); emit bindingsChanged();
}

void KeybindingsController::resetBinding(int index)
{
    if (index < 0 || index >= m_bindings.size()) return;
    const QString id = m_bindings.at(index).toMap().value(QStringLiteral("id")).toString();
    for (const QVariant &entry : defaultBindings()) {
        if (entry.toMap().value(QStringLiteral("id")).toString() == id) {
            m_bindings[index] = entry; setDirty(true); emit bindingsChanged(); return;
        }
    }
}

QString KeybindingsController::validationError() const
{
    const QString conflict = conflictSummary();
    if (!conflict.isEmpty()) return QStringLiteral("Key conflict: %1").arg(conflict.section(QLatin1Char('\n'), 0, 0));
    for (qsizetype i = 0; i < m_bindings.size(); ++i) {
        const QVariantMap item = m_bindings.at(i).toMap();
        const QString category = item.value(QStringLiteral("category")).toString();
        const QString mode = item.value(QStringLiteral("mode")).toString();
        const QString modifiers = item.value(QStringLiteral("modifiers")).toString();
        const QString key = item.value(QStringLiteral("key")).toString();
        const QString command = item.value(QStringLiteral("command")).toString();
        if (item.value(QStringLiteral("label")).toString().isEmpty() || key.isEmpty() || command.isEmpty())
            return QStringLiteral("Binding %1 needs a name, key and command.").arg(i + 1);
        if (!QStringList{QStringLiteral("keyboard"), QStringLiteral("mouse"), QStringLiteral("media")}.contains(category))
            return QStringLiteral("Binding %1 has an invalid category.").arg(i + 1);
        if (!QStringList{QStringLiteral("normal"), QStringLiteral("locked"), QStringLiteral("both")}.contains(mode))
            return QStringLiteral("Binding %1 has an invalid River mode.").arg(i + 1);
        if (category == QLatin1String("mouse") && mode != QLatin1String("normal"))
            return QStringLiteral("Mouse binding %1 must use normal mode.").arg(i + 1);
        if (modifiers.contains(QRegularExpression(QStringLiteral("[\\s;`$]")))
            || key.contains(QRegularExpression(QStringLiteral("[\\s;`$]")))
            || command.contains(QLatin1Char('\n')))
            return QStringLiteral("Binding %1 contains unsupported characters.").arg(i + 1);
    }
    return {};
}

bool KeybindingsController::load(QString *error)
{
    QFile file(m_settingsPath);
    if (!file.exists()) { setDirty(true); return true; }
    if (!file.open(QIODevice::ReadOnly)) { if (error) *error = file.errorString(); return false; }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (!document.isObject() || parseError.error != QJsonParseError::NoError) {
        if (error) *error = QStringLiteral("Invalid keybindings JSON: %1").arg(parseError.errorString());
        return false;
    }
    const QJsonArray entries = document.object().value(QStringLiteral("bindings")).toArray();
    m_bindings.clear();
    for (const QJsonValue &value : entries) {
        const QJsonObject o = value.toObject();
        m_bindings.append(binding(o.value(QStringLiteral("id")).toString(), o.value(QStringLiteral("label")).toString(),
                                  o.value(QStringLiteral("category")).toString(), o.value(QStringLiteral("mode")).toString(),
                                  o.value(QStringLiteral("modifiers")).toString(), o.value(QStringLiteral("key")).toString(),
                                  o.value(QStringLiteral("command")).toString()));
    }
    setDirty(false); emit bindingsChanged(); return true;
}

bool KeybindingsController::writeFile(const QString &path, const QByteArray &contents, QString *error) const
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(contents) != contents.size() || !file.commit()) {
        if (error) *error = QStringLiteral("Could not write %1: %2").arg(path, file.errorString());
        return false;
    }
    return true;
}

bool KeybindingsController::save(QString *error)
{
    QJsonArray entries;
    for (const QVariant &entry : m_bindings) entries.append(QJsonObject::fromVariantMap(entry.toMap()));
    return writeFile(m_settingsPath, QJsonDocument(QJsonObject{{QStringLiteral("schemaVersion"), 1},
                                                                {QStringLiteral("bindings"), entries}})
                                         .toJson(QJsonDocument::Indented), error);
}

QString KeybindingsController::printableMap() const
{
    QString result = QStringLiteral("# River Classic keybindings\n\nGenerated by River Control Center.\n\n");
    const QStringList categories = {QStringLiteral("keyboard"), QStringLiteral("mouse"), QStringLiteral("media")};
    for (const QString &category : categories) {
        result += QStringLiteral("## %1\n\n| Shortcut | Action | River command | Mode |\n|---|---|---|---|\n")
            .arg(category == QLatin1String("keyboard") ? QStringLiteral("Keyboard")
                 : category == QLatin1String("mouse") ? QStringLiteral("Mouse") : QStringLiteral("Media keys"));
        for (const QVariant &entry : m_bindings) {
            const QVariantMap item = entry.toMap();
            if (item.value(QStringLiteral("category")).toString() != category) continue;
            auto escape = [](QString text) { text.replace(QLatin1Char('|'), QStringLiteral("\\|")); return text; };
            result += QStringLiteral("| `%1` | %2 | `%3` | %4 |\n")
                .arg(escape(shownChord(item)), escape(item.value(QStringLiteral("label")).toString()),
                     escape(item.value(QStringLiteral("command")).toString()),
                     item.value(QStringLiteral("mode")).toString());
        }
        result += QLatin1Char('\n');
    }
    result += QStringLiteral("## Tags (managed by the Windows page)\n\n"
                             "- `Super+1…N`: show tag\n- `Super+Shift+1…N`: move window to tag\n"
                             "- `Super+Control+1…N`: toggle displayed tag\n"
                             "- `Super+Shift+Control+1…N`: toggle the window on a tag\n"
                             "- `Super+0`: show all configured tags\n- `Super+Shift+0`: put the window on all configured tags\n");
    return result;
}

bool KeybindingsController::generate(QString *error)
{
    QString script = QStringLiteral("#!/bin/sh\n# Generated by River Control Center. Do not edit directly.\n\n");
    for (const QVariant &entry : m_bindings) {
        const QVariantMap item = entry.toMap();
        const QString verb = item.value(QStringLiteral("category")).toString() == QLatin1String("mouse")
            ? QStringLiteral("map-pointer") : QStringLiteral("map");
        for (const QString &mode : bindingModes(item)) {
            script += QStringLiteral("riverctl %1 %2 %3 %4 %5\n")
                .arg(verb, mode, item.value(QStringLiteral("modifiers")).toString(),
                     item.value(QStringLiteral("key")).toString(), item.value(QStringLiteral("command")).toString());
        }
    }
    script += QStringLiteral(
        "\n# Tag bindings use the count configured on the Windows page.\n"
        "RCC_TAG_COUNT=9\n"
        "rcc_windows=\"${XDG_CONFIG_HOME:-$HOME/.config}/river-control-center/generated/window-management.conf\"\n"
        "[ ! -r \"$rcc_windows\" ] || . \"$rcc_windows\"\n"
        "for rcc_tag in $(seq 1 \"$RCC_TAG_COUNT\"); do\n"
        "    rcc_mask=$((1 << (rcc_tag - 1)))\n"
        "    riverctl map normal Super \"$rcc_tag\" set-focused-tags \"$rcc_mask\"\n"
        "    riverctl map normal Super+Shift \"$rcc_tag\" set-view-tags \"$rcc_mask\"\n"
        "    riverctl map normal Super+Control \"$rcc_tag\" toggle-focused-tags \"$rcc_mask\"\n"
        "    riverctl map normal Super+Shift+Control \"$rcc_tag\" toggle-view-tags \"$rcc_mask\"\n"
        "done\n"
        "rcc_all_tags=$(((1 << RCC_TAG_COUNT) - 1))\n"
        "riverctl map normal Super 0 set-focused-tags \"$rcc_all_tags\"\n"
        "riverctl map normal Super+Shift 0 set-view-tags \"$rcc_all_tags\"\n"
        "unset rcc_windows rcc_tag rcc_mask rcc_all_tags\n");
    return writeFile(QDir(m_generatedPath).filePath(QStringLiteral("keybindings.sh")), script.toUtf8(), error)
        && writeFile(QDir(m_generatedPath).filePath(QStringLiteral("keybindings.md")), printableMap().toUtf8(), error);
}

bool KeybindingsController::regenerate(QString *error)
{
    const QString invalid = validationError();
    if (!invalid.isEmpty()) { if (error) *error = invalid; return false; }
    if (!QFileInfo::exists(m_settingsPath) && !save(error)) return false;
    return generate(error);
}

bool KeybindingsController::applyLive()
{
    auto applyOne = [this](const QVariantMap &item, bool remove) {
        const QString verb = item.value(QStringLiteral("category")).toString() == QLatin1String("mouse")
            ? (remove ? QStringLiteral("unmap-pointer") : QStringLiteral("map-pointer"))
            : (remove ? QStringLiteral("unmap") : QStringLiteral("map"));
        bool ok = true;
        for (const QString &mode : bindingModes(item)) {
            QStringList args{verb, mode, item.value(QStringLiteral("modifiers")).toString(),
                             item.value(QStringLiteral("key")).toString()};
            if (!remove) args += QProcess::splitCommand(item.value(QStringLiteral("command")).toString());
            QProcess process; process.start(QStringLiteral("riverctl"), args);
            ok = process.waitForStarted(1500) && process.waitForFinished(1500)
                && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && ok;
        }
        return ok;
    };
    bool ok = true;
    for (const QVariant &entry : m_savedBindings) ok = applyOne(entry.toMap(), true) && ok;
    for (const QVariant &entry : m_bindings) ok = applyOne(entry.toMap(), false) && ok;
    return ok;
}

bool KeybindingsController::saveAndApply()
{
    const QString invalid = validationError();
    if (!invalid.isEmpty()) { setStatus(invalid, true); return false; }
    QString error;
    if (!save(&error) || !generate(&error)) { setStatus(error, true); return false; }
    bool applied = true;
    if (qEnvironmentVariable("XDG_CURRENT_DESKTOP").contains(QStringLiteral("river"), Qt::CaseInsensitive)
        && !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) applied = applyLive();
    m_savedBindings = m_bindings; setDirty(false);
    setStatus(applied ? QStringLiteral("Keybindings saved and applied.")
                      : QStringLiteral("Saved, but River rejected part of the live update."), !applied);
    return applied;
}

void KeybindingsController::reload()
{
    m_bindings = defaultBindings();
    QString error;
    if (!load(&error)) { setStatus(error, true); return; }
    m_savedBindings = m_bindings; emit bindingsChanged(); setStatus(QStringLiteral("Discarded unsaved keybinding changes."));
}

bool KeybindingsController::exportPrintableMap()
{
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) documents = QDir::homePath();
    const QString path = QDir(documents).filePath(QStringLiteral("River-Classic-Keybindings.md"));
    QString error;
    if (!writeFile(path, printableMap().toUtf8(), &error)) { setStatus(error, true); return false; }
    setStatus(QStringLiteral("Printable keybinding map exported to %1").arg(path));
    return true;
}
