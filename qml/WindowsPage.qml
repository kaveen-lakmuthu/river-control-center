import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth

    function comboIndex(values, current) { return Math.max(0, values.indexOf(current)) }

    ColumnLayout {
        width: Math.min(page.availableWidth, 1180)
        x: Math.max(0, (page.availableWidth - width) / 2)
        spacing: Kirigami.Units.largeSpacing

        Item { implicitHeight: Kirigami.Units.smallSpacing }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Kirigami.Heading { text: qsTr("Windows"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("Configure River’s layout defaults, pointer focus, tags and rules for newly opened windows.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Layout")
            description: qsTr("Layout changes are sent to every connected output. The defaults are also restored at the next River login.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Main-window ratio"); color: settingsController.text; Layout.fillWidth: true }
                Controls.Slider {
                    id: ratioSlider
                    from: 0.10; to: 0.90; stepSize: 0.05
                    value: settingsController.mainWindowRatio
                    Layout.preferredWidth: 240
                    onMoved: settingsController.mainWindowRatio = value
                }
                Controls.Label {
                    text: Math.round(ratioSlider.value * 100) + "%"
                    color: settingsController.mutedText
                    Layout.preferredWidth: 42
                    horizontalAlignment: Text.AlignRight
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > 620 ? 4 : 2
                columnSpacing: Kirigami.Units.largeSpacing

                Controls.Label { text: qsTr("Main-window count"); color: settingsController.text }
                Controls.SpinBox {
                    from: 1; to: 16; editable: true
                    value: settingsController.mainWindowCount
                    onValueModified: settingsController.mainWindowCount = value
                }
                Controls.Label { text: qsTr("Main position"); color: settingsController.text }
                Controls.ComboBox {
                    model: [qsTr("Left"), qsTr("Right"), qsTr("Top"), qsTr("Bottom")]
                    currentIndex: page.comboIndex(["left", "right", "top", "bottom"], settingsController.mainPosition)
                    onActivated: settingsController.mainPosition = ["left", "right", "top", "bottom"][currentIndex]
                }

                Controls.Label { text: qsTr("Attach new windows"); color: settingsController.text }
                Controls.ComboBox {
                    model: [qsTr("Bottom of stack"), qsTr("Top of stack"), qsTr("Above focused"), qsTr("Below focused")]
                    currentIndex: page.comboIndex(["bottom", "top", "above", "below"], settingsController.defaultAttachMode)
                    onActivated: settingsController.defaultAttachMode = ["bottom", "top", "above", "below"][currentIndex]
                }
                Controls.Label { text: qsTr("Default mode"); color: settingsController.text }
                Controls.ComboBox {
                    model: [qsTr("Tiled"), qsTr("Floating")]
                    currentIndex: settingsController.defaultLayoutMode === "floating" ? 1 : 0
                    onActivated: settingsController.defaultLayoutMode = currentIndex === 1 ? "floating" : "tiled"
                }

                Controls.Label { text: qsTr("Pointer focus"); color: settingsController.text }
                Controls.ComboBox {
                    Layout.preferredWidth: 210
                    model: [qsTr("Mode-aware"), qsTr("Disabled"), qsTr("Normal"), qsTr("Always")]
                    currentIndex: page.comboIndex(["mode-aware", "disabled", "normal", "always"], settingsController.focusPolicy)
                    onActivated: settingsController.focusPolicy = ["mode-aware", "disabled", "normal", "always"][currentIndex]
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: qsTr("Mode-aware follows the pointer while tiled and disables it while floating.")
                }
                Controls.Label { text: qsTr("Number of tags"); color: settingsController.text }
                Controls.SpinBox {
                    from: 1; to: 9; editable: true
                    value: settingsController.tagCount
                    onValueModified: settingsController.tagCount = value
                }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Borders and spacing")
            description: qsTr("Zero outer padding lets a lone tiled window fill the usable area below Waybar.")

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                Controls.Label { text: qsTr("Border width"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox { from: 0; to: 12; value: settingsController.borderWidth; editable: true; onValueModified: settingsController.borderWidth = value }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
                Controls.Label { text: qsTr("Between windows"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox { from: 0; to: 32; value: settingsController.viewPadding; editable: true; onValueModified: settingsController.viewPadding = value }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
                Controls.Label { text: qsTr("Around screen edges"); color: settingsController.text; Layout.fillWidth: true }
                Controls.SpinBox { from: 0; to: 32; value: settingsController.outerPadding; editable: true; onValueModified: settingsController.outerPadding = value }
                Controls.Label { text: qsTr("px"); color: settingsController.mutedText }
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Window rules")
            description: qsTr("Application ID accepts River glob patterns. Title is a case-sensitive contains match. Float rules affect windows opened after Apply.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: qsTr("Application ID"); color: settingsController.mutedText; font.bold: true; Layout.fillWidth: true; Layout.preferredWidth: 220 }
                Controls.Label { text: qsTr("Title contains"); color: settingsController.mutedText; font.bold: true; Layout.fillWidth: true; Layout.preferredWidth: 220 }
                Controls.Label { text: qsTr("Action"); color: settingsController.mutedText; font.bold: true; Layout.minimumWidth: 170; Layout.maximumWidth: 170 }
                Item { Layout.minimumWidth: 52; Layout.maximumWidth: 52 }
                Item { Layout.minimumWidth: 38; Layout.maximumWidth: 38 }
            }

            Repeater {
                model: settingsController.windowRules
                delegate: RowLayout {
                    id: ruleRow
                    required property int index
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    function commit() {
                        settingsController.updateWindowRule(index, appField.text, titleField.text,
                                                            ["float", "no-float", "ssd", "csd"][actionBox.currentIndex])
                    }

                    Controls.TextField {
                        id: appField
                        Layout.fillWidth: true; Layout.preferredWidth: 220
                        text: ruleRow.modelData.appId
                        placeholderText: qsTr("e.g. firefox or org.kde.*")
                        selectByMouse: true
                        onEditingFinished: ruleRow.commit()
                    }
                    Controls.TextField {
                        id: titleField
                        Layout.fillWidth: true; Layout.preferredWidth: 220
                        text: ruleRow.modelData.titleContains
                        placeholderText: qsTr("optional")
                        selectByMouse: true
                        onEditingFinished: ruleRow.commit()
                    }
                    Controls.ComboBox {
                        id: actionBox
                        Layout.minimumWidth: 170; Layout.maximumWidth: 170
                        model: [qsTr("Float"), qsTr("Force tiled"), qsTr("Server decoration"), qsTr("Client decoration")]
                        currentIndex: page.comboIndex(["float", "no-float", "ssd", "csd"], ruleRow.modelData.action)
                        onActivated: ruleRow.commit()
                    }
                    Controls.Button {
                        Layout.minimumWidth: 52; Layout.maximumWidth: 52
                        text: qsTr("Test")
                        onClicked: { ruleRow.commit(); settingsController.testWindowRule(ruleRow.index) }
                    }
                    Controls.ToolButton {
                        Layout.minimumWidth: 38; Layout.maximumWidth: 38
                        text: "×"
                        Controls.ToolTip.visible: hovered
                        Controls.ToolTip.text: qsTr("Remove rule")
                        onClicked: settingsController.removeWindowRule(ruleRow.index)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Controls.Button { text: qsTr("Add rule"); icon.name: "list-add"; onClicked: settingsController.addWindowRule() }
                Item { Layout.fillWidth: true }
                Controls.Label { text: qsTr("Rules are evaluated when a window is created."); color: settingsController.mutedText }
            }
            Controls.Label {
                visible: settingsController.ruleTestResult.length > 0
                text: settingsController.ruleTestResult
                color: settingsController.ruleTestResult.startsWith("Matched") ? settingsController.success : settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Currently open windows")
            description: qsTr("Use these live application IDs and titles when constructing a rule. Refresh does not modify any window.")

            RowLayout {
                Layout.fillWidth: true
                Controls.Button { text: qsTr("Refresh list"); icon.name: "view-refresh"; onClicked: settingsController.refreshOpenWindows() }
                Controls.Label { text: qsTr("%1 window(s)").arg(settingsController.openWindows.length); color: settingsController.mutedText }
                Item { Layout.fillWidth: true }
            }
            Repeater {
                model: settingsController.openWindows
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: windowDetails.implicitHeight + Kirigami.Units.smallSpacing * 2
                    radius: 6
                    color: settingsController.secondarySurface
                    RowLayout {
                        id: windowDetails
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.smallSpacing
                        Controls.Label {
                            text: modelData.appId.length ? modelData.appId : qsTr("(no application ID)")
                            color: settingsController.highlight
                            font.family: "monospace"
                            Layout.minimumWidth: 260; Layout.maximumWidth: 260
                            elide: Text.ElideRight
                        }
                        Controls.Label {
                            text: modelData.title.length ? modelData.title : qsTr("(untitled)")
                            color: settingsController.text
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
