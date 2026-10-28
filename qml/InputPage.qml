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
        Controls.Label { text: parent.label; color: settingsController.text; Layout.fillWidth: true }
        Controls.SpinBox {
            Layout.minimumWidth: 120
            Layout.maximumWidth: 120
            from: parent.minimum
            to: parent.maximum
            value: parent.currentValue
            editable: true
            onValueModified: parent.edited(value)
        }
        Controls.Label { text: parent.suffix; color: settingsController.mutedText; Layout.minimumWidth: 90; Layout.maximumWidth: 90 }
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
            Kirigami.Heading { text: qsTr("Input"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("Configure River’s touchpad and keyboard behavior, plus tag-switching touchpad gestures.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Touchpad")
            description: qsTr("These options are applied immediately to every input device matching *Touchpad.")

            GridLayout {
                Layout.fillWidth: true
                columns: width > 560 ? 2 : 1
                columnSpacing: Kirigami.Units.largeSpacing * 2

                Controls.Switch {
                    text: qsTr("Tap to click")
                    checked: settingsController.touchpadTap
                    onToggled: settingsController.touchpadTap = checked
                }
                Controls.Switch {
                    text: qsTr("Natural scrolling")
                    checked: settingsController.touchpadNaturalScroll
                    onToggled: settingsController.touchpadNaturalScroll = checked
                }
                Controls.Switch {
                    text: qsTr("Disable while typing")
                    checked: settingsController.touchpadDisableWhileTyping
                    onToggled: settingsController.touchpadDisableWhileTyping = checked
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Click method"); color: settingsController.text; Layout.fillWidth: true }
                Controls.ComboBox {
                    Layout.minimumWidth: 260; Layout.maximumWidth: 260
                    model: [qsTr("Clickfinger"), qsTr("Button areas"), qsTr("Disabled")]
                    currentIndex: settingsController.touchpadClickMethod === "button-areas" ? 1
                                  : settingsController.touchpadClickMethod === "none" ? 2 : 0
                    onActivated: settingsController.touchpadClickMethod =
                                     currentIndex === 1 ? "button-areas"
                                   : currentIndex === 2 ? "none" : "clickfinger"
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Pointer acceleration"); color: settingsController.text; Layout.fillWidth: true }
                Controls.Slider {
                    id: accelerationSlider
                    from: -1.0
                    to: 1.0
                    stepSize: 0.05
                    value: settingsController.touchpadPointerAccel
                    Layout.preferredWidth: 260
                    onMoved: settingsController.touchpadPointerAccel = value
                }
                Controls.Label {
                    text: accelerationSlider.value.toFixed(2)
                    color: settingsController.mutedText
                    Layout.preferredWidth: 42
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Keyboard")
            description: qsTr("English (US) remains enabled because the lock screen deliberately switches to it before accepting a password.")

            NumberSetting {
                label: qsTr("Repeat rate")
                currentValue: settingsController.keyboardRepeatRate
                minimum: 1
                maximum: 100
                suffix: qsTr("per second")
                onEdited: value => settingsController.keyboardRepeatRate = value
            }
            NumberSetting {
                label: qsTr("Repeat delay")
                currentValue: settingsController.keyboardRepeatDelay
                minimum: 100
                maximum: 2000
                suffix: qsTr("ms")
                onEdited: value => settingsController.keyboardRepeatDelay = value
            }

            Controls.Label { text: qsTr("Enabled layouts"); color: settingsController.text; font.bold: true }
            RowLayout {
                Layout.fillWidth: true
                Controls.CheckBox { text: qsTr("English (US)"); checked: true; enabled: false }
                Controls.CheckBox {
                    text: qsTr("Sinhala (phonetic)")
                    checked: settingsController.sinhalaLayoutEnabled
                    onToggled: settingsController.sinhalaLayoutEnabled = checked
                }
                Item { Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Compose key"); color: settingsController.text; Layout.fillWidth: true }
                Controls.ComboBox {
                    id: composeBox
                    Layout.minimumWidth: 260; Layout.maximumWidth: 260
                    textRole: "label"
                    valueRole: "value"
                    model: [
                        { label: qsTr("Right Ctrl"), value: "compose:rctrl" },
                        { label: qsTr("Left Ctrl"), value: "compose:lctrl" },
                        { label: qsTr("Right Alt"), value: "compose:ralt" },
                        { label: qsTr("Left Alt"), value: "compose:lalt" },
                        { label: qsTr("Menu"), value: "compose:menu" },
                        { label: qsTr("Caps Lock"), value: "compose:caps" },
                        { label: qsTr("Disabled"), value: "" }
                    ]
                    currentIndex: Math.max(0, indexOfValue(settingsController.composeKey))
                    onActivated: settingsController.composeKey = currentValue
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Touchpad gestures")
            description: qsTr("Fusuma is restarted only when an input setting changes. The current threshold, debounce interval and device filter are preserved.")

            NumberSetting {
                label: qsTr("Swipe finger count")
                currentValue: settingsController.gestureFingerCount
                minimum: 3
                maximum: 5
                suffix: qsTr("fingers")
                onEdited: value => settingsController.gestureFingerCount = value
            }
            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Tag direction"); color: settingsController.text; Layout.fillWidth: true }
                Controls.ComboBox {
                    Layout.minimumWidth: 300; Layout.maximumWidth: 300
                    model: [qsTr("Swipe left → next tag"), qsTr("Swipe right → next tag")]
                    currentIndex: settingsController.gestureDirection === "right-next" ? 1 : 0
                    onActivated: settingsController.gestureDirection = currentIndex === 1 ? "right-next" : "left-next"
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Fusuma device"); color: settingsController.text; Layout.preferredWidth: 150 }
                Controls.TextField {
                    Layout.fillWidth: true
                    text: settingsController.gestureDeviceName
                    selectByMouse: true
                    onEditingFinished: settingsController.gestureDeviceName = text
                }
            }
        }

        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
