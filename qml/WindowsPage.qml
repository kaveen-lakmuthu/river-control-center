import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: page.availableWidth
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Kirigami.Heading { text: qsTr("Windows"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("River borders update immediately. Rivertile padding applies at the next River login.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Window borders")
            description: qsTr("The focused and unfocused colours come from the active palette.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Border width"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox {
                    from: 0
                    to: 12
                    value: settingsController.borderWidth
                    editable: true
                    onValueModified: settingsController.borderWidth = value
                }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Tiled layout spacing")
            description: qsTr("Zero keeps a single tiled window edge-to-edge below Waybar.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Between windows"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox {
                    from: 0
                    to: 32
                    value: settingsController.viewPadding
                    editable: true
                    onValueModified: settingsController.viewPadding = value
                }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
            }

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Around screen edges"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox {
                    from: 0
                    to: 32
                    value: settingsController.outerPadding
                    editable: true
                    onValueModified: settingsController.outerPadding = value
                }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
            }
        }

        Item { Layout.fillHeight: true; implicitHeight: Kirigami.Units.largeSpacing }
    }
}

