import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: page
    clip: true
    contentWidth: availableWidth
    property int bindingNameWidth: Math.min(260, Math.max(150, availableWidth * 0.20))
    property int bindingChordWidth: Math.min(210, Math.max(130, availableWidth * 0.16))
    property int editWidth: 52
    property int resetWidth: 58
    property int removeWidth: 34
    property int columnGap: Kirigami.Units.smallSpacing

    function categoryIndex(value) { return Math.max(0, ["keyboard", "mouse", "media"].indexOf(value)) }
    function modeIndex(value) { return Math.max(0, ["normal", "locked", "both"].indexOf(value)) }

    Controls.Dialog {
        id: editor
        property int sourceIndex: -1
        parent: Controls.Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(620, parent.width - 48)
        modal: true
        title: sourceIndex < 0 ? qsTr("Add keybinding") : qsTr("Edit keybinding")
        standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

        function openNew(category) {
            sourceIndex = -1
            labelField.text = ""
            categoryBox.currentIndex = page.categoryIndex(category)
            modeBox.currentIndex = 0
            modifiersField.text = category === "media" ? "None" : "Super"
            keyField.text = ""
            commandField.text = ""
            open()
        }
        function openBinding(item) {
            sourceIndex = item.sourceIndex
            labelField.text = item.label
            categoryBox.currentIndex = page.categoryIndex(item.category)
            modeBox.currentIndex = page.modeIndex(item.mode)
            modifiersField.text = item.modifiers
            keyField.text = item.key
            commandField.text = item.command
            open()
        }
        onAccepted: {
            const category = ["keyboard", "mouse", "media"][categoryBox.currentIndex]
            const mode = category === "mouse" ? "normal" : ["normal", "locked", "both"][modeBox.currentIndex]
            if (sourceIndex < 0)
                keybindingsController.addBinding(labelField.text, category, mode, modifiersField.text, keyField.text, commandField.text)
            else
                keybindingsController.updateBinding(sourceIndex, labelField.text, category, mode, modifiersField.text, keyField.text, commandField.text)
        }

        ColumnLayout {
            width: parent.width
            spacing: Kirigami.Units.smallSpacing
            Controls.Label { text: qsTr("Name"); color: settingsController.text }
            Controls.TextField { id: labelField; Layout.fillWidth: true; placeholderText: qsTr("Open terminal") }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    Controls.Label { text: qsTr("Category"); color: settingsController.text }
                    Controls.ComboBox {
                        id: categoryBox
                        Layout.fillWidth: true
                        model: [qsTr("Keyboard"), qsTr("Mouse"), qsTr("Media")]
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Controls.Label { text: qsTr("River mode"); color: settingsController.text }
                    Controls.ComboBox {
                        id: modeBox
                        Layout.fillWidth: true
                        enabled: categoryBox.currentIndex !== 1
                        model: [qsTr("Normal"), qsTr("Locked"), qsTr("Normal and locked")]
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    Controls.Label { text: qsTr("Modifiers"); color: settingsController.text }
                    Controls.TextField { id: modifiersField; Layout.fillWidth: true; placeholderText: qsTr("Super+Shift or None") }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Controls.Label { text: qsTr("Key or button"); color: settingsController.text }
                    Controls.TextField { id: keyField; Layout.fillWidth: true; placeholderText: qsTr("Return, F, BTN_LEFT…") }
                }
            }
            Controls.Label { text: qsTr("River command"); color: settingsController.text }
            Controls.TextField {
                id: commandField
                Layout.fillWidth: true
                placeholderText: qsTr("spawn foot, close, focus-view next…")
            }
            Controls.Label {
                text: qsTr("For a shell command, use River’s spawn action, for example: spawn 'playerctl play-pause'")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }

    Controls.Dialog {
        id: removeDialog
        property int sourceIndex: -1
        parent: Controls.Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: qsTr("Remove keybinding?")
        standardButtons: Controls.Dialog.Yes | Controls.Dialog.No
        onAccepted: keybindingsController.removeBinding(sourceIndex)
        Controls.Label { text: qsTr("The shortcut will be unmapped when you apply the changes.") }
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
            Kirigami.Heading { text: qsTr("Keybindings"); level: 1; color: settingsController.text }
            Controls.Label {
                text: qsTr("Search, edit and apply River shortcuts without changing river/init. Number-tag bindings remain managed by the Windows page.")
                color: settingsController.mutedText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        SettingCard {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            title: qsTr("Bindings")
            description: qsTr("Commands are shown exactly as they are passed to riverctl. Apply validates conflicts before changing the live session.")

            RowLayout {
                Layout.fillWidth: true
                Controls.TextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search shortcuts, actions or commands…")
                    text: keybindingsController.searchText
                    onTextChanged: keybindingsController.searchText = text
                }
                Controls.ComboBox {
                    model: [qsTr("All"), qsTr("Keyboard"), qsTr("Mouse"), qsTr("Media")]
                    currentIndex: ["all", "keyboard", "mouse", "media"].indexOf(keybindingsController.categoryFilter)
                    onActivated: keybindingsController.categoryFilter = ["all", "keyboard", "mouse", "media"][currentIndex]
                }
                Controls.Button {
                    text: qsTr("Add")
                    icon.name: "list-add"
                    onClicked: editor.openNew(keybindingsController.categoryFilter === "all" ? "keyboard" : keybindingsController.categoryFilter)
                }
            }

            Controls.Label {
                visible: keybindingsController.conflictSummary.length > 0
                text: qsTr("Conflicts detected:\n") + keybindingsController.conflictSummary
                color: settingsController.urgent
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item {
                Layout.fillWidth: true
                implicitHeight: 20
                Controls.Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: page.bindingNameWidth; text: qsTr("Binding"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { anchors.left: parent.left; anchors.leftMargin: page.bindingNameWidth + page.columnGap; anchors.verticalCenter: parent.verticalCenter; width: page.bindingChordWidth; text: qsTr("Shortcut"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { anchors.left: parent.left; anchors.leftMargin: page.bindingNameWidth + page.bindingChordWidth + page.columnGap * 2; anchors.right: parent.right; anchors.rightMargin: page.editWidth + page.resetWidth + page.removeWidth + page.columnGap * 3; anchors.verticalCenter: parent.verticalCenter; text: qsTr("River command"); color: settingsController.mutedText; font.bold: true }
                Controls.Label { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; width: page.editWidth + page.resetWidth + page.removeWidth + page.columnGap * 2; text: qsTr("Actions"); color: settingsController.mutedText; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            }

            Repeater {
                model: keybindingsController.bindings
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 52
                    radius: 7
                    color: settingsController.secondarySurface
                    border.width: 1
                    border.color: settingsController.borderUnfocused

                    Item {
                        id: bindingRow
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.smallSpacing

                        Controls.ToolButton {
                            id: removeButton
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            width: page.removeWidth
                            text: "×"
                            Controls.ToolTip.visible: hovered
                            Controls.ToolTip.text: qsTr("Remove")
                            onClicked: { removeDialog.sourceIndex = modelData.sourceIndex; removeDialog.open() }
                        }
                        Controls.ToolButton {
                            id: resetButton
                            anchors.right: removeButton.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            width: page.resetWidth
                            text: qsTr("Reset")
                            enabled: modelData.canReset
                            onClicked: keybindingsController.resetBinding(modelData.sourceIndex)
                        }
                        Controls.ToolButton {
                            id: editButton
                            anchors.right: resetButton.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            width: page.editWidth
                            text: qsTr("Edit")
                            onClicked: editor.openBinding(modelData)
                        }
                        Column {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            width: page.bindingNameWidth
                            spacing: 1
                            Controls.Label {
                                width: parent.width
                                text: modelData.label
                                color: settingsController.text
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Controls.Label {
                                width: parent.width
                                text: modelData.category + " · " + modelData.mode
                                color: settingsController.mutedText
                                elide: Text.ElideRight
                            }
                        }
                        Controls.Label {
                            anchors.left: parent.left; anchors.leftMargin: page.bindingNameWidth + page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            width: page.bindingChordWidth
                            text: modelData.chord
                            color: settingsController.accent
                            font.family: "monospace"
                            elide: Text.ElideRight
                        }
                        Controls.Label {
                            anchors.left: parent.left; anchors.leftMargin: page.bindingNameWidth + page.bindingChordWidth + page.columnGap * 2
                            anchors.right: editButton.left; anchors.rightMargin: page.columnGap; anchors.verticalCenter: parent.verticalCenter
                            text: modelData.command
                            color: settingsController.highlight
                            font.family: "monospace"
                            elide: Text.ElideRight
                            Controls.ToolTip.visible: hovered
                            Controls.ToolTip.text: modelData.command
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
            Controls.Label {
                text: keybindingsController.statusMessage
                color: keybindingsController.statusError ? settingsController.urgent : settingsController.success
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Controls.Button { text: qsTr("Export printable map"); onClicked: keybindingsController.exportPrintableMap() }
            Controls.Button { text: qsTr("Discard"); enabled: keybindingsController.dirty; onClicked: keybindingsController.reload() }
            Controls.Button {
                text: qsTr("Apply keybindings")
                highlighted: true
                enabled: keybindingsController.dirty && keybindingsController.conflictSummary.length === 0
                onClicked: keybindingsController.saveAndApply()
            }
        }
        Item { implicitHeight: Kirigami.Units.largeSpacing }
    }
}
