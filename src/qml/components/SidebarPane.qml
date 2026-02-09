import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
    color: themeColors.card
    border.color: themeColors.border

    FontLoader {
        id: audioWide
        source: "qrc:/qt/qml/StarBOM/src/asset/Audiowide-Regular.ttf"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Image {
                source: app.theme.currentThemeName === "Dark"
                    ? "qrc:/qt/qml/StarBOM/src/asset/Github-dark.png"
                    : "qrc:/qt/qml/StarBOM/src/asset/Github-light.png"
                width: 34
                height: 34
                fillMode: Image.PreserveAspectFit
                smooth: true
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally("https://github.com/890mn/StarBOM")
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label {
                    text: "StarBOM"
                    font.family: audioWide.name
                    font.pixelSize: 34
                    font.bold: true
                    color: themeColors.primary
                }
                Text {
                    text: app.theme.currentThemeName
                    font.pixelSize: 34
                    color: themeColors.primary
                    font.bold: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: app.cycleTheme()
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "890mn"; color: themeColors.muted; font.pixelSize: 12 }
                    Label { text: "v0.0.4"; color: themeColors.muted; font.pixelSize: 12 }
                }
            }
        }

        GroupBox {
            title: "导入"
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                Button { text: "立创导入（XLS）"; Layout.fillWidth: true; onClicked: root.requestImport() }
                Button { text: "从 XLS/XLSX 导入"; Layout.fillWidth: true; onClicked: root.requestImport() }
                Button { text: "OCR 图片导入（后续）"; Layout.fillWidth: true; onClicked: app.notify("OCR 导入：目标项目 " + app.projects.selectedProject + "（识别流程待接入）。") }
            }
        }

        GroupBox {
            title: "导出"
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                Button { text: "导出 CSV"; Layout.fillWidth: true; onClicked: app.notify("CSV 导出任务已触发：范围 " + app.projects.selectedProject) }
            }
        }

        GroupBox {
            title: "项目"
            Layout.fillWidth: true
            Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                ListView {
                    id: projectList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    clip: true
                    model: app.projects.model
                    currentIndex: Math.max(0, app.projects.projectNames(true).indexOf(app.projects.selectedProject))
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: (model.display !== undefined) ? model.display : ""
                        leftPadding: 14
                        background: Rectangle {
                            color: app.projects.selectedProject === parent.text ? Qt.rgba(46/255, 91/255, 1, 0.12) : "transparent"
                            Rectangle {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                width: app.projects.selectedProject === parent.text ? 5 : 2
                                height: parent.height * 0.72
                                radius: 2
                                color: app.projects.selectedProject === parent.text ? themeColors.primary : "transparent"
                            }
                        }
                        onClicked: app.projects.selectedProject = text
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Button { text: "新建"; Layout.fillWidth: true; onClicked: root.requestNewProject() }
                    Button {
                        text: "重命名"
                        Layout.fillWidth: true
                        onClicked: root.requestRenameProject(projectList.currentIndex, app.projects.selectedProject)
                    }
                    Button { text: "取消选中"; Layout.fillWidth: true; onClicked: app.projects.clearSelection() }
                }
            }
        }

        GroupBox {
            title: "分类组"
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                ListView {
                    id: categoryList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: app.categories.model
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: (model.display !== undefined) ? model.display : ""
                        onClicked: {
                            categoryList.currentIndex = index
                            root.selectedCategoryName = text
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Button { text: "新增"; Layout.fillWidth: true; onClicked: root.requestNewCategory() }
                    Button { text: "修改"; Layout.fillWidth: true; onClicked: root.requestRenameCategory(categoryList.currentIndex, root.selectedCategoryName) }
                }
            }
        }
    }
}
