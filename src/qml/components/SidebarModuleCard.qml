pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property string title
    required property var themeColors
    property bool collapsed: false
    property int minModuleHeight: 46
    property int normalHeight: 170
    property bool draggable: false
    signal startDrag(real globalY)
    signal dragging(real globalY)
    signal endDrag()

    radius: 10
    color: themeColors.subtle
    border.color: themeColors.border
    implicitWidth: 300
    implicitHeight: collapsed ? minModuleHeight : normalHeight
    height: collapsed ? minModuleHeight : normalHeight

    Rectangle {
        id: headerBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        height: 30
        radius: 6
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 6

            Label {
                text: root.title
                color: root.themeColors.text
                font.bold: true
                Layout.fillWidth: true
            }

            ToolButton {
                text: root.collapsed ? "▸" : "▾"
                font.pixelSize: 13
                implicitWidth: 28
                implicitHeight: 24
                onClicked: root.collapsed = !root.collapsed
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.draggable
            cursorShape: Qt.OpenHandCursor
            onPressed: function(mouse) {
                cursorShape = Qt.ClosedHandCursor
                root.startDrag(mapToGlobal(mouse.x, mouse.y).y)
            }
            onPositionChanged: function(mouse) {
                if (pressed) {
                    root.dragging(mapToGlobal(mouse.x, mouse.y).y)
                }
            }
            onReleased: {
                cursorShape = Qt.OpenHandCursor
                root.endDrag()
            }
        }
    }

    default property alias moduleContent: contentHost.data

    Item {
        id: contentHost
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: headerBar.y + headerBar.height + 8
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8
        visible: !root.collapsed
    }
}
