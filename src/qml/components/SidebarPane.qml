pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

Rectangle {
    id: root
    required property var app
    required property var themeColors
    required property bool pinnedTopMost

    signal togglePinned()
    signal requestImport()
    signal requestNewProject()
    signal requestRenameProject(int index, string currentName)
    signal requestDeleteProject(int index, string currentName)
    signal requestNewCategory()
    signal requestRenameCategory(int index, string currentName)
    signal requestDeleteCategory(int index, string currentName)
    signal toggleDebugPanel()

    property int selectedCategoryIndex: -1
    property string selectedCategoryName: ""
    property var brandTreeGroups: []
    property var brandTreeChildren: ({})
    property var brandTreeExpanded: ({})
    property var packageTreeGroups: []
    property var packageTreeChildren: ({})
    property var packageTreeExpanded: ({})
    property var typeTreeGroups: []
    property var typeTreeChildren: ({})
    property var typeTreeExpanded: ({})

    radius: 12
    color: root.themeColors.card
    border.color: root.themeColors.border

    function primaryTint(alpha) {
        return Qt.rgba(root.themeColors.primary.r, root.themeColors.primary.g, root.themeColors.primary.b, alpha)
    }

    function majorKind(text) {
        const value = String(text).toLowerCase()
        if (value.includes("resistor") || value.includes("电阻")) return "电阻/阻值件"
        if (value.includes("capacitor") || value.includes("cap") || value.includes("电容")) return "电容"
        if (value.includes("inductor") || value.includes("电感")) return "电感"
        if (value.includes("ic") || value.includes("mcu") || value.includes("芯片")) return "IC/芯片"
        if (value.includes("connector") || value.includes("连接器")) return "连接器"
        if (value.includes("switch") || value.includes("按键")) return "开关"
        if (value.includes("power") || value.includes("regulator") || value.includes("dc-dc")) return "电源"
        return "其他"
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
        const brandValues = root.app.bomModel.distinctValuesByHeaderAliases(["品牌", "brand"], 2)
        const packageValues = root.app.bomModel.distinctValuesByHeaderAliases(["封装", "package"], 4)
        const typeValues = root.app.bomModel.distinctValuesByHeaderAliases(["商品名称", "name", "description"], 5)

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

    FontLoader {
        id: audioWide
        source: "qrc:/assets/Audiowide-Regular.ttf"
    }

    Settings {
        id: sidebarSettings
        category: "Sidebar"
        property bool importCollapsed: false
        property bool exportCollapsed: false
        property bool projectsCollapsed: false
        property bool categoriesCollapsed: false
        property bool categoriesAutoHeight: true
        property int categoriesCustomHeight: 430
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
                        onClicked: {
                            treeSection.toggleGroup(groupNode.modelData)
                        }
                    }
                }

                Column {
                    width: parent.width
                    visible: treeSection.expandedMap[groupNode.modelData]
                    spacing: 0
                    Repeater {
                        model: treeSection.childrenMap[groupNode.modelData] ? treeSection.childrenMap[groupNode.modelData] : []
                        delegate: Rectangle {
                            id: leafNode
                            required property string modelData
                            width: parent.width
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

    Rectangle {
        id: headerCard
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        height: 96
        radius: 12
        color: root.themeColors.card
        border.color: root.themeColors.border

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            anchors.topMargin: 4
            anchors.leftMargin: 10

            Image {
                source: root.app.theme.currentThemeName === "Dark" ? "qrc:/assets/Github-dark.png" : "qrc:/assets/Github-light.png"
                Layout.preferredWidth: 85
                Layout.preferredHeight: 85
                fillMode: Image.PreserveAspectFit
                smooth: true
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Qt.openUrlExternally("https://github.com/890mn/Link2BOM") }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Layout.topMargin: -8

                Item {
                    Layout.preferredWidth: 190
                    Layout.preferredHeight: 34
                    Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Link2BOM"; font.family: audioWide.name; font.pixelSize: 28; font.bold: true; color: root.themeColors.primary }
                    Image { visible: root.pinnedTopMost; source: root.app.theme.currentThemeName === "Dark" ? "qrc:/assets/pin-dark.png" : "qrc:/assets/pin-light.png"; anchors.right: parent.right; anchors.top: parent.top; anchors.topMargin: 4; width: 16; height: 16; fillMode: Image.PreserveAspectFit }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.togglePinned() }
                }

                RowLayout {
                    spacing: 8
                    Repeater {
                        model: 3
                        delegate: Rectangle {
                            id: themeDot
                            required property int index
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            radius: 8
                            color: root.app.theme.currentIndex === index ? root.themeColors.primary : "transparent"
                            border.color: root.themeColors.primary
                            MouseArea { anchors.fill: parent; onClicked: root.app.theme.currentIndex = themeDot.index }
                        }
                    }
                    Label { text: "890mn"; color: root.themeColors.muted; font.pixelSize: 12; MouseArea { anchors.fill: parent; onPressAndHold: root.toggleDebugPanel() } }
                    Label { text: "v0.0.4"; color: root.themeColors.muted; font.pixelSize: 12 }
                }
            }
        }
    }

    Flickable {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerCard.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 12
        anchors.topMargin: 8
        clip: true
        contentWidth: width
        contentHeight: modulesColumn.childrenRect.height + 12
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: modulesColumn
            width: parent.width
            spacing: 10

            SidebarModuleCard {
                width: modulesColumn.width
                title: "Import"
                themeColors: root.themeColors
                darkTheme: root.app.theme.currentThemeName === "Dark"
                collapsed: sidebarSettings.importCollapsed
                normalHeight: 176
                onCollapsedChanged: sidebarSettings.importCollapsed = collapsed
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    AppButton { themeColors: root.themeColors; text: "LCSC Import (XLS)"; Layout.fillWidth: true; onClicked: root.requestImport() }
                    AppButton { themeColors: root.themeColors; text: "Import XLS/XLSX"; Layout.fillWidth: true; onClicked: root.requestImport() }
                    AppButton { themeColors: root.themeColors; text: "OCR Import (Later)"; Layout.fillWidth: true; onClicked: root.app.notify("OCR flow is not connected yet.") }
                }
            }

            SidebarModuleCard {
                width: modulesColumn.width
                title: "Export"
                themeColors: root.themeColors
                darkTheme: root.app.theme.currentThemeName === "Dark"
                collapsed: sidebarSettings.exportCollapsed
                normalHeight: 96
                onCollapsedChanged: sidebarSettings.exportCollapsed = collapsed
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    AppButton { themeColors: root.themeColors; text: "Export CSV"; Layout.fillWidth: true; onClicked: root.app.notify("CSV export is triggered.") }
                }
            }

            SidebarModuleCard {
                width: modulesColumn.width
                title: "Projects"
                themeColors: root.themeColors
                darkTheme: root.app.theme.currentThemeName === "Dark"
                collapsed: sidebarSettings.projectsCollapsed
                normalHeight: 260
                onCollapsedChanged: sidebarSettings.projectsCollapsed = collapsed
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    ListView {
                        id: projectList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.app.projects.model
                        delegate: ItemDelegate {
                            id: projectDelegate
                            required property int index
                            width: ListView.view.width
                            leftPadding: 14
                            rightPadding: 8
                            text: root.app.projects.model.data(root.app.projects.model.index(projectDelegate.index, 0), Qt.DisplayRole) ?? ""
                            contentItem: Text { anchors.fill: parent; anchors.leftMargin: 18; text: projectDelegate.text; color: root.themeColors.text; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
                            background: Rectangle {
                                color: root.app.projects.selectedProject === projectDelegate.text ? root.primaryTint(0.14) : "transparent"
                                Rectangle { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: root.app.projects.selectedProject === projectDelegate.text ? 5 : 2; height: parent.height * 0.72; radius: 2; color: root.app.projects.selectedProject === projectDelegate.text ? root.themeColors.primary : "transparent" }
                            }
                            onClicked: { projectList.currentIndex = projectDelegate.index; root.app.projects.selectedProject = text }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        AppButton { themeColors: root.themeColors; text: "New"; Layout.fillWidth: true; onClicked: root.requestNewProject() }
                        AppButton { themeColors: root.themeColors; text: "Rename"; Layout.fillWidth: true; onClicked: root.requestRenameProject(projectList.currentIndex, root.app.projects.selectedProject) }
                        AppButton { themeColors: root.themeColors; text: "Delete"; Layout.fillWidth: true; onClicked: root.requestDeleteProject(projectList.currentIndex, root.app.projects.selectedProject) }
                    }
                }
            }

            SidebarModuleCard {
                id: categoriesCard
                width: modulesColumn.width
                title: "Categories"
                themeColors: root.themeColors
                darkTheme: root.app.theme.currentThemeName === "Dark"
                collapsed: sidebarSettings.categoriesCollapsed
                normalHeight: sidebarSettings.categoriesAutoHeight ? Math.max(430, categoriesContent.implicitHeight + 86) : sidebarSettings.categoriesCustomHeight
                onCollapsedChanged: sidebarSettings.categoriesCollapsed = collapsed
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        AppButton { themeColors: root.themeColors; text: "Auto Height"; Layout.fillWidth: true; accent: sidebarSettings.categoriesAutoHeight; onClicked: sidebarSettings.categoriesAutoHeight = true }
                        AppButton { themeColors: root.themeColors; text: "Custom Height"; Layout.fillWidth: true; accent: !sidebarSettings.categoriesAutoHeight; onClicked: sidebarSettings.categoriesAutoHeight = false }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: !sidebarSettings.categoriesAutoHeight
                        Label { text: "Height: " + sidebarSettings.categoriesCustomHeight; color: root.themeColors.text; font.pixelSize: 12 }
                        Slider {
                            Layout.fillWidth: true
                            from: 320; to: 860; stepSize: 10
                            value: sidebarSettings.categoriesCustomHeight
                            onMoved: sidebarSettings.categoriesCustomHeight = Math.round(value)
                            onValueChanged: if (pressed) sidebarSettings.categoriesCustomHeight = Math.round(value)
                        }
                    }

                    ListView {
                        id: categoryGroupList
                        Layout.fillWidth: true
                        Layout.preferredHeight: 90
                        model: root.app.categories.model
                        clip: true
                        spacing: 2
                        currentIndex: root.selectedCategoryIndex
                        delegate: ItemDelegate {
                            id: categoryDelegate
                            required property int index
                            width: ListView.view.width
                            leftPadding: 14
                            rightPadding: 8
                            text: root.app.categories.model.data(root.app.categories.model.index(categoryDelegate.index, 0), Qt.DisplayRole) ?? ""
                            contentItem: Text { anchors.fill: parent; anchors.leftMargin: 18; text: categoryDelegate.text; color: root.themeColors.text; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
                            background: Rectangle {
                                color: root.selectedCategoryIndex === categoryDelegate.index ? root.primaryTint(0.14) : "transparent"
                                Rectangle { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: root.selectedCategoryIndex === categoryDelegate.index ? 5 : 2; height: parent.height * 0.72; radius: 2; color: root.selectedCategoryIndex === categoryDelegate.index ? root.themeColors.primary : "transparent" }
                            }
                            onClicked: { root.selectedCategoryIndex = categoryDelegate.index; root.selectedCategoryName = text }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        AppButton { themeColors: root.themeColors; text: "New"; Layout.fillWidth: true; onClicked: root.requestNewCategory() }
                        AppButton { themeColors: root.themeColors; text: "Rename"; Layout.fillWidth: true; onClicked: root.requestRenameCategory(root.selectedCategoryIndex, root.selectedCategoryName) }
                        AppButton { themeColors: root.themeColors; text: "Delete"; Layout.fillWidth: true; onClicked: root.requestDeleteCategory(root.selectedCategoryIndex, root.selectedCategoryName) }
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        Column {
                            id: categoriesContent
                            width: categoriesCard.width - 36
                            spacing: 8
                            TreeSection {
                                title: "Brand"
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
                                title: "Package"
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
                                title: "Type"
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
                        AppButton { themeColors: root.themeColors; text: "Clear Type Filter"; Layout.fillWidth: true; onClicked: root.app.bomModel.clearTypeFilter() }
                    }
                }
            }

            Item { width: 1; height: 12 }
        }
    }
}
