import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    component TimeoutSetting: RowLayout {
        required property string label
        required property int currentValue
        required property bool actionEnabled
        signal valueEdited(int value)
        signal enabledEdited(bool value)

        Layout.fillWidth: true

        Controls.Switch {
            text: parent.label
            checked: parent.actionEnabled
            Layout.fillWidth: true
            onToggled: parent.enabledEdited(checked)
        }
        Controls.SpinBox {
            Layout.minimumWidth: 120
            Layout.maximumWidth: 120
            from: 1
            to: 360
            value: parent.currentValue
            editable: true
            enabled: parent.actionEnabled
            onValueModified: parent.valueEdited(value)
        }
        Controls.Label {
            Layout.minimumWidth: 70
            Layout.maximumWidth: 70
            text: qsTr("min")
            color: settingsController.mutedText
            enabled: parent.actionEnabled
        }
    }

    component TimeSetting: RowLayout {
        required property string label
        required property string currentValue
        signal edited(string value)

        Layout.fillWidth: true
        Controls.Label {
            text: parent.label
            color: settingsController.text
            Layout.fillWidth: true
        }
        Controls.TextField {
            text: parent.currentValue
            placeholderText: "HH:MM"
            inputMethodHints: Qt.ImhTime
            selectByMouse: true
            Layout.minimumWidth: 120
            Layout.maximumWidth: 120
            validator: RegularExpressionValidator {
                regularExpression: /^([01][0-9]|2[0-3]):[0-5][0-9]$/
            }
            onEditingFinished: if (acceptableInput) parent.edited(text)
        }
    }

    ColumnLayout {
        width: Math.min(page.availableWidth, 1180)
        x: Math.max(0, (page.availableWidth - width) / 2)
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Kirigami.Heading { text: qsTr("Power & Idle"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("Configure River’s idle sequence, default power profile and fixed night-light schedule.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Idle actions")
            description: qsTr("Each timeout is measured from the last activity. Applying safely restarts only the swayidle process owned by this River session.")

            TimeoutSetting {
                label: qsTr("Lock screen")
                currentValue: settingsController.lockTimeoutMinutes
                actionEnabled: settingsController.lockOnIdle
                onValueEdited: value => settingsController.lockTimeoutMinutes = value
                onEnabledEdited: value => settingsController.lockOnIdle = value
            }
            TimeoutSetting {
                label: qsTr("Turn displays off")
                currentValue: settingsController.displayOffTimeoutMinutes
                actionEnabled: settingsController.displayOffOnIdle
                onValueEdited: value => settingsController.displayOffTimeoutMinutes = value
                onEnabledEdited: value => settingsController.displayOffOnIdle = value
            }
            TimeoutSetting {
                label: qsTr("Suspend on battery")
                currentValue: settingsController.batterySuspendTimeoutMinutes
                actionEnabled: settingsController.suspendOnBattery
                onValueEdited: value => settingsController.batterySuspendTimeoutMinutes = value
                onEnabledEdited: value => settingsController.suspendOnBattery = value
            }
            TimeoutSetting {
                label: qsTr("Suspend on AC power")
                currentValue: settingsController.acSuspendTimeoutMinutes
                actionEnabled: settingsController.suspendOnAc
                onValueEdited: value => settingsController.acSuspendTimeoutMinutes = value
                onEnabledEdited: value => settingsController.suspendOnAc = value
            }
            Controls.Switch {
                text: qsTr("Lock before suspend")
                checked: settingsController.lockBeforeSuspend
                onToggled: settingsController.lockBeforeSuspend = checked
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Power profile")
            description: qsTr("This profile is selected when River starts and whenever these settings are applied.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Default profile"); color: settingsController.text; Layout.fillWidth: true }
                Controls.ComboBox {
                    id: profileBox
                    Layout.minimumWidth: 260; Layout.maximumWidth: 260
                    model: ["power-saver", "balanced", "performance"]
                    currentIndex: Math.max(0, model.indexOf(settingsController.defaultPowerProfile))
                    onActivated: settingsController.defaultPowerProfile = currentText
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Night light schedule")
            description: qsTr("A fixed daily schedule is used; no location or sunset service is required.")

            TimeSetting {
                label: qsTr("Start warming")
                currentValue: settingsController.nightLightStart
                onEdited: value => settingsController.nightLightStart = value
            }
            TimeSetting {
                label: qsTr("Stop warming")
                currentValue: settingsController.nightLightStop
                onEdited: value => settingsController.nightLightStop = value
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Lid close policy")
            description: qsTr("Read-only systemd-logind policy. Change it through the system configuration, not this application.")

            RowLayout {
                Layout.fillWidth: true
                Kirigami.Icon { source: "computer-laptop"; implicitWidth: 22; implicitHeight: 22 }
                Controls.Label {
                    text: settingsController.lidClosePolicy
                    color: settingsController.text
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
