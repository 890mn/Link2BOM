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
        color: root.themeColors.card
        border.color: root.themeColors.border

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
                    color: root.themeColors.subtle
                    border.color: root.themeColors.border

                    property real dragStartX: 0
                    property real dragStartWidth: 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 10
                        spacing: 4

                        AppToolButton {
                            themeColors: root.themeColors
                            text: "R"
                            onClicked: root.cycleHeader(headerCell.column)
                        }

                        Label {
                            Layout.fillWidth: true
                            text: headerCell.display
                            color: root.themeColors.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            font.bold: true
                        }

                        AppToolButton {
                            themeColors: root.themeColors
                            text: root.slotAscending[headerCell.column] ? "^" : "v"
                            onClicked: root.toggleSort(headerCell.column)
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
                                headerCell.dragStartWidth = root.slotWidth(headerCell.column)
                            }
                            onPositionChanged: function(mouse) {
                                if (pressed) {
                                    const delta = mouse.x - headerCell.dragStartX
                                    root.setSlotWidth(headerCell.column, headerCell.dragStartWidth + delta)
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
                    id: cell
                    required property int row
                    required property string display
                    implicitHeight: 34
                    color: cell.row % 2 === 0 ? root.themeColors.card : root.themeColors.subtle
                    border.color: root.themeColors.border

                    Text {
                        anchors.fill: parent
                        anchors.margins: 8
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: cell.display === undefined ? "" : cell.display
                        color: root.themeColors.text
                    }
                }
            }
        }
    }
}
