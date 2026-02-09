pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var app
    required property var themeColors

    property var columnWidths: [180, 180, 180, 180, 180, 180]
    property var slotAscending: [true, true, true, true, true, true]

    function ensureColumnState() {
        while (columnWidths.length < 6) columnWidths.push(180)
        while (slotAscending.length < 6) slotAscending.push(true)
    }

    function slotWidth(slot) {
        ensureColumnState()
        return columnWidths[slot]
    }

    function setSlotWidth(slot, widthValue) {
        ensureColumnState()
        columnWidths[slot] = Math.max(120, Math.min(440, widthValue))
        tableView.forceLayout()
        header.forceLayout()
    }

    function toggleSort(slot) {
        ensureColumnState()
        app.bomModel.sortByVisibleColumn(slot, slotAscending[slot])
        slotAscending[slot] = !slotAscending[slot]
    }

    function cycleHeader(slot) {
        const headers = app.bomModel.availableHeaders()
        if (!headers || headers.length === 0) return
        const current = app.bomModel.visibleHeaderAt(slot)
        const idx = headers.indexOf(current)
        const nextIdx = idx < 0 ? 0 : (idx + 1) % headers.length
        app.bomModel.setVisibleHeaderAt(slot, headers[nextIdx])
    }

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: themeColors.card
        border.color: themeColors.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 6
            spacing: 1

            HorizontalHeaderView {
                id: header
                Layout.fillWidth: true
                syncView: tableView
                clip: true
                columnWidthProvider: function(column) { return root.slotWidth(column) }

                delegate: Rectangle {
                    id: headerCell
                    required property int column
                    required property string display
                    implicitHeight: 40
                    color: themeColors.subtle
                    border.color: themeColors.border

                    property real dragStartX: 0
                    property real dragStartWidth: 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 10
                        spacing: 4

                        ToolButton {
                            text: "⇄"
                            font.pixelSize: 14
                            onClicked: root.cycleHeader(column)
                        }

                        Label {
                            Layout.fillWidth: true
                            text: display
                            color: themeColors.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            font.bold: true
                        }

                        ToolButton {
                            text: root.slotAscending[column] ? "↑" : "↓"
                            font.pixelSize: 14
                            onClicked: root.toggleSort(column)
                        }
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        width: 8
                        color: "transparent"

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            onPressed: function(mouse) {
                                headerCell.dragStartX = mouse.x
                                headerCell.dragStartWidth = root.slotWidth(column)
                            }
                            onPositionChanged: function(mouse) {
                                if (pressed) {
                                    const delta = mouse.x - headerCell.dragStartX
                                    root.setSlotWidth(column, headerCell.dragStartWidth + delta)
                                }
                            }
                        }
                    }
                }
            }

            TableView {
                id: tableView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: root.app.bomModel
                boundsBehavior: Flickable.StopAtBounds
                rowSpacing: 1
                columnSpacing: 1
                columnWidthProvider: function(column) { return root.slotWidth(column) }

                delegate: Rectangle {
                    required property int row
                    required property string display
                    implicitHeight: 34
                    color: row % 2 === 0 ? themeColors.card : themeColors.subtle
                    border.color: themeColors.border
                    Text {
                        anchors.fill: parent
                        anchors.margins: 8
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: display === undefined ? "" : display
                        color: themeColors.text
                    }
                }
            }
        }
    }
}
