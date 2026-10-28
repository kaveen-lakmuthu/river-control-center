import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth
    property int sideMargin: Kirigami.Units.largeSpacing * 2
    property var paletteEntries: [
        { key: "base", label: qsTr("Base"), description: qsTr("Desktop and terminal background") },
        { key: "surface", label: qsTr("Surface"), description: qsTr("Bar, notifications and panels") },
        { key: "secondarySurface", label: qsTr("Secondary surface"), description: qsTr("Selections and raised controls") },
        { key: "text", label: qsTr("Primary text"), description: qsTr("Normal foreground text") },
        { key: "mutedText", label: qsTr("Muted text"), description: qsTr("Secondary labels and inactive content") },
        { key: "accent", label: qsTr("Primary accent"), description: qsTr("Focused borders, tags and prompts") },
        { key: "secondaryAccent", label: qsTr("Secondary accent"), description: qsTr("Secondary interface accents") },
        { key: "highlight", label: qsTr("Cool highlight"), description: qsTr("Links and information") },
        { key: "urgent", label: qsTr("Urgent"), description: qsTr("Errors and critical states") },
        { key: "success", label: qsTr("Success"), description: qsTr("Charging and successful states") },
        { key: "borderUnfocused", label: qsTr("Unfocused border"), description: qsTr("Inactive River windows") }
    ]

    FileDialog {
        id: wallpaperDialog
        title: qsTr("Choose wallpaper")
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.webp *.bmp)")]
        onAccepted: settingsController.wallpaperPath = selectedFile.toString().replace(/^file:\/\//, "")
    }
    FileDialog {
        id: importDialog
        title: qsTr("Import River theme")
        nameFilters: [qsTr("River themes (*.json *.river-theme.json)")]
        onAccepted: settingsController.importTheme(selectedFile)
    }

    ColumnLayout {
        width: Math.min(page.availableWidth, 1180)
        x: Math.max(0, (page.availableWidth - width) / 2)
        spacing: Kirigami.Units.largeSpacing
        Item { implicitHeight: Kirigami.Units.smallSpacing }
        ColumnLayout {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            Kirigami.Heading { text: qsTr("Appearance"); level: 1; color: settingsController.text }
            Controls.Label { text: qsTr("Theme River and its desktop tools without silently changing Plasma."); color: settingsController.mutedText }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Themes")
            description: qsTr("Start from a preset or save the complete appearance as a reusable custom theme.")
            RowLayout {
                Layout.fillWidth: true
                Repeater { model: ["Númenor", "Ocean", "Graphite"]
                    delegate: Controls.Button { required property string modelData; Layout.fillWidth: true; text: modelData; onClicked: settingsController.applyPreset(modelData) }
                }
            }
            Rectangle { Layout.fillWidth: true; height: 1; color: settingsController.secondarySurface }
            RowLayout {
                Layout.fillWidth: true
                Controls.ComboBox { id: savedTheme; Layout.preferredWidth: 220; model: settingsController.customThemes; displayText: currentIndex >= 0 ? currentText : qsTr("No saved themes") }
                Controls.TextField { id: themeName; Layout.fillWidth: true; placeholderText: qsTr("Theme name") }
                Controls.Button { text: qsTr("Save current"); onClicked: settingsController.saveCustomTheme(themeName.text) }
                Controls.Button { text: qsTr("Load"); enabled: savedTheme.currentIndex >= 0; onClicked: settingsController.applyCustomTheme(savedTheme.currentText) }
            }
            RowLayout {
                Layout.fillWidth: true
                Controls.Button { text: qsTr("Duplicate as"); enabled: savedTheme.currentIndex >= 0 && themeName.text.length > 0; onClicked: settingsController.duplicateTheme(savedTheme.currentText, themeName.text) }
                Controls.Button { text: qsTr("Rename to"); enabled: savedTheme.currentIndex >= 0 && themeName.text.length > 0; onClicked: settingsController.renameTheme(savedTheme.currentText, themeName.text) }
                Controls.Button { text: qsTr("Import…"); onClicked: importDialog.open() }
                Controls.Button { text: qsTr("Export"); enabled: savedTheme.currentIndex >= 0; onClicked: settingsController.exportTheme(savedTheme.currentText) }
                Item { Layout.fillWidth: true }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Desktop background")
            RowLayout {
                Layout.fillWidth: true
                Controls.ComboBox {
                    id: backgroundMode
                    model: [{ text: qsTr("Solid base colour"), value: "solid" }, { text: qsTr("Wallpaper"), value: "wallpaper" }]
                    textRole: "text"; valueRole: "value"
                    Component.onCompleted: currentIndex = settingsController.backgroundMode === "wallpaper" ? 1 : 0
                    onActivated: settingsController.backgroundMode = currentValue
                }
                Controls.TextField { Layout.fillWidth: true; enabled: backgroundMode.currentValue === "wallpaper"; text: settingsController.wallpaperPath; placeholderText: qsTr("Wallpaper image"); onEditingFinished: settingsController.wallpaperPath = text }
                Controls.Button { text: qsTr("Browse…"); enabled: backgroundMode.currentValue === "wallpaper"; onClicked: wallpaperDialog.open() }
            }
            Controls.Label { text: qsTr("Wallpaper uses swaybg's fill mode on every output."); color: settingsController.mutedText }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Fonts and geometry")
            description: qsTr("Font sizes are in points except Waybar, which uses pixels.")
            GridLayout {
                Layout.fillWidth: true; columns: 3; columnSpacing: Kirigami.Units.largeSpacing; rowSpacing: Kirigami.Units.smallSpacing
                Controls.Label { text: qsTr("Component"); color: settingsController.mutedText }
                Controls.Label { text: qsTr("Font family"); color: settingsController.mutedText }
                Controls.Label { text: qsTr("Size"); color: settingsController.mutedText }
                Controls.Label { text: qsTr("Waybar") }
                Controls.TextField { Layout.fillWidth: true; text: settingsController.waybarFontFamily; onEditingFinished: settingsController.waybarFontFamily = text }
                Controls.SpinBox { from: 8; to: 24; value: settingsController.barFontSize; onValueModified: settingsController.barFontSize = value }
                Controls.Label { text: qsTr("Notifications") }
                Controls.TextField { Layout.fillWidth: true; text: settingsController.makoFontFamily; onEditingFinished: settingsController.makoFontFamily = text }
                Controls.SpinBox { from: 7; to: 28; value: settingsController.makoFontSize; onValueModified: settingsController.makoFontSize = value }
                Controls.Label { text: qsTr("Launcher") }
                Controls.TextField { Layout.fillWidth: true; text: settingsController.fuzzelFontFamily; onEditingFinished: settingsController.fuzzelFontFamily = text }
                Controls.SpinBox { from: 7; to: 28; value: settingsController.fuzzelFontSize; onValueModified: settingsController.fuzzelFontSize = value }
                Controls.Label { text: qsTr("Terminal") }
                Controls.TextField { Layout.fillWidth: true; text: settingsController.footFontFamily; onEditingFinished: settingsController.footFontFamily = text }
                Controls.SpinBox { id: footSize; from: 60; to: 320; stepSize: 5; value: Math.round(settingsController.footFontSize * 10); textFromValue: function(value) { return (value / 10).toFixed(1) }; valueFromText: function(text) { return Math.round(Number(text) * 10) }; onValueModified: settingsController.footFontSize = value / 10 }
                Controls.Label { text: qsTr("Popup panels") }
                Controls.TextField { Layout.fillWidth: true; text: settingsController.panelFontFamily; onEditingFinished: settingsController.panelFontFamily = text }
                Controls.SpinBox { from: 7; to: 28; value: settingsController.panelFontSize; onValueModified: settingsController.panelFontSize = value }
            }
            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Corner radius") }
                Controls.SpinBox { from: 0; to: 30; value: settingsController.interfaceRadius; onValueModified: settingsController.interfaceRadius = value }
                Controls.Label { text: qsTr("Panel spacing") }
                Controls.SpinBox { from: 0; to: 30; value: settingsController.panelSpacing; onValueModified: settingsController.panelSpacing = value }
                Item { Layout.fillWidth: true }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Theme targets")
            description: qsTr("Turn off “Follow” to freeze a component at its last applied theme. River window colours and the background always follow this page.")
            Flow {
                Layout.fillWidth: true; spacing: Kirigami.Units.largeSpacing
                Controls.Switch { text: qsTr("Waybar follows"); checked: settingsController.themeWaybar; onToggled: settingsController.themeWaybar = checked }
                Controls.Switch { text: qsTr("Notifications follow"); checked: settingsController.themeMako; onToggled: settingsController.themeMako = checked }
                Controls.Switch { text: qsTr("Launcher follows"); checked: settingsController.themeFuzzel; onToggled: settingsController.themeFuzzel = checked }
                Controls.Switch { text: qsTr("Terminal follows"); checked: settingsController.themeFoot; onToggled: settingsController.themeFoot = checked }
                Controls.Switch { text: qsTr("Popup panels follow"); checked: settingsController.themePanels; onToggled: settingsController.themePanels = checked }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Component preview")
            description: qsTr("Preview each managed component independently before applying.")
            Controls.TabBar { id: previewTabs; Layout.fillWidth: true
                Controls.TabButton { text: qsTr("Waybar") }
                Controls.TabButton { text: qsTr("Notification") }
                Controls.TabButton { text: qsTr("Launcher") }
                Controls.TabButton { text: qsTr("Terminal") }
            }
            StackLayout {
                Layout.fillWidth: true; Layout.preferredHeight: 150; currentIndex: previewTabs.currentIndex
                Rectangle { color: settingsController.surface; radius: settingsController.interfaceRadius
                    RowLayout { anchors.fill: parent; anchors.margins: 12
                        Controls.Label { text: "1   2   3"; color: settingsController.accent; font.family: settingsController.waybarFontFamily; font.pixelSize: settingsController.barFontSize }
                        Item { Layout.fillWidth: true }
                        Controls.Label { text: "Sun 18 Oct  20:15"; color: settingsController.text; font.family: settingsController.waybarFontFamily; font.pixelSize: settingsController.barFontSize }
                        Item { Layout.fillWidth: true }
                        Controls.Label { text: "VOL 42%   BAT 86%   US"; color: settingsController.text; font.family: settingsController.waybarFontFamily; font.pixelSize: settingsController.barFontSize }
                    }
                }
                Rectangle { color: settingsController.base
                    Rectangle { width: Math.min(parent.width - 40, 420); height: 100; anchors.centerIn: parent; radius: settingsController.interfaceRadius; color: settingsController.surface; border.color: settingsController.accent; border.width: 2
                        Column { anchors.fill: parent; anchors.margins: 14; spacing: 7
                            Controls.Label { text: qsTr("Notification title"); color: settingsController.text; font.family: settingsController.makoFontFamily; font.pointSize: settingsController.makoFontSize; font.bold: true }
                            Controls.Label { text: qsTr("This is how notification text will appear."); color: settingsController.mutedText; font.family: settingsController.makoFontFamily; font.pointSize: settingsController.makoFontSize }
                        }
                    }
                }
                Rectangle { color: settingsController.surface; radius: settingsController.interfaceRadius; border.color: settingsController.accent; border.width: 2
                    Column { anchors.fill: parent; anchors.margins: 14; spacing: 7
                        Controls.Label { text: ">  Search applications…"; color: settingsController.text; font.family: settingsController.fuzzelFontFamily; font.pointSize: settingsController.fuzzelFontSize }
                        Rectangle { width: parent.width; height: 34; radius: Math.max(0, settingsController.interfaceRadius - 3); color: settingsController.secondarySurface
                            Controls.Label { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 10; text: "River Control Center"; color: settingsController.text; font.family: settingsController.fuzzelFontFamily; font.pointSize: settingsController.fuzzelFontSize }
                        }
                    }
                }
                Rectangle { color: settingsController.base; radius: settingsController.interfaceRadius; border.color: settingsController.borderUnfocused
                    Column { anchors.fill: parent; anchors.margins: 14; spacing: 7
                        Controls.Label { text: "kaveen@river ~/.config"; color: settingsController.success; font.family: settingsController.footFontFamily; font.pointSize: settingsController.footFontSize }
                        Controls.Label { text: "❯ river-control-center"; color: settingsController.text; font.family: settingsController.footFontFamily; font.pointSize: settingsController.footFontSize }
                        Controls.Label { text: "Settings ready."; color: settingsController.highlight; font.family: settingsController.footFontFamily; font.pointSize: settingsController.footFontSize }
                    }
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Contrast checks")
            description: settingsController.contrastWarnings.length === 0 ? qsTr("No readability problems found in the main colour combinations.") : qsTr("These combinations may be difficult to read:")
            Repeater { model: settingsController.contrastWarnings
                delegate: Controls.Label { required property string modelData; Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "⚠  " + modelData; color: settingsController.urgent }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Palette"); description: qsTr("Colours use hexadecimal notation such as #D6AE4A.")
            Repeater { model: page.paletteEntries
                delegate: ColorSettingRow { required property var modelData; Layout.fillWidth: true; label: modelData.label; colorKey: modelData.key; description: modelData.description }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("GTK and KDE application themes — system-wide")
            description: qsTr("Not changed by this page. GTK settings, kdeglobals, Qt styles and icon themes can also affect Plasma, so they remain separate from River component themes.")
            Controls.Label { text: qsTr("Your existing dark Breeze/Qt system configuration remains in charge of application themes."); color: settingsController.mutedText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
