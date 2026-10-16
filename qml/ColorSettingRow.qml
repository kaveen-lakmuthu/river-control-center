import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    id: root
    required property string label
    required property string colorKey
    property string description: ""
    property int updateRevision: settingsController.revision

    spacing: Kirigami.Units.largeSpacing

    onUpdateRevisionChanged: valueField.text = settingsController.colorValue(colorKey)

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 1

        Controls.Label {
            text: root.label
            color: settingsController.text
        }
        Controls.Label {
            visible: root.description.length > 0
            text: root.description
            color: settingsController.mutedText
            font.pixelSize: 11
        }
    }

    Controls.Button {
        id: swatch
        Layout.preferredWidth: 42
        Layout.preferredHeight: 32
        onClicked: colorDialog.open()
        contentItem: Rectangle {
            radius: 5
            color: settingsController.colorValue(root.colorKey)
            border.width: 1
            border.color: settingsController.mutedText
        }
    }

    Controls.TextField {
        id: valueField
        Layout.preferredWidth: 108
        text: settingsController.colorValue(root.colorKey)
        selectByMouse: true
        inputMethodHints: Qt.ImhUppercaseOnly
        onEditingFinished: {
            if (!settingsController.setColor(root.colorKey, text))
                text = settingsController.colorValue(root.colorKey)
        }
    }

    ColorDialog {
        id: colorDialog
        title: qsTr("Choose %1").arg(root.label)
        selectedColor: settingsController.colorValue(root.colorKey)
        onAccepted: settingsController.setColor(root.colorKey, selectedColor.toString())
    }
}

