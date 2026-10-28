import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth
    property int sideMargin: Kirigami.Units.largeSpacing * 2
    property int ownerColumnWidth: 150
    property int stateColumnWidth: 110
    property int actionColumnWidth: 96
    property int columnGap: Kirigami.Units.largeSpacing

    Timer {
        interval: 5000
        running: true
        repeat: true
        onTriggered: settingsController.refreshSessionStatus()
    }

    ColumnLayout {
        width: Math.min(page.availableWidth, 1180)
        x: Math.max(0, (page.availableWidth - width) / 2)
        spacing: Kirigami.Units.largeSpacing
        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: page.sideMargin
            Layout.rightMargin: page.sideMargin
            Kirigami.Heading { text: qsTr("Session & Startup"); level: 1; color: settingsController.text }
            RowLayout {
                Controls.Label { text: qsTr("Current session:"); color: settingsController.mutedText }
                Controls.Label { text: settingsController.currentSessionOwner; color: settingsController.inRiverSession ? settingsController.success : settingsController.accent; font.bold: true }
                Item { Layout.fillWidth: true }
                Controls.Button { text: qsTr("Refresh status"); icon.name: "view-refresh"; onClicked: settingsController.refreshSessionStatus() }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("River-only autostart")
            description: qsTr("These switches affect only ~/.config/river/init. They do not enable systemd services or change Plasma startup. Autostart changes take effect at the next River login; Restart only refreshes a component now.")
            GridLayout {
                Layout.fillWidth: true; columns: 2; columnSpacing: Kirigami.Units.largeSpacing; rowSpacing: Kirigami.Units.smallSpacing
                Controls.Switch { text: qsTr("Waybar"); checked: settingsController.autostartWaybar; onToggled: settingsController.autostartWaybar = checked }
                Controls.Switch { text: qsTr("Mako notifications"); checked: settingsController.autostartMako; onToggled: settingsController.autostartMako = checked }
                Controls.Switch { text: qsTr("Kanshi output profiles"); checked: settingsController.autostartKanshi; onToggled: settingsController.autostartKanshi = checked }
                Controls.Switch { text: qsTr("Fusuma gestures"); checked: settingsController.autostartFusuma; onToggled: settingsController.autostartFusuma = checked }
                Controls.Switch { text: qsTr("swayidle lock and suspend"); checked: settingsController.autostartSwayidle; onToggled: settingsController.autostartSwayidle = checked }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("River components")
            description: qsTr("Process counts are checked directly. Two or more instances are marked as duplicates.")
            Item {
                Layout.fillWidth: true
                implicitHeight: 20
                Controls.Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("Component"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { anchors.right: stateHeading.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.ownerColumnWidth; text: qsTr("Session owner"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { id: stateHeading; anchors.right: actionHeading.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.stateColumnWidth; text: qsTr("Status"); color: settingsController.mutedText; font.bold: true }
                Item { id: actionHeading; anchors.right: parent.right; width: page.actionColumnWidth }
            }
            Repeater {
                model: settingsController.sessionComponents
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 52
                    radius: settingsController.interfaceRadius
                    color: settingsController.secondarySurface
                    border.width: modelData.duplicate ? 2 : 1
                    border.color: modelData.duplicate ? settingsController.urgent : settingsController.borderUnfocused
                    Item {
                        id: componentRow
                        anchors.fill: parent; anchors.margins: Kirigami.Units.smallSpacing
                        Controls.Button {
                            id: restartButton
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            width: page.actionColumnWidth
                            text: qsTr("Restart"); enabled: settingsController.inRiverSession
                            onClicked: settingsController.restartSessionComponent(modelData.key)
                        }
                        Controls.Label {
                            id: componentState
                            anchors.right: restartButton.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            width: page.stateColumnWidth
                            text: modelData.state
                            color: modelData.duplicate ? settingsController.urgent : modelData.count > 0 ? settingsController.success : settingsController.mutedText
                        }
                        Controls.Label {
                            id: componentOwner
                            anchors.right: componentState.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            width: page.ownerColumnWidth
                            text: modelData.owner; color: settingsController.highlight
                        }
                        Column {
                            anchors.left: parent.left; anchors.right: componentOwner.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            spacing: 1
                            Controls.Label { width: parent.width; text: modelData.name; color: settingsController.text; font.bold: true; elide: Text.ElideRight }
                            Controls.Label { width: parent.width; text: modelData.detail; color: modelData.duplicate ? settingsController.urgent : settingsController.mutedText; elide: Text.ElideRight }
                        }
                    }
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("River graphical-session target")
            description: qsTr("river-session.target belongs only to River and pulls in graphical-session.target. Plasma keeps its own session target.")
            Item {
                Layout.fillWidth: true
                implicitHeight: 30
                Controls.Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("river-session.target"); color: settingsController.text; font.bold: true }
                Controls.Label { id: targetState; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; width: page.stateColumnWidth; text: settingsController.riverSessionTargetStatus; color: settingsController.riverSessionTargetStatus === "active" ? settingsController.success : settingsController.mutedText }
                Controls.Label { anchors.right: targetState.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.ownerColumnWidth; text: qsTr("River only"); color: settingsController.highlight }
            }
        }

        SettingCard {
            Layout.fillWidth: true; Layout.leftMargin: page.sideMargin; Layout.rightMargin: page.sideMargin
            title: qsTr("Desktop portals")
            description: settingsController.portalConfigurationStatus
            Item {
                Layout.fillWidth: true; implicitHeight: 20
                Controls.Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("Portal component"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { anchors.right: portalStateHeading.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.ownerColumnWidth; text: qsTr("Session owner"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { id: portalStateHeading; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; width: page.stateColumnWidth; text: qsTr("Status"); color: settingsController.mutedText; font.bold: true }
            }
            Repeater {
                model: settingsController.portalBackends
                delegate: Item {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 42
                    Controls.Label { id: portalState; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; width: page.stateColumnWidth; text: modelData.state; color: modelData.active ? settingsController.success : settingsController.mutedText }
                    Controls.Label { id: portalOwner; anchors.right: portalState.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.ownerColumnWidth; text: modelData.owner; color: settingsController.highlight }
                    Column {
                        anchors.left: parent.left; anchors.right: portalOwner.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                        spacing: 1
                        Controls.Label { width: parent.width; text: modelData.name; color: settingsController.text; font.bold: true; elide: Text.ElideRight }
                        Controls.Label { width: parent.width; text: modelData.detail; color: settingsController.mutedText; elide: Text.ElideRight }
                    }
                }
            }
            Controls.Label {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: qsTr("Portals are shared infrastructure and remain D-Bus/systemd activated. The control center never starts them manually.")
                color: settingsController.mutedText
            }
        }
        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
