import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: Math.min(page.availableWidth, 1180)
        x: Math.max(0, (page.availableWidth - width) / 2)
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Kirigami.Heading { text: qsTr("About"); level: 1; color: settingsController.text }
            Controls.Label { text: qsTr("A control center built specifically for this River Classic configuration."); color: settingsController.mutedText }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Version 0.1")
            description: qsTr("C++20 backend with a Qt 6, QML and Kirigami interface.")

            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: settingsController.text
                text: qsTr("This version manages the shared palette, River and rivertile appearance, Waybar and panel layout, power and idle behavior, and input devices. Settings are saved atomically and the previous ten versions are retained as backups.")
            }
            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WrapAnywhere
                color: settingsController.mutedText
                text: qsTr("Settings: %1").arg(settingsController.settingsPath)
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Planned pages")
            description: qsTr("The backend is structured so these can be added without putting system logic into QML.")

            Controls.Label {
                color: settingsController.text
                text: qsTr("Displays  •  Startup  •  Diagnostics")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        Item { Layout.fillHeight: true; implicitHeight: Kirigami.Units.largeSpacing }
    }
}

