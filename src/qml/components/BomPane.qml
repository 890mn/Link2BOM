pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var app
    required property var themeColors
    signal debugLog(string level, string message)

    property var slotAscending: []
    property var customRatios: []
    property int minColumnWidth: 120
    property int widthBucket: Math.max(320, Math.round(contentFrame.width / 16) * 16)
    property color ratioAccentColor: "#C9778F"
    property real popupMaxHeight: Math.max(260, root.height - 24)

    function openColumnConfig(slot, anchorItem) {
        if (slot < 0 || slot >= root.app.bomModel.visibleSlotCount()) {
            root.debugLog("WARNING", "Open column config rejected: invalid slot " + slot)
            return
        }
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
        root.debugLog("INFO", "Open column config for slot " + slot + " (" + root.app.bomModel.visibleHeaderAt(slot) + ")")
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
            root.debugLog("INFO", "Load custom ratios from settings, slots=" + count)
        } else {
            customRatios = []
            root.debugLog("WARNING", "No saved custom ratios matched current layout, reset to auto")
        }
        ensureCustomRatios()
        tableView.forceLayout()
        header.forceLayout()
    }

    function persistCustomRatios() {
        ensureCustomRatios()
        root.app.saveBomWidthRatios(layoutKey(), customRatios)
        root.debugLog("INFO", "Persist custom ratios: " + JSON.stringify(customRatios))
    }

    function setSlotRatio(slot, ratio) {
        if (slot < 0 || slot >= root.app.bomModel.visibleSlotCount()) {
            root.debugLog("WARNING", "Set slot ratio rejected: invalid slot " + slot)
            return
        }
        ensureCustomRatios()
        const next = customRatios.slice()
        next[slot] = Math.max(0.2, Math.min(6.0, ratio))
        customRatios = next
        root.debugLog("INFO", "Set ratio for slot " + slot + " to " + next[slot].toFixed(2))
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

        let ratioSum = 0.0
        for (let index = 0; index < count; ++index) {
            ratioSum += root.slotRatio(index)
        }
        if (ratioSum <= 0.0001) {
            return Math.max(40, Math.floor(available / count))
        }

        const minTotal = count * root.minColumnWidth
        if (available <= minTotal) {
            const weightedTight = Math.floor(available * (root.slotRatio(slot) / ratioSum))
            return Math.max(40, weightedTight)
        }

        const extra = available - minTotal
        const weighted = root.minColumnWidth + extra * (root.slotRatio(slot) / ratioSum)
        return Math.floor(weighted)
    }

    function toggleSort(slot) {
        if (slot < 0 || slot >= root.app.bomModel.visibleSlotCount()) {
            root.debugLog("WARNING", "Sort rejected: invalid slot " + slot)
            return
        }
        ensureSortState()
        root.debugLog("INFO", "Sort slot " + slot + ", ascending=" + slotAscending[slot])
        root.app.bomModel.sortByVisibleColumn(slot, slotAscending[slot])
        slotAscending[slot] = !slotAscending[slot]
    }

    Component.onCompleted: {
        ensureSortState()
        restoreCustomRatios()
        root.debugLog("INFO", "BomPane initialized")
    }

    onWidthBucketChanged: restoreCustomRatios()

    Connections {
        target: root.app.bomModel
        function onModelReset() {
            root.ensureSortState()
            root.restoreCustomRatios()
            root.debugLog("INFO", "BOM model reset")
        }
        function onHeaderDataChanged() {
            root.restoreCustomRatios()
            root.debugLog("INFO", "BOM header changed")
        }
    }

    Popup {
        id: columnConfigPopup
        property int slot: -1
        width: 300
        height: Math.min(root.popupMaxHeight, contentColumn.implicitHeight + 20)
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
            id: contentColumn
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
                        if (!enabled) {
                            root.debugLog("ERROR", "Delete column failed: at least one visible column is required")
                            return
                        }
                        root.debugLog("INFO", "Delete visible slot " + columnConfigPopup.slot)
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
                        root.debugLog("INFO", "Insert slot at left of " + columnConfigPopup.slot)
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
                        root.debugLog("INFO", "Insert slot at right of " + columnConfigPopup.slot)
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
                Layout.preferredHeight: Math.max(110, Math.min(200, root.height * 0.26))
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
                                    const before = root.app.bomModel.visibleHeaderAt(columnConfigPopup.slot)
                                    root.debugLog("INFO", "Change header slot " + columnConfigPopup.slot + ": " + before + " -> " + fieldOption.modelData)
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
                from: 0.2
                to: 6.0
                stepSize: 0.1
                value: root.slotRatio(columnConfigPopup.slot)
                palette.accent: root.ratioAccentColor
                palette.highlight: root.ratioAccentColor
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
                        root.debugLog("INFO", "Reset ratio for slot " + columnConfigPopup.slot)
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
                    interactive: true
                    flickableDirection: Flickable.VerticalFlick
                    rowSpacing: 0
                    columnSpacing: 0
                    columnWidthProvider: function(column) { return root.slotWidth(column) }
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

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

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                width: 14
                height: 14
                radius: 10
                color: root.themeColors.card
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                width: 14
                height: 14
                radius: 10
                color: root.themeColors.card
            }

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: 14
                height: 14
                radius: 10
                color: root.themeColors.card
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 14
                height: 14
                radius: 10
                color: root.themeColors.card
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
