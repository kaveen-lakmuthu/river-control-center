import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    component NumberSetting: RowLayout {
        required property string label
        required property int currentValue
        required property int minimum
        required property int maximum
        property string suffix: ""
        signal edited(int value)

        Layout.fillWidth: true
        Controls.Label {
            text: parent.label
            color: settingsController.text
            Layout.fillWidth: true
        }
        Controls.SpinBox {
            from: parent.minimum
            to: parent.maximum
            value: parent.currentValue
            editable: true
            onValueModified: parent.edited(value)
        }
        Controls.Label {
            visible: parent.suffix.length > 0
            text: parent.suffix
            color: settingsController.mutedText
        }
    }

    ColumnLayout {
        width: page.availableWidth
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Kirigami.Heading { text: qsTr("Bar & Panels"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("Configure the persistent Waybar layout and the shared GTK popup geometry.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Bar dimensions")
            description: qsTr("Waybar reloads immediately when these settings are applied.")

            NumberSetting {
                label: qsTr("Height")
                currentValue: settingsController.barHeight
                minimum: 20
                maximum: 64
                suffix: qsTr("px")
                onEdited: value => settingsController.barHeight = value
            }
            NumberSetting {
                label: qsTr("Font size")
                currentValue: settingsController.barFontSize
                minimum: 8
                maximum: 24
                suffix: qsTr("px")
                onEdited: value => settingsController.barFontSize = value
            }
            NumberSetting {
                label: qsTr("Module padding")
                currentValue: settingsController.modulePadding
                minimum: 0
                maximum: 24
                suffix: qsTr("px")
                onEdited: value => settingsController.modulePadding = value
            }
            NumberSetting {
                label: qsTr("Tag padding")
                currentValue: settingsController.tagPadding
                minimum: 0
                maximum: 24
                suffix: qsTr("px")
                onEdited: value => settingsController.tagPadding = value
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Visible modules")
            description: qsTr("Modules retain their current logical grouping and order.")

            GridLayout {
                Layout.fillWidth: true
                columns: width > 560 ? 2 : 1
                columnSpacing: Kirigami.Units.largeSpacing * 2

                Controls.Switch { text: qsTr("River tags"); checked: settingsController.showTags; onToggled: settingsController.showTags = checked }
                Controls.Switch { text: qsTr("Window title"); checked: settingsController.showWindowTitle; onToggled: settingsController.showWindowTitle = checked }
                Controls.Switch { text: qsTr("Clock"); checked: settingsController.showClock; onToggled: settingsController.showClock = checked }
                Controls.Switch { text: qsTr("Volume"); checked: settingsController.showVolume; onToggled: settingsController.showVolume = checked }
                Controls.Switch { text: qsTr("Battery"); checked: settingsController.showBattery; onToggled: settingsController.showBattery = checked }
                Controls.Switch { text: qsTr("Keyboard layout"); checked: settingsController.showKeyboardLayout; onToggled: settingsController.showKeyboardLayout = checked }
                Controls.Switch { text: qsTr("Clipboard"); checked: settingsController.showClipboard; onToggled: settingsController.showClipboard = checked }
                Controls.Switch { text: qsTr("Floating-window controls"); checked: settingsController.showWindowControls; onToggled: settingsController.showWindowControls = checked }
                Controls.Switch { text: qsTr("System tray"); checked: settingsController.showTray; onToggled: settingsController.showTray = checked }
                Controls.Switch { text: qsTr("SYS menu"); checked: settingsController.showSystemMenu; onToggled: settingsController.showSystemMenu = checked }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Clock and battery")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Clock format"); color: settingsController.text; Layout.preferredWidth: 150 }
                Controls.TextField {
                    Layout.fillWidth: true
                    text: settingsController.clockFormat
                    selectByMouse: true
                    onEditingFinished: settingsController.clockFormat = text
                }
            }
            Controls.Label {
                text: qsTr("Uses Waybar’s strftime syntax. Including %S enables one-second updates.")
                color: settingsController.mutedText
                font.pixelSize: 11
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            NumberSetting {
                label: qsTr("Battery warning")
                currentValue: settingsController.batteryWarning
                minimum: 2
                maximum: 80
                suffix: "%"
                onEdited: value => settingsController.batteryWarning = value
            }
            NumberSetting {
                label: qsTr("Battery critical")
                currentValue: settingsController.batteryCritical
                minimum: 1
                maximum: 50
                suffix: "%"
                onEdited: value => settingsController.batteryCritical = value
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Popup panels")
            description: qsTr("Margins and padding affect all custom Waybar popups. Width and height apply to clipboard and notification history.")

            NumberSetting {
                label: qsTr("History panel width")
                currentValue: settingsController.panelWidth
                minimum: 300
                maximum: 900
                suffix: qsTr("px")
                onEdited: value => settingsController.panelWidth = value
            }
            NumberSetting {
                label: qsTr("History panel height")
                currentValue: settingsController.panelHeight
                minimum: 240
                maximum: 900
                suffix: qsTr("px")
                onEdited: value => settingsController.panelHeight = value
            }
            NumberSetting {
                label: qsTr("Content padding")
                currentValue: settingsController.panelPadding
                minimum: 4
                maximum: 32
                suffix: qsTr("px")
                onEdited: value => settingsController.panelPadding = value
            }
            NumberSetting {
                label: qsTr("Distance below bar")
                currentValue: settingsController.panelTopMargin
                minimum: 0
                maximum: 48
                suffix: qsTr("px")
                onEdited: value => settingsController.panelTopMargin = value
            }
            NumberSetting {
                label: qsTr("Distance from right edge")
                currentValue: settingsController.panelRightMargin
                minimum: 0
                maximum: 48
                suffix: qsTr("px")
                onEdited: value => settingsController.panelRightMargin = value
            }
            NumberSetting {
                label: qsTr("Clipboard rows shown")
                currentValue: settingsController.clipboardHistoryLimit
                minimum: 5
                maximum: 200
                onEdited: value => settingsController.clipboardHistoryLimit = value
            }
            NumberSetting {
                label: qsTr("Stored notifications")
                currentValue: settingsController.notificationHistoryLimit
                minimum: 5
                maximum: 100
                onEdited: value => settingsController.notificationHistoryLimit = value
            }
        }

        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
