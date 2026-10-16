import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: card
    property alias title: heading.text
    property alias description: descriptionLabel.text
    default property alias content: contentColumn.data

    color: settingsController.base
    radius: 10
    border.width: 1
    border.color: settingsController.secondarySurface
    implicitHeight: contentColumn.implicitHeight + Kirigami.Units.largeSpacing * 2

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            id: heading
            level: 3
            color: settingsController.text
            Layout.fillWidth: true
        }

        Controls.Label {
            id: descriptionLabel
            visible: text.length > 0
            color: settingsController.mutedText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }
    }
}

