pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var app
    required property var themeColors
    required property var textMap
    property bool collapsed: false
    property bool autoHeight: true
    property int customHeight: 430

    property var brandTreeGroups: []
    property var brandTreeChildren: ({})
    property var brandTreeExpanded: ({})
    property var packageTreeGroups: []
    property var packageTreeChildren: ({})
    property var packageTreeExpanded: ({})
    property var typeTreeGroups: []
    property var typeTreeChildren: ({})
    property var typeTreeExpanded: ({})

    function txSafe(key, fallback) {
        if (root.textMap && root.textMap[key] !== undefined) {
            const value = root.textMap[key]
            if (value !== undefined && value !== key) {
                return value
            }
        }
        return fallback
    }

    function majorKind(text) {
        const value = String(text).toLowerCase()
        if (value.includes("resistor")) return txSafe("type.resistor", "Resistor")
        if (value.includes("capacitor") || value.includes("cap")) return txSafe("type.capacitor", "Capacitor")
        if (value.includes("inductor")) return txSafe("type.inductor", "Inductor")
        if (value.includes("ic") || value.includes("mcu")) return txSafe("type.ic", "IC/Chip")
        if (value.includes("connector")) return txSafe("type.connector", "Connector")
        if (value.includes("switch")) return txSafe("type.switch", "Switch")
        if (value.includes("power") || value.includes("regulator") || value.includes("dc-dc")) return txSafe("type.power", "Power")
        return txSafe("type.other", "Other")
    }

    function buildTreeByInitial(values) {
        const groups = []
        const childrenMap = {}
        const expandedMap = {}
        for (let i = 0; i < values.length; ++i) {
            const value = String(values[i]).trim()
            if (value.length === 0) continue
            const key = value[0].toUpperCase()
            if (!childrenMap[key]) {
                childrenMap[key] = []
                groups.push(key)
                expandedMap[key] = true
            }
            childrenMap[key].push(value)
        }
        groups.sort()
        return { "groups": groups, "children": childrenMap, "expanded": expandedMap }
    }

    function buildTypeTree(values) {
        const groups = []
        const childrenMap = {}
        const expandedMap = {}
        for (let i = 0; i < values.length; ++i) {
            const value = String(values[i]).trim()
            if (value.length === 0) continue
            const group = majorKind(value)
            if (!childrenMap[group]) {
                childrenMap[group] = []
                groups.push(group)
                expandedMap[group] = true
            }
            childrenMap[group].push(value)
        }
        return { "groups": groups, "children": childrenMap, "expanded": expandedMap }
    }

    function refreshCategoryBuckets() {
        const brandValues = root.app.bomModel.distinctValuesByHeaderAliases(["brand"], 2)
        const packageValues = root.app.bomModel.distinctValuesByHeaderAliases(["package"], 4)
        const typeValues = root.app.bomModel.distinctValuesByHeaderAliases(["name", "description"], 5)

        const brandTree = buildTreeByInitial(brandValues)
        brandTreeGroups = brandTree.groups
        brandTreeChildren = brandTree.children
        brandTreeExpanded = brandTree.expanded

        const packageTree = buildTreeByInitial(packageValues)
        packageTreeGroups = packageTree.groups
        packageTreeChildren = packageTree.children
        packageTreeExpanded = packageTree.expanded

        const typeTree = buildTypeTree(typeValues)
        typeTreeGroups = typeTree.groups
        typeTreeChildren = typeTree.children
        typeTreeExpanded = typeTree.expanded
    }

    Component.onCompleted: refreshCategoryBuckets()

    Connections {
        target: root.app.bomModel
        function onModelReset() { root.refreshCategoryBuckets() }
        function onHeaderDataChanged() { root.refreshCategoryBuckets() }
    }

    component TreeSection: Column {
        id: treeSection
        required property string title
        required property var groups
        required property var childrenMap
        required property var expandedMap
        required property color textColor
        required property color mutedColor
        required property color subtleColor
        required property color borderColor
        required property color activeColor
        property string activeValue: ""
        property bool clickableLeaves: false
        signal toggleGroup(string groupKey)
        signal leafClicked(string value)
        width: parent ? parent.width : 280
        spacing: 2

        Label { text: treeSection.title; color: treeSection.mutedColor; font.pixelSize: 12; font.bold: true }

        Repeater {
            model: treeSection.groups
            delegate: Column {
                id: groupNode
                required property string modelData
                width: treeSection.width
                spacing: 1

                Rectangle {
                    width: parent.width
                    height: 28
                    radius: 8
                    color: treeSection.subtleColor
                    border.color: treeSection.borderColor
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 6
                        Label { text: treeSection.expandedMap[groupNode.modelData] ? "▼" : "▶"; color: treeSection.mutedColor }
                        Label { Layout.fillWidth: true; text: groupNode.modelData; color: treeSection.textColor; font.bold: true; elide: Text.ElideRight }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: treeSection.toggleGroup(groupNode.modelData)
                    }
                }

                Column {
                    width: parent.width
                    visible: !!treeSection.expandedMap[groupNode.modelData]
                    spacing: 0
                    Repeater {
                        model: treeSection.childrenMap[groupNode.modelData] ? treeSection.childrenMap[groupNode.modelData] : []
                        delegate: Rectangle {
                            id: leafNode
                            required property string modelData
                            width: treeSection.width
                            height: 24
                            color: "transparent"
                            Rectangle { x: 10; y: 0; width: 1; height: parent.height; color: treeSection.borderColor }
                            Rectangle { x: 10; y: parent.height / 2; width: 12; height: 1; color: treeSection.borderColor }
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 28
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 32
                                text: leafNode.modelData
                                color: treeSection.activeValue === leafNode.modelData ? treeSection.activeColor : treeSection.textColor
                                elide: Text.ElideRight
                            }
                            MouseArea {
                                anchors.fill: parent
                                enabled: treeSection.clickableLeaves
                                onClicked: treeSection.leafClicked(leafNode.modelData)
                            }
                        }
                    }
                }
            }
        }
    }

    implicitWidth: moduleCard.implicitWidth
    implicitHeight: moduleCard.implicitHeight

    SidebarModuleCard {
        id: moduleCard
        width: parent ? parent.width : 280
        title: root.txSafe("sidebar.filters", "Filters")
        themeColors: root.themeColors
        darkTheme: root.app.theme.currentThemeName === "Dark"
        collapsed: root.collapsed
        normalHeight: {
            const contentHeight = Math.max(420, filtersContent.implicitHeight + 46)
            const available = (parent && parent.height > 0) ? (parent.height - 18) : contentHeight
            return Math.max(minModuleHeight, Math.min(available, contentHeight))
        }
        onCollapsedChanged: root.collapsed = collapsed

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                padding: 0
                Column {
                    id: filtersContent
                    width: parent.width
                    spacing: 8
                    TreeSection {
                        title: root.txSafe("tree.brand", "Brand")
                        groups: root.brandTreeGroups
                        childrenMap: root.brandTreeChildren
                        expandedMap: root.brandTreeExpanded
                        textColor: root.themeColors.text
                        mutedColor: root.themeColors.muted
                        subtleColor: root.themeColors.subtle
                        borderColor: root.themeColors.border
                        activeColor: root.themeColors.primary
                        onToggleGroup: function(groupKey) {
                            const next = Object.assign({}, root.brandTreeExpanded)
                            next[groupKey] = !root.brandTreeExpanded[groupKey]
                            root.brandTreeExpanded = next
                        }
                    }
                    TreeSection {
                        title: root.txSafe("tree.package", "Package")
                        groups: root.packageTreeGroups
                        childrenMap: root.packageTreeChildren
                        expandedMap: root.packageTreeExpanded
                        textColor: root.themeColors.text
                        mutedColor: root.themeColors.muted
                        subtleColor: root.themeColors.subtle
                        borderColor: root.themeColors.border
                        activeColor: root.themeColors.primary
                        onToggleGroup: function(groupKey) {
                            const next = Object.assign({}, root.packageTreeExpanded)
                            next[groupKey] = !root.packageTreeExpanded[groupKey]
                            root.packageTreeExpanded = next
                        }
                    }
                    TreeSection {
                        title: root.txSafe("tree.type", "Type")
                        groups: root.typeTreeGroups
                        childrenMap: root.typeTreeChildren
                        expandedMap: root.typeTreeExpanded
                        textColor: root.themeColors.text
                        mutedColor: root.themeColors.muted
                        subtleColor: root.themeColors.subtle
                        borderColor: root.themeColors.border
                        activeColor: root.themeColors.primary
                        activeValue: root.app.bomModel.typeFilter
                        clickableLeaves: true
                        onToggleGroup: function(groupKey) {
                            const next = Object.assign({}, root.typeTreeExpanded)
                            next[groupKey] = !root.typeTreeExpanded[groupKey]
                            root.typeTreeExpanded = next
                        }
                        onLeafClicked: function(value) { root.app.bomModel.setTypeFilter(value) }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                AppButton { themeColors: root.themeColors; text: root.txSafe("tree.type.clear", "Clear Type Filter"); Layout.fillWidth: true; onClicked: root.app.bomModel.clearTypeFilter() }
            }
        }
    }
}
