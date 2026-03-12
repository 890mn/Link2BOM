pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

Item {
    id: root
    required property var app
    required property var themeColors
    required property bool pinnedTopMost
    required property string uiLanguage
    required property var textMap

    signal togglePinned()
    signal openSettings()
    signal requestImport(string mode)
    signal requestExport()
    signal requestNewProject()
    signal requestRenameProject(int index, string currentName)
    signal requestDeleteProject(int index, string currentName)
    signal requestNewCategory()
    signal requestRenameCategory(int index, string currentName)
    signal requestDeleteCategory(int index, string currentName)
    signal toggleDebugPanel()
    signal openArchive()

    function txSafe(key, fallback) {
        if (root.textMap && root.textMap[key] !== undefined) {
            const value = root.textMap[key]
            if (value !== undefined && value !== key) {
                return value
            }
        }
        return fallback
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
        property bool filtersCollapsed: false
        property bool categoriesAutoHeight: true
        property int categoriesCustomHeight: 430
    }

    // Title bar
    Rectangle {
        id: headerCard
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        height: 95
        radius: 12
        color: root.themeColors.card
        border.color: root.themeColors.border

        RowLayout {
            anchors.fill: parent
            anchors.margins: 6

            Image {
                source: root.app.theme.currentThemeName === "Dark" ? "qrc:/assets/Github-dark.png" : "qrc:/assets/Github-light.png"
                Layout.preferredWidth: 90
                Layout.preferredHeight: 90
                fillMode: Image.PreserveAspectFit
                smooth: true
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Qt.openUrlExternally("https://github.com/890mn/Link2BOM") }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Item {
                    Layout.preferredWidth: 190
                    Layout.preferredHeight: 20
                    Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Link2BOM"; font.family: audioWide.name; font.pixelSize: 28; font.bold: true; color: root.themeColors.primary }
                    Image { visible: root.pinnedTopMost; source: root.app.theme.currentThemeName === "Dark" ? "qrc:/assets/pin-dark.png" : "qrc:/assets/pin-light.png"; anchors.right: parent.right; anchors.top: parent.top; anchors.topMargin: 4; width: 16; height: 16; fillMode: Image.PreserveAspectFit }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.togglePinned() }
                }

                Label {
                    text: "Settings / Themes / V1"
                    color: root.themeColors.muted
                    font.pixelSize: 16
                    font.family: audioWide.name
                    MouseArea { anchors.fill: parent; onPressAndHold: root.toggleDebugPanel() }
                }

                RowLayout {
                    spacing: 8
                    Layout.topMargin: -2

                    Item {
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18

                        Image {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            source: root.app.theme.currentThemeName === "Dark"
                                ? "qrc:/assets/setting-dark.png"
                                : "qrc:/assets/setting-light.png"
                            smooth: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.openSettings()
                        }
                    }

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

                    Item {
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18

                        Image {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            source: root.app.theme.currentThemeName === "Dark"
                                ? "qrc:/assets/achieve-dark.png"
                                : "qrc:/assets/achieve-light.png"
                            smooth: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.openArchive()
                        }
                    }
                }
            }
        }
    }

    // left all
    Item {
        id: modulesArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerCard.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        anchors.topMargin: 8

        Flickable {
            id: modulesFlick
            anchors.fill: parent
            clip: true
            contentWidth: width
            contentHeight: modulesColumn.childrenRect.height + 12
            boundsBehavior: Flickable.StopAtBounds

            Column {
                id: modulesColumn
                width: modulesFlick.width
                spacing: 10

                SidebarModuleCard {
                    width: modulesColumn.width
                    title: root.txSafe("sidebar.import", "Import")
                    themeColors: root.themeColors
                    darkTheme: root.app.theme.currentThemeName === "Dark"
                    collapsed: sidebarSettings.importCollapsed
                    normalHeight: 176
                    onCollapsedChanged: sidebarSettings.importCollapsed = collapsed
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8
                        AppButton { themeColors: root.themeColors; text: root.txSafe("sidebar.import.lcsc", "LCSC Import (XLS)"); Layout.fillWidth: true; onClicked: root.requestImport("lcsc") }
                        AppButton { themeColors: root.themeColors; text: root.txSafe("sidebar.import.sheet", "Import XLS/XLSX/CSV"); Layout.fillWidth: true; onClicked: root.requestImport("generic") }
                        AppButton { themeColors: root.themeColors; text: root.txSafe("sidebar.import.ocr", "OCR Import (Later)"); Layout.fillWidth: true; onClicked: root.requestImport("ocr") }
                    }
                }

                SidebarModuleCard {
                    width: modulesColumn.width
                    title: root.txSafe("sidebar.export", "Export")
                    themeColors: root.themeColors
                    darkTheme: root.app.theme.currentThemeName === "Dark"
                    collapsed: sidebarSettings.exportCollapsed
                    normalHeight: 96
                    onCollapsedChanged: sidebarSettings.exportCollapsed = collapsed
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8
                        AppButton { themeColors: root.themeColors; text: root.txSafe("sidebar.export.csv", "Export CSV"); Layout.fillWidth: true; onClicked: root.requestExport() }
                    }
                }

                SidebarProjectActions {
                    width: modulesColumn.width
                    app: root.app
                    themeColors: root.themeColors
                    textMap: root.textMap
                    collapsed: sidebarSettings.projectsCollapsed
                    onCollapsedChanged: sidebarSettings.projectsCollapsed = collapsed
                    onRequestNewProject: root.requestNewProject()
                    onRequestRenameProject: function(index, currentName) { root.requestRenameProject(index, currentName) }
                    onRequestDeleteProject: function(index, currentName) { root.requestDeleteProject(index, currentName) }
                }

                SidebarCategoryActions {
                    width: modulesColumn.width
                    app: root.app
                    themeColors: root.themeColors
                    textMap: root.textMap
                    collapsed: sidebarSettings.categoriesCollapsed
                    onCollapsedChanged: sidebarSettings.categoriesCollapsed = collapsed
                    onRequestNewCategory: root.requestNewCategory()
                    onRequestRenameCategory: function(index, currentName) { root.requestRenameCategory(index, currentName) }
                    onRequestDeleteCategory: function(index, currentName) { root.requestDeleteCategory(index, currentName) }
                }

                SidebarFilterTree {
                    width: modulesColumn.width
                    app: root.app
                    themeColors: root.themeColors
                    textMap: root.textMap
                    collapsed: sidebarSettings.filtersCollapsed
                    autoHeight: sidebarSettings.categoriesAutoHeight
                    customHeight: sidebarSettings.categoriesCustomHeight
                    onCollapsedChanged: sidebarSettings.filtersCollapsed = collapsed
                    onAutoHeightChanged: sidebarSettings.categoriesAutoHeight = autoHeight
                    onCustomHeightChanged: sidebarSettings.categoriesCustomHeight = customHeight
                }

                Item { width: 1; height: 12 }
            }
        }
    }
}







