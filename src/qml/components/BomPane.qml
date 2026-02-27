pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var app
    required property var themeColors

    property var slotAscending: []
    property var customRatios: []
    property int minColumnWidth: 120
    property int widthBucket: Math.max(320, Math.round(contentFrame.width / 16) * 16)

    function openColumnConfig(slot, anchorItem) {
        columnConfigPopup.slot = slot
        const margin = 8
        const point = anchorItem.mapToItem(root, 0, anchorItem.height)
        const maxX = Math.max(margin, root.width - columnConfigPopup.width - margin)
        const preferredX = point.x - 8
        columnConfigPopup.x = Math.max(margin, Math.min(maxX, preferredX))

        const preferredY = point.y + 6
        const maxY = Math.max(margin, root.height - columnConfigPopup.height - margin)
        if (preferredY <= maxY) {
            columnConfigPopup.y = preferredY
        } else {
            const topY = point.y - columnConfigPopup.height - 10
            columnConfigPopup.y = Math.max(margin, Math.min(maxY, topY))
        }
        columnConfigPopup.open()
    }

    function ensureSortState() {
        const count = root.app.bomModel.visibleSlotCount()
        while (slotAscending.length < count) slotAscending.push(true)
        while (slotAscending.length > count) slotAscending.pop()
    }

    function slotWeight(slot) {
        const name = root.app.bomModel.visibleHeaderAt(slot)
        if (!name || name.length === 0) return 1.0

        if (name.includes("备注") || name.includes("描述") || name.includes("规格") || name.includes("型号")) return 1.8
        if (name.includes("名称") || name.includes("物料")) return 1.5
        if (name.includes("位号") || name.includes("Ref")) return 1.3
        if (name.includes("料号") || name.includes("编号") || name.includes("Part")) return 1.25
        if (name.includes("封装")) return 1.2
        if (name.includes("数量") || name.includes("Qty")) return 0.9
        if (name.includes("单价") || name.includes("价格") || name.includes("金额")) return 1.0

        return Math.min(1.8, Math.max(0.9, name.length * 0.16))
    }

    function layoutKey() {
        const count = root.app.bomModel.visibleSlotCount()
        const names = []
        for (let index = 0; index < count; ++index) {
            names.push(root.app.bomModel.visibleHeaderAt(index))
        }
        return "w" + widthBucket + "|" + names.join("||")
    }

    function ensureCustomRatios() {
        const count = root.app.bomModel.visibleSlotCount()
        let changed = false
        const next = customRatios.slice(0, count)
        for (let index = 0; index < count; ++index) {
            const value = Number(next[index])
            if (!(value > 0.01)) {
                next[index] = slotWeight(index)
                changed = true
            }
        }
        if (changed || next.length !== customRatios.length) {
            customRatios = next
        }
    }

    function restoreCustomRatios() {
        const count = root.app.bomModel.visibleSlotCount()
        const saved = root.app.loadBomWidthRatios(layoutKey())
        if (saved && saved.length === count) {
            customRatios = saved
        } else {
            customRatios = []
        }
        ensureCustomRatios()
        tableView.forceLayout()
        header.forceLayout()
    }

    function persistCustomRatios() {
        ensureCustomRatios()
        root.app.saveBomWidthRatios(layoutKey(), customRatios)
    }

    function setSlotRatio(slot, ratio) {
        ensureCustomRatios()
        const next = customRatios.slice()
        next[slot] = Math.max(0.4, Math.min(3.0, ratio))
        customRatios = next
        persistCustomRatios()
        tableView.forceLayout()
        header.forceLayout()
    }

    function slotRatio(slot) {
        ensureCustomRatios()
        const value = Number(customRatios[slot])
        return value > 0.01 ? value : slotWeight(slot)
    }

    function slotWidth(slot) {
        const count = Math.max(1, root.app.bomModel.visibleSlotCount())
        const total = Math.max(420, contentFrame.width)
        const spacingTotal = (count - 1) * tableView.columnSpacing
        const available = Math.max(0, total - spacingTotal)

        const minTotal = count * root.minColumnWidth
        if (available <= minTotal) {
            return Math.max(80, Math.floor(available / count))
        }

        let ratioSum = 0.0
        for (let index = 0; index < count; ++index) {
            ratioSum += root.slotRatio(index)
        }
        if (ratioSum <= 0.0001) {
            return Math.floor(available / count)
        }

        const extra = available - minTotal
        const weighted = root.minColumnWidth + extra * (root.slotRatio(slot) / ratioSum)
        return Math.floor(weighted)
    }

    function toggleSort(slot) {
        ensureSortState()
        root.app.bomModel.sortByVisibleColumn(slot, slotAscending[slot])
        slotAscending[slot] = !slotAscending[slot]
    }

    Component.onCompleted: {
        ensureSortState()
        restoreCustomRatios()
    }

    onWidthBucketChanged: restoreCustomRatios()

    Connections {
        target: root.app.bomModel
        function onModelReset() {
            root.ensureSortState()
            root.restoreCustomRatios()
        }
        function onHeaderDataChanged() {
            root.restoreCustomRatios()
        }
    }

    Popup {
        id: columnConfigPopup
        property int slot: -1
        width: 300
        modal: false
        focus: true
        padding: 10
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 12
            color: root.themeColors.card
            border.color: root.themeColors.border
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            ButtonGroup {
                id: fieldChoiceGroup
                exclusive: true
            }

            Label {
                Layout.fillWidth: true
                text: "列设置"
                color: root.themeColors.text
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                AppButton {
                    text: "删除"
                    themeColors: root.themeColors
                    Layout.fillWidth: true
                    enabled: root.app.bomModel.visibleSlotCount() > 1
                    onClicked: {
                        root.app.bomModel.removeVisibleSlot(columnConfigPopup.slot)
                        root.ensureSortState()
                        root.restoreCustomRatios()
                    }
                }
                AppButton {
                    text: "左侧新建"
                    themeColors: root.themeColors
                    Layout.fillWidth: true
                    onClicked: {
                        root.app.bomModel.insertVisibleSlot(columnConfigPopup.slot)
                        root.ensureSortState()
                        root.restoreCustomRatios()
                    }
                }
                AppButton {
                    text: "右侧新建"
                    themeColors: root.themeColors
                    Layout.fillWidth: true
                    onClicked: {
                        root.app.bomModel.insertVisibleSlot(columnConfigPopup.slot + 1)
                        root.ensureSortState()
                        root.restoreCustomRatios()
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: "字段选择"
                color: root.themeColors.muted
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                clip: true

                Column {
                    width: parent.width
                    spacing: 6

                    Repeater {
                        model: root.app.bomModel.availableHeaders()
                        delegate: Rectangle {
                            id: fieldOption
                            required property string modelData
                            width: parent.width
                            height: 32
                            radius: 10
                            color: root.themeColors.subtle
                            border.color: root.themeColors.border

                            RadioButton {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                ButtonGroup.group: fieldChoiceGroup
                                text: fieldOption.modelData
                                checked: fieldOption.modelData === root.app.bomModel.visibleHeaderAt(columnConfigPopup.slot)
                                onClicked: {
                                    root.app.bomModel.setVisibleHeaderAt(columnConfigPopup.slot, fieldOption.modelData)
                                    root.restoreCustomRatios()
                                }
                            }
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: "自定义宽度"
                color: root.themeColors.muted
            }

            Slider {
                Layout.fillWidth: true
                from: 0.4
                to: 3.0
                stepSize: 0.05
                value: root.slotRatio(columnConfigPopup.slot)
                onValueChanged: {
                    if (pressed) {
                        root.setSlotRatio(columnConfigPopup.slot, value)
                    }
                }
                onPressedChanged: {
                    if (!pressed) {
                        root.setSlotRatio(columnConfigPopup.slot, value)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "比例 " + root.slotRatio(columnConfigPopup.slot).toFixed(2)
                    color: root.themeColors.text
                }

                Item { Layout.fillWidth: true }

                AppButton {
                    text: "重置"
                    themeColors: root.themeColors
                    onClicked: {
                        root.setSlotRatio(columnConfigPopup.slot, root.slotWeight(columnConfigPopup.slot))
                    }
                }
            }
        }
    }

    Rectangle {
        id: contentFrame
        anchors.fill: parent
        radius: 14
        color: root.themeColors.card
        border.width: 1
        border.color: root.themeColors.border
        antialiasing: true
        clip: true

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: 12
            color: root.themeColors.card
            antialiasing: true
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

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
                        border.color: "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 6

                            Label {
                                Layout.fillWidth: true
                                text: headerCell.display
                                color: root.themeColors.text
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.bold: true

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        root.openColumnConfig(headerCell.column, headerCell)
                                    }
                                }
                            }

                            Item {
                                implicitWidth: 16
                                implicitHeight: 16

                                Image {
                                    anchors.centerIn: parent
                                    width: 14
                                    height: 14
                                    fillMode: Image.PreserveAspectFit
                                    source: {
                                        const dark = root.app.theme.currentThemeName === "Dark"
                                        if (root.slotAscending[headerCell.column]) {
                                            return dark ? "qrc:/assets/up-dark.png" : "qrc:/assets/up-light.png"
                                        }
                                        return dark ? "qrc:/assets/down-dark.png" : "qrc:/assets/down-light.png"
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.toggleSort(headerCell.column)
                                }
                            }
                        }
                    }
                }

                TableView {
                    id: tableView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 0
                    Layout.rightMargin: 0
                    Layout.bottomMargin: 0
                    clip: true
                    model: root.app.bomModel
                    boundsBehavior: Flickable.StopAtBounds
                    interactive: false
                    rowSpacing: 0
                    columnSpacing: 0
                    columnWidthProvider: function(column) { return root.slotWidth(column) }

                    delegate: Rectangle {
                        id: cell
                        required property int row
                        required property int column
                        required property string display
                        implicitHeight: 34
                        color: cell.row % 2 === 0 ? root.themeColors.card : root.themeColors.subtle
                        border.width: 0

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            text: cell.display === undefined ? "" : cell.display
                            color: root.themeColors.text
                        }
                    }
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 14
            border.width: 1
            border.color: root.themeColors.border
            antialiasing: true
        }
    }
}
