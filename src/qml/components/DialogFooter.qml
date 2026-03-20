pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

Item {
    id: root
    required property var themeColors
    required property string cancelText
    required property string okText
    property bool okAccent: true
    signal cancelled()
    signal accepted()

    implicitHeight: row.implicitHeight
    implicitWidth: row.implicitWidth
    width: parent ? parent.width : implicitWidth

    RowLayout {
        id: row
        anchors.fill: parent

        Item { Layout.fillWidth: true }

        AppButton {
            themeColors: root.themeColors
            text: root.cancelText
            onClicked: root.cancelled()
        }

        AppButton {
            themeColors: root.themeColors
            text: root.okText
            accent: root.okAccent
            onClicked: root.accepted()
        }
    }
}
