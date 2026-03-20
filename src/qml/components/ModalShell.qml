pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

Popup {
    id: root
    required property color subtleColor
    required property color borderColor

    modal: true
    focus: true
    padding: 10
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: 12
        color: root.subtleColor
        border.color: root.borderColor
    }
}
