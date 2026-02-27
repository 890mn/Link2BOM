pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

ApplicationWindow {
    id: root
    width: minimumWidth
    height: minimumHeight
    visible: true
    title: "Link2BOM"
    minimumWidth: 1100
    minimumHeight: 700

    required property var appCtx
    property string activeProjectForImport: ""
    property int renameProjectIndex: -1
    property int renameCategoryIndex: -1
    property bool pinnedTopMost: false

    flags: root.pinnedTopMost
        ? (Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint)
        : (Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint)

    property bool darkTheme: root.appCtx.theme.currentThemeName === "Dark"
    property color bgColor: darkTheme ? "#141622" : "#F8FAFC"
    property color cardColor: darkTheme ? "#1C2030" : "#FFFFFF"
    property color borderColor: darkTheme ? "#2F3447" : "#D9E2EC"
    property color textColor: darkTheme ? "#E6E1E8" : "#0F172A"
    property color mutedTextColor: darkTheme ? "#9A8FA2" : "#5F6B73"
    property color primaryColor: darkTheme ? "#B08FA8" : "#C9778F"
    property color subtleColor: darkTheme ? "#22283A" : "#F1F5F9"

    function themeColorsObj() {
        return {
            "card": root.cardColor,
            "border": root.borderColor,
            "text": root.textColor,
            "muted": root.mutedTextColor,
            "primary": root.primaryColor,
            "subtle": root.subtleColor
        }
    }

    color: bgColor
    palette.window: bgColor
    palette.windowText: textColor
    palette.base: cardColor
    palette.alternateBase: subtleColor
    palette.text: textColor
    palette.button: subtleColor
    palette.buttonText: textColor
    palette.highlight: primaryColor
    palette.highlightedText: darkTheme ? "#141622" : "#FFFFFF"
    palette.placeholderText: mutedTextColor
    palette.mid: borderColor

    Dialog {
        id: projectForImportDialog
        title: "选择导入项目"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 420

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            ComboBox {
                id: projectCombo
                Layout.fillWidth: true
                model: root.appCtx.projects.projectNames(false)
            }
            TextField {
                id: newProjectField
                Layout.fillWidth: true
                placeholderText: "或新建项目"
            }
        }

        onAccepted: {
            const created = newProjectField.text.trim()
            if (created.length > 0) {
                root.appCtx.projects.addProject(created)
                root.activeProjectForImport = created
            } else {
                root.activeProjectForImport = projectCombo.currentText
            }
            fileDialog.open()
            newProjectField.clear()
        }
    }

    FileDialog {
        id: fileDialog
        title: "选择立创导出文件"
        nameFilters: ["Spreadsheet Files (*.xlsx *.xls *.csv)", "All Files (*.*)"]
        onAccepted: root.appCtx.importLichuang(selectedFile, root.activeProjectForImport)
    }

    Dialog {
        id: inputDialog
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        property string mode: ""

        ColumnLayout {
            anchors.fill: parent
            TextField { id: dialogInput; Layout.fillWidth: true; placeholderText: "请输入名称" }
        }

        onAccepted: {
            const value = dialogInput.text.trim()
            if (mode === "newProject") root.appCtx.projects.addProject(value)
            if (mode === "renameProject") root.appCtx.projects.renameProject(root.renameProjectIndex, value)
            if (mode === "newCategory") root.appCtx.categories.addCategory(value)
            if (mode === "renameCategory") root.appCtx.categories.renameCategory(root.renameCategoryIndex, value)
            dialogInput.clear()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        SidebarPane {
            Layout.preferredWidth: 340
            Layout.fillHeight: true
            app: root.appCtx
            pinnedTopMost: root.pinnedTopMost
            themeColors: root.themeColorsObj()
            onTogglePinned: root.pinnedTopMost = !root.pinnedTopMost
            onRequestImport: projectForImportDialog.open()
            onRequestNewProject: {
                inputDialog.title = "新建项目"
                inputDialog.mode = "newProject"
                inputDialog.open()
            }
            onRequestRenameProject: function(index, currentName) {
                if (index <= 0 || currentName === "全部项目") {
                    root.appCtx.notify("请先选择一个具体项目再重命名。")
                    return
                }
                root.renameProjectIndex = index
                inputDialog.title = "重命名项目"
                inputDialog.mode = "renameProject"
                dialogInput.text = currentName
                inputDialog.open()
            }
            onRequestNewCategory: {
                inputDialog.title = "新增分类组"
                inputDialog.mode = "newCategory"
                inputDialog.open()
            }
            onRequestRenameCategory: function(index, currentName) {
                if (index < 0) {
                    root.appCtx.notify("请先选择要修改的分类组。")
                    return
                }
                root.renameCategoryIndex = index
                inputDialog.title = "修改分类组"
                inputDialog.mode = "renameCategory"
                dialogInput.text = currentName
                inputDialog.open()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: root.cardColor
            border.color: root.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    radius: 12
                    color: "transparent"
                    border.color: "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 8

                        Rectangle {
                            Layout.preferredWidth: 290
                            Layout.minimumWidth: 290
                            Layout.maximumWidth: 290
                            Layout.preferredHeight: 42
                            radius: 12
                            color: root.cardColor
                            border.color: root.borderColor

                            TabBar {
                                id: tabs
                                anchors.fill: parent
                                anchors.margins: 0
                                anchors.topMargin: 3
                                anchors.bottomMargin: -1
                                spacing: 6
                                padding: 0
                                background: Rectangle {
                                    radius: 10
                                    color: "transparent"
                                }

                                TabButton {
                                    text: "BOM 视图"
                                    height: tabs.height
                                    implicitWidth: 136
                                    implicitHeight: 36
                                    topPadding: 0
                                    bottomPadding: 0
                                    leftPadding: 0
                                    rightPadding: 0
                                    background: Rectangle {
                                        anchors.fill: parent
                                        radius: 10
                                        antialiasing: true
                                        color: tabs.currentIndex === 0 ? Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.18) : "transparent"
                                        border.color: tabs.currentIndex === 0 ? root.primaryColor : "transparent"
                                        border.width: tabs.currentIndex === 0 ? 1 : 0
                                    }
                                    contentItem: Text {
                                        text: "BOM 视图"
                                        color: root.textColor
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 14
                                        font.bold: tabs.currentIndex === 0
                                    }
                                }

                                TabButton {
                                    text: "差异分析"
                                    height: tabs.height
                                    implicitWidth: 136
                                    implicitHeight: 36
                                    topPadding: 0
                                    bottomPadding: 0
                                    leftPadding: 0
                                    rightPadding: 0
                                    background: Rectangle {
                                        anchors.fill: parent
                                        radius: 10
                                        antialiasing: true
                                        color: tabs.currentIndex === 1 ? Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.18) : "transparent"
                                        border.color: tabs.currentIndex === 1 ? root.primaryColor : "transparent"
                                        border.width: tabs.currentIndex === 1 ? 1 : 0
                                    }
                                    contentItem: Text {
                                        text: "差异分析"
                                        color: root.textColor
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 14
                                        font.bold: tabs.currentIndex === 1
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            radius: 12
                            color: root.cardColor
                            border.color: root.borderColor

                            TextField {
                                id: globalSearch
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                placeholderText: "全文搜索（料号/位号/规格/备注）"
                                color: root.textColor
                                placeholderTextColor: root.mutedTextColor
                                verticalAlignment: TextInput.AlignVCenter
                                background: Item {}
                                onTextChanged: root.appCtx.bomModel.setFilterKeyword(text)
                            }
                        }

                        AppButton {
                            themeColors: root.themeColorsObj()
                            text: "清空"
                            font.pixelSize: 14
                            cornerRadius: 10
                            implicitHeight: 42
                            implicitWidth: 78
                            onClicked: {
                                globalSearch.clear()
                                root.appCtx.bomModel.setFilterKeyword("")
                            }
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: tabs.currentIndex

                    BomPane {
                        app: root.appCtx
                        themeColors: root.themeColorsObj()
                    }

                    Rectangle {
                        color: root.cardColor
                        border.color: root.borderColor
                        radius: 12
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8
                            Label { text: "差异分析"; font.bold: true; color: root.textColor }
                            Label { text: "后续接入版本对比、替代料推荐、成本变化趋势。"; color: root.mutedTextColor }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    radius: 12
                    color: root.subtleColor
                    border.color: root.borderColor
                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        verticalAlignment: Text.AlignVCenter
                        text: root.appCtx.status
                        color: root.textColor
                    }
                }
            }
        }
    }
}
