pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

Item {
    id: root
    required property var themeColors
    required property color textColor
    required property color mutedTextColor
    required property color primaryColor
    required property color subtleColor
    required property bool darkTheme
    required property var textMap
    required property string groupMode
    required property string viewMode
    required property var diffItems
    required property var diffStats
    signal groupModeSelected(string value)
    signal viewModeSelected(string value)

    function txSafe(key, fallback) {
        if (root.textMap && root.textMap[key] !== undefined) {
            return root.textMap[key]
        }
        return fallback
    }

    function buildBarData() {
        const items = (root.diffStats && root.diffStats.groupItems) ? root.diffStats.groupItems : []
        const labels = []
        const values = []
        for (let i = 0; i < items.length; ++i) {
            labels.push(String(items[i].name))
            values.push(Number(items[i].count))
        }
        return { labels: labels, values: values }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 50
            radius: 12
            color: root.themeColors.card
            border.color: root.themeColors.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Label {
                    text: root.txSafe("diff.group.by", "Group By")
                    color: root.mutedTextColor
                }

                RowLayout {
                    spacing: 6

                    Repeater {
                        model: [
                            { "label": root.txSafe("diff.group.project", "By Project"), "value": "project" },
                            { "label": root.txSafe("diff.group.package", "By Package"), "value": "package" },
                            { "label": root.txSafe("diff.group.brand", "By Brand"), "value": "brand" }
                        ]

                        delegate: AppButton {
                            required property var modelData
                            themeColors: root.themeColors
                            text: modelData.label
                            accent: root.groupMode === modelData.value
                            implicitHeight: 30
                            cornerRadius: 8
                            onClicked: {
                                if (root.groupMode !== modelData.value) {
                                    root.groupModeSelected(modelData.value)
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                RowLayout {
                    spacing: 6
                    Repeater {
                        model: [
                            { "label": root.txSafe("diff.view.list", "Diff List"), "value": "list" },
                            { "label": root.txSafe("diff.view.bar", "Bar Chart"), "value": "bar" }
                        ]
                        delegate: AppButton {
                            required property var modelData
                            themeColors: root.themeColors
                            text: modelData.label
                            accent: root.viewMode === modelData.value
                            implicitHeight: 30
                            cornerRadius: 8
                            onClicked: root.viewModeSelected(modelData.value)
                        }
                    }
                }

                Label {
                    text: root.txSafe("diff.result.count", "Diff Items") + ": " + root.diffItems.length
                    color: root.textColor
                    font.bold: true
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.viewMode === "bar" ? 1 : 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: root.diffItems

                        delegate: Rectangle {
                            id: diffCard
                            required property var modelData
                            width: ListView.view.width
                            radius: 12
                            color: root.themeColors.card
                            border.color: Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.45)
                            implicitHeight: itemColumn.implicitHeight + 16

                            ColumnLayout {
                                id: itemColumn
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Label {
                                        Layout.fillWidth: true
                                        text: diffCard.modelData.key
                                        color: root.textColor
                                        elide: Text.ElideRight
                                        font.bold: true
                                    }

                                    Rectangle {
                                        radius: 8
                                        color: Qt.rgba(239/255, 68/255, 68/255, 0.15)
                                        border.color: "#EF4444"
                                        implicitHeight: 24
                                        implicitWidth: diffBadge.implicitWidth + 14

                                        Label {
                                            id: diffBadge
                                            anchors.centerIn: parent
                                            text: root.txSafe("diff.changed.fields", "Changed Fields") + ": " + diffCard.modelData.changedFieldCount
                                            color: "#EF4444"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }
                                }

                                Repeater {
                                    model: diffCard.modelData.fieldDetails

                                    delegate: RowLayout {
                                        id: fieldRow
                                        required property var modelData
                                        Layout.fillWidth: true
                                        Layout.topMargin: 1
                                        Layout.bottomMargin: 1
                                        spacing: 8

                                        Label {
                                            Layout.preferredWidth: 160
                                            Layout.maximumWidth: 160
                                            text: fieldRow.modelData.field
                                            color: root.primaryColor
                                            font.bold: true
                                            elide: Text.ElideRight
                                        }

                                        Item {
                                            Layout.fillWidth: true
                                            implicitHeight: valueFlow.implicitHeight
                                            Layout.preferredHeight: implicitHeight

                                            Flow {
                                                id: valueFlow
                                                width: parent.width
                                                spacing: 6

                                                Repeater {
                                                    model: fieldRow.modelData.values
                                                    delegate: Rectangle {
                                                        id: valueChip
                                                        required property string modelData
                                                        radius: 6
                                                        color: root.subtleColor
                                                        border.color: root.themeColors.border
                                                        implicitHeight: 22
                                                        implicitWidth: valueText.implicitWidth + 12

                                                        Label {
                                                            id: valueText
                                                            anchors.centerIn: parent
                                                            text: valueChip.modelData
                                                            color: root.textColor
                                                            font.pixelSize: 12
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        visible: root.diffItems.length === 0
                        text: root.txSafe("diff.noresult", "No diff items found")
                        color: root.mutedTextColor
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 12
                        color: root.themeColors.card
                        border.color: root.themeColors.border

                        ChartView {
                            id: barChart
                            anchors.fill: parent
                            anchors.margins: 12
                            antialiasing: true
                            legend.visible: false
                            backgroundColor: "transparent"

                            BarSeries {
                                id: barSeries
                                axisX: barAxis
                                axisY: valueAxis
                                BarSet {
                                    id: barSet
                                    label: root.txSafe("diff.result.count", "Diff Items")
                                    color: root.primaryColor
                                    values: buildBarData().values
                                }
                            }

                            BarCategoryAxis {
                                id: barAxis
                                categories: buildBarData().labels
                                labelsAngle: -30
                                labelsColor: root.textColor                            }

                            ValueAxis {
                                id: valueAxis
                                min: 0
                                max: {
                                    const values = buildBarData().values
                                    let maxValue = 1
                                    for (let i = 0; i < values.length; ++i) {
                                        if (values[i] > maxValue) maxValue = values[i]
                                    }
                                    return Math.ceil(maxValue * 1.15)
                                }
                                labelFormat: "%d"
                                labelsColor: root.textColor
                                gridLineColor: root.themeColors.border
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 122
                        radius: 12
                        color: root.themeColors.card
                        border.color: root.themeColors.border

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 4
                            Label { text: root.txSafe("diff.health.score", "Inventory Health") + ": " + (root.diffStats && root.diffStats.healthScore !== undefined ? root.diffStats.healthScore : 0); color: root.textColor; font.bold: true }
                            Label { text: root.txSafe("diff.health.total", "Total Rows") + ": " + (root.diffStats && root.diffStats.totalRows !== undefined ? root.diffStats.totalRows : 0) + "   " + root.txSafe("diff.health.unique", "Unique Parts") + ": " + (root.diffStats && root.diffStats.uniquePartCount !== undefined ? root.diffStats.uniquePartCount : 0); color: root.textColor }
                            Label { text: root.txSafe("diff.health.lowqty", "Low Stock") + ": " + (root.diffStats && root.diffStats.lowQtyCount !== undefined ? root.diffStats.lowQtyCount : 0) + "   " + root.txSafe("diff.health.missing", "Missing Key Fields") + ": " + (root.diffStats && root.diffStats.missingPartCount !== undefined ? root.diffStats.missingPartCount : 0); color: root.textColor }
                            Label { text: root.txSafe("diff.health.duplicate", "Duplicate Parts") + ": " + (root.diffStats && root.diffStats.duplicatePartCount !== undefined ? root.diffStats.duplicatePartCount : 0); color: root.textColor }
                        }
                    }
                }
            }
        }
    }
}

