import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    property var paletteEntries: [
        { key: "base", label: qsTr("Base"), description: qsTr("Desktop and terminal background") },
        { key: "surface", label: qsTr("Surface"), description: qsTr("Bar, notifications and panels") },
        { key: "secondarySurface", label: qsTr("Secondary surface"), description: qsTr("Selections and raised controls") },
        { key: "text", label: qsTr("Primary text"), description: qsTr("Normal foreground text") },
        { key: "mutedText", label: qsTr("Muted text"), description: qsTr("Secondary labels and inactive content") },
        { key: "accent", label: qsTr("Primary accent"), description: qsTr("Focused borders, tags and prompts") },
        { key: "secondaryAccent", label: qsTr("Secondary accent"), description: qsTr("Blue terminal and interface accents") },
        { key: "highlight", label: qsTr("Cool highlight"), description: qsTr("Links and informational states") },
        { key: "urgent", label: qsTr("Urgent"), description: qsTr("Errors and critical states") },
        { key: "success", label: qsTr("Success"), description: qsTr("Charging and successful states") },
        { key: "borderUnfocused", label: qsTr("Unfocused border"), description: qsTr("Inactive River windows") }
    ]

    ColumnLayout {
        width: page.availableWidth
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2

            Kirigami.Heading {
                text: qsTr("Appearance")
                level: 1
                color: settingsController.text
            }
            Controls.Label {
                text: qsTr("One semantic palette for River, Waybar, Mako, Foot and Fuzzel.")
                color: settingsController.mutedText
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Theme presets")
            description: qsTr("Load a starting palette, then adjust individual colours below.")

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing

                Repeater {
                    model: ["Númenor", "Ocean", "Graphite"]
                    delegate: Controls.Button {
                        required property string modelData
                        Layout.fillWidth: true
                        text: modelData
                        onClicked: settingsController.applyPreset(modelData)
                    }
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Desktop preview")

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 156
                radius: 8
                color: settingsController.base
                border.width: 2
                border.color: settingsController.accent

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 28
                    radius: 6
                    color: settingsController.surface

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 8
                        spacing: 4
                        Repeater {
                            model: 4
                            delegate: Rectangle {
                                width: 26
                                height: 20
                                radius: 3
                                color: index === 1 ? settingsController.accent : "transparent"
                                Controls.Label {
                                    anchors.centerIn: parent
                                    text: index + 1
                                    color: index === 1 ? settingsController.base : settingsController.mutedText
                                }
                            }
                        }
                    }
                    Controls.Label {
                        anchors.centerIn: parent
                        text: qsTr("Fri 10 Oct  20:42")
                        color: settingsController.text
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.68
                    height: 82
                    radius: 7
                    color: settingsController.surface
                    border.width: 1
                    border.color: settingsController.secondarySurface

                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        Controls.Label { text: qsTr("River Control Center"); color: settingsController.text }
                        Controls.Label { text: qsTr("A consistent desktop palette"); color: settingsController.mutedText }
                    }
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Palette")
            description: qsTr("Colours must use hexadecimal notation such as #D6AE4A.")

            Repeater {
                model: page.paletteEntries
                delegate: ColorSettingRow {
                    required property var modelData
                    Layout.fillWidth: true
                    label: modelData.label
                    colorKey: modelData.key
                    description: modelData.description
                }
            }
        }

        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}

