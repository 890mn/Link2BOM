pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

Button {
    id: control
    required property var themeColors
    property bool accent: false
    property int cornerRadius: 10

    implicitHeight: 34
    implicitWidth: Math.max(88, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: 14
    rightPadding: 14

    contentItem: Text {
        text: control.text
        color: control.enabled
            ? (control.accent ? "#FFFFFF" : control.themeColors.text)
            : control.themeColors.muted
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 13
        font.bold: control.accent
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: control.cornerRadius
        border.width: 1
        border.color: control.accent
            ? Qt.darker(control.themeColors.primary, 1.15)
            : (control.hovered ? control.themeColors.primary : control.themeColors.border)
        color: {
            if (!control.enabled) return Qt.rgba(control.themeColors.subtle.r, control.themeColors.subtle.g, control.themeColors.subtle.b, 0.55)
            if (control.accent) {
                return control.down
                    ? Qt.darker(control.themeColors.primary, 1.15)
                    : (control.hovered ? Qt.lighter(control.themeColors.primary, 1.08) : control.themeColors.primary)
            }
            if (control.down) return Qt.rgba(control.themeColors.primary.r, control.themeColors.primary.g, control.themeColors.primary.b, 0.20)
            if (control.hovered) return Qt.rgba(control.themeColors.primary.r, control.themeColors.primary.g, control.themeColors.primary.b, 0.12)
            return control.themeColors.subtle
        }
    }
}

