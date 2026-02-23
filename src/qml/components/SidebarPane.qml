pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

Rectangle {
    id: root
    required property var app
    required property var themeColors
    signal requestImport()
    signal requestNewProject()
    signal requestRenameProject(int index, string currentName)
    signal requestNewCategory()
    signal requestRenameCategory(int index, string currentName)
    property string selectedCategoryName: ""

    radius: 12
    color: root.themeColors.card
    border.color: root.themeColors.border

    function primaryTint(alpha) {
        return Qt.rgba(root.themeColors.primary.r, root.themeColors.primary.g, root.themeColors.primary.b, alpha)
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
    }

    Rectangle {
        id: headerCard
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        height: 92
        radius: 12
        color: root.themeColors.card
        border.color: root.themeColors.border

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            anchors.topMargin: 4
            anchors.leftMargin: 10

            Image {
                source: root.app.theme.currentThemeName === "Dark"
                    ? "qrc:/assets/Github-dark.png"
                    : "qrc:/assets/Github-light.png"
                Layout.preferredWidth: 85
                Layout.preferredHeight: 85
                fillMode: Image.PreserveAspectFit
                smooth: true
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally("https://github.com/890mn/Link2BOM")
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Layout.topMargin: -8

                Label {
                    text: "Link2BOM"
                    font.family: audioWide.name
                    font.pixelSize: 28
                    font.bold: true
                    color: root.themeColors.primary
                }

                RowLayout {
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14
                        radius: 7
                        color: root.app.theme.currentThemeName === "Light" ? root.themeColors.primary : "transparent"
                        border.color: root.themeColors.primary
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.app.theme.currentIndex = 0
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14
                        radius: 7
                        color: root.app.theme.currentThemeName === "Dark" ? root.themeColors.primary : "transparent"
                        border.color: root.themeColors.primary
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.app.theme.currentIndex = 1
                        }
                    }

                    Label { text: "890mn"; color: root.themeColors.muted; font.pixelSize: 12 }
                    Label { text: "v0.0.4"; color: root.themeColors.muted; font.pixelSize: 12 }
                }
            }
        }
    }

    Flickable {
        id: modulesFlick
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
                title: "导入"
                themeColors: root.themeColors
                collapsed: sidebarSettings.importCollapsed
                normalHeight: 156
                onCollapsedChanged: sidebarSettings.importCollapsed = collapsed

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    AppButton {
                        themeColors: root.themeColors
                        text: "立创导入（XLS）"
                        Layout.fillWidth: true
                        onClicked: root.requestImport()
                    }

                    AppButton {
                        themeColors: root.themeColors
                        text: "从 XLS/XLSX 导入"
                        Layout.fillWidth: true
                        onClicked: root.requestImport()
                    }

                    AppButton {
                        themeColors: root.themeColors
                        text: "OCR 图片导入（后续）"
                        Layout.fillWidth: true
                        onClicked: root.app.notify("OCR 导入：目标项目 " + root.app.projects.selectedProject + "（识别流程待接入）")
                    }
                }
            }

            SidebarModuleCard {
                width: modulesColumn.width
                title: "导出"
                themeColors: root.themeColors
                collapsed: sidebarSettings.exportCollapsed
                normalHeight: 92
                onCollapsedChanged: sidebarSettings.exportCollapsed = collapsed

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    AppButton {
                        themeColors: root.themeColors
                        text: "导出 CSV"
                        Layout.fillWidth: true
                        onClicked: root.app.notify("CSV 导出任务已触发：范围 " + root.app.projects.selectedProject)
                    }
                }
            }

            SidebarModuleCard {
                width: modulesColumn.width
                title: "项目"
                themeColors: root.themeColors
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
                            text: root.app.projects.model.data(
                                      root.app.projects.model.index(projectDelegate.index, 0),
                                      Qt.DisplayRole
                                  ) ?? ""

                            contentItem: Text {
                                text: projectDelegate.text
                                color: root.themeColors.text
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            background: Rectangle {
                                color: root.app.projects.selectedProject === projectDelegate.text ? root.primaryTint(0.14) : "transparent"
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: root.app.projects.selectedProject === projectDelegate.text ? 5 : 2
                                    height: parent.height * 0.72
                                    radius: 2
                                    color: root.app.projects.selectedProject === projectDelegate.text ? root.themeColors.primary : "transparent"
                                }
                            }

                            onClicked: root.app.projects.selectedProject = text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        AppButton {
                            themeColors: root.themeColors
                            text: "新建"
                            Layout.fillWidth: true
                            onClicked: root.requestNewProject()
                        }

                        AppButton {
                            themeColors: root.themeColors
                            text: "重命名"
                            Layout.fillWidth: true
                            onClicked: root.requestRenameProject(projectList.currentIndex, root.app.projects.selectedProject)
                        }

                        AppButton {
                            themeColors: root.themeColors
                            text: "取消选中"
                            Layout.fillWidth: true
                            onClicked: root.app.projects.clearSelection()
                        }
                    }
                }
            }

            SidebarModuleCard {
                width: modulesColumn.width
                title: "分类组"
                themeColors: root.themeColors
                collapsed: sidebarSettings.categoriesCollapsed
                normalHeight: 220
                onCollapsedChanged: sidebarSettings.categoriesCollapsed = collapsed

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    ListView {
                        id: categoryList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.app.categories.model

                        delegate: ItemDelegate {
                            id: categoryDelegate
                            required property int index
                            width: ListView.view.width
                            text: root.app.categories.model.data(
                                      root.app.categories.model.index(categoryDelegate.index, 0),
                                      Qt.DisplayRole
                                  ) ?? ""

                            contentItem: Text {
                                text: categoryDelegate.text
                                color: root.themeColors.text
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            background: Rectangle {
                                color: categoryList.currentIndex === categoryDelegate.index ? root.primaryTint(0.14) : "transparent"
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: categoryList.currentIndex === categoryDelegate.index ? 5 : 2
                                    height: parent.height * 0.72
                                    radius: 2
                                    color: categoryList.currentIndex === categoryDelegate.index ? root.themeColors.primary : "transparent"
                                }
                            }

                            onClicked: {
                                categoryList.currentIndex = categoryDelegate.index
                                root.selectedCategoryName = text
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        AppButton {
                            themeColors: root.themeColors
                            text: "新增"
                            Layout.fillWidth: true
                            onClicked: root.requestNewCategory()
                        }

                        AppButton {
                            themeColors: root.themeColors
                            text: "修改"
                            Layout.fillWidth: true
                            onClicked: root.requestRenameCategory(categoryList.currentIndex, root.selectedCategoryName)
                        }
                    }
                }
            }

            Item {
                width: 1
                height: 12
            }
        }
    }
}

