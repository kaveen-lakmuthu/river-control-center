import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ApplicationWindow {
    id: window
    width: 1120
    height: 740
    minimumWidth: 860
    minimumHeight: 600
    visible: true
    title: qsTr("River Control Center")

    palette.window: settingsController.surface
    palette.windowText: settingsController.text
    palette.base: settingsController.base
    palette.alternateBase: settingsController.secondarySurface
    palette.text: settingsController.text
    palette.button: settingsController.secondarySurface
    palette.buttonText: settingsController.text
    palette.highlight: settingsController.accent
    palette.highlightedText: settingsController.base
    palette.placeholderText: settingsController.mutedText

    property int currentPage: 0

    background: Rectangle { color: settingsController.surface }

    header: Controls.ToolBar {
        implicitHeight: 58
        background: Rectangle {
            color: settingsController.surface
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: settingsController.secondarySurface
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: qsTr("River Control Center")
                level: 2
                color: settingsController.text
            }

            Controls.Label {
                visible: settingsController.dirty
                text: qsTr("Unsaved changes")
                color: settingsController.accent
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                text: qsTr("Discard")
                enabled: settingsController.dirty
                onClicked: settingsController.reload()
            }

            Controls.Button {
                text: qsTr("Apply")
                highlighted: true
                enabled: settingsController.dirty
                onClicked: settingsController.saveAndApply()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Controls.Pane {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            padding: Kirigami.Units.largeSpacing

            background: Rectangle {
                color: settingsController.base
                Rectangle {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    width: 1
                    color: settingsController.secondarySurface
                }
            }

            Controls.ButtonGroup { id: navigationGroup }

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: qsTr("Settings")
                    level: 4
                    color: settingsController.mutedText
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                }

                Repeater {
                    model: [
                        { label: qsTr("Appearance"), icon: "preferences-desktop-theme", page: 0 },
                        { label: qsTr("Windows"), icon: "preferences-system-windows", page: 1 },
                        { label: qsTr("Bar & Panels"), icon: "view-list-icons", page: 2 },
                        { label: qsTr("About"), icon: "help-about", page: 3 }
                    ]

                    delegate: Controls.Button {
                        required property var modelData
                        Layout.fillWidth: true
                        text: modelData.label
                        icon.name: modelData.icon
                        checkable: true
                        checked: window.currentPage === modelData.page
                        autoExclusive: true
                        Controls.ButtonGroup.group: navigationGroup
                        onClicked: window.currentPage = modelData.page
                    }
                }

                Item { Layout.fillHeight: true }

                Controls.Label {
                    text: qsTr("River Classic")
                    color: settingsController.mutedText
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.margins: visible ? Kirigami.Units.largeSpacing : 0
                visible: settingsController.statusMessage.length > 0
                text: settingsController.statusMessage
                type: settingsController.statusError ? Kirigami.MessageType.Error : Kirigami.MessageType.Positive
                showCloseButton: true
            }

            StackLayout {
                currentIndex: window.currentPage
                Layout.fillWidth: true
                Layout.fillHeight: true

                AppearancePage {}
                WindowsPage {}
                BarPanelsPage {}
                AboutPage {}
            }
        }
    }
}
