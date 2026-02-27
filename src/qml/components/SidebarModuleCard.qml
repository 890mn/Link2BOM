pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property string title
    required property var themeColors
    required property bool darkTheme
    property bool collapsed: false
    property int minModuleHeight: 46
    property int normalHeight: 170
    property bool draggable: false
    signal startDrag(real globalY)
    signal dragging(real globalY)
    signal endDrag()

    property real expandedHeight: Math.max(normalHeight, headerBar.implicitHeight + contentHost.childrenRect.height + 20)

    radius: 12
    color: root.themeColors.subtle
    border.color: root.themeColors.border
    implicitWidth: 300
    implicitHeight: root.collapsed ? minModuleHeight : expandedHeight
    height: root.collapsed ? minModuleHeight : expandedHeight

    Rectangle {
        id: headerBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        implicitHeight: 30
        radius: 10
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 6

            Item {
                Layout.preferredWidth: 2
            }

            Label {
                text: root.title
                color: root.themeColors.text
                font.bold: true
                font.pixelSize: 14
                Layout.fillWidth: true
            }

            Item {
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22

                Image {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    fillMode: Image.PreserveAspectFit
                    source: {
                        if (root.collapsed) {
                            return root.darkTheme ? "qrc:/assets/down-dark.png" : "qrc:/assets/down-light.png"
                        }
                        return root.darkTheme ? "qrc:/assets/up-dark.png" : "qrc:/assets/up-light.png"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.collapsed = !root.collapsed
                }
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

