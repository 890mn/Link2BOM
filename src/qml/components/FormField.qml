pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property color textColor
    required property color mutedTextColor
    required property color primaryColor
    required property color cardColor
    required property color borderColor
    property string label: ""
    property string placeholderText: ""
    property alias text: field.text
    property int fieldHeight: 36

    implicitHeight: layout.implicitHeight

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: 6

        Label {
            visible: root.label.length > 0
            text: root.label
            color: root.textColor
        }

        TextField {
            id: field
            Layout.fillWidth: true
            placeholderText: root.placeholderText
            implicitHeight: root.fieldHeight
            color: root.textColor
            placeholderTextColor: root.mutedTextColor
            selectionColor: root.primaryColor
            selectedTextColor: "#FFFFFF"
            background: Rectangle {
                radius: 10
                color: root.cardColor
                border.color: root.borderColor
            }
        }
    }
}
