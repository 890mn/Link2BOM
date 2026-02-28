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
    property bool debugPanelVisible: false
    property var debugEntries: []
    property string debugLogText: ""
    property int debugLogLimit: 500
    property bool showInfoLogs: true
    property bool showWarningLogs: true
    property bool showErrorLogs: true

    function rebuildDebugLogText() {
        const lines = []
        for (let index = 0; index < debugEntries.length; ++index) {
            const entry = debugEntries[index]
            const visible = (entry.level === "INFO" && showInfoLogs)
                || (entry.level === "WARNING" && showWarningLogs)
                || (entry.level === "ERROR" && showErrorLogs)
            if (visible) {
                lines.push("[" + entry.time + "][" + entry.level + "] " + entry.message)
            }
        }
        debugLogText = lines.join("\n")
    }

    function appendDebugLog(level, message) {
        if (message === undefined) {
            message = level
            level = "INFO"
        }
        const stamp = Qt.formatDateTime(new Date(), "hh:mm:ss")
        const normalized = String(level).toUpperCase()
        const safeLevel = (normalized === "WARNING" || normalized === "ERROR") ? normalized : "INFO"
        const next = debugEntries.slice()
        next.push({
            "time": stamp,
            "level": safeLevel,
            "message": String(message)
        })
        if (next.length > debugLogLimit) {
            next.splice(0, next.length - debugLogLimit)
        }
        debugEntries = next
        rebuildDebugLogText()
    }

    function logInfo(message) { appendDebugLog("INFO", message) }
    function logWarning(message) { appendDebugLog("WARNING", message) }
    function logError(message) { appendDebugLog("ERROR", message) }

    onShowInfoLogsChanged: rebuildDebugLogText()
    onShowWarningLogsChanged: rebuildDebugLogText()
    onShowErrorLogsChanged: rebuildDebugLogText()
    onDebugEntriesChanged: rebuildDebugLogText()
    onDebugPanelVisibleChanged: {
        if (debugPanelVisible) {
            logInfo("Debug panel opened")
        } else {
            logInfo("Debug panel closed")
        }
    }

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
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnEscape
        padding: 14

        background: Rectangle {
            radius: 14
            color: root.cardColor
            border.color: root.borderColor
        }

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
                root.logInfo("Create project for import: " + created)
            } else {
                root.activeProjectForImport = projectCombo.currentText
                root.logInfo("Use existing project for import: " + root.activeProjectForImport)
            }
            root.logInfo("Open file picker for BOM import")
            fileDialog.open()
            newProjectField.clear()
        }
    }

    FileDialog {
        id: fileDialog
        title: "选择立创导出文件"
        nameFilters: ["Spreadsheet Files (*.xlsx *.xls *.csv)", "All Files (*.*)"]
        onAccepted: {
            root.logInfo("Import file selected: " + selectedFile.toString())
            root.appCtx.importLichuang(selectedFile, root.activeProjectForImport)
        }
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
            if (mode === "newProject") {
                root.logInfo("New project: " + value)
                root.appCtx.projects.addProject(value)
            }
            if (mode === "renameProject") {
                root.logInfo("Rename project index " + root.renameProjectIndex + " -> " + value)
                root.appCtx.projects.renameProject(root.renameProjectIndex, value)
            }
            if (mode === "newCategory") {
                root.logInfo("New category: " + value)
                root.appCtx.categories.addCategory(value)
            }
            if (mode === "renameCategory") {
                root.logInfo("Rename category index " + root.renameCategoryIndex + " -> " + value)
                root.appCtx.categories.renameCategory(root.renameCategoryIndex, value)
            }
            dialogInput.clear()
        }
    }

    Connections {
        target: root.appCtx
        function onStatusChanged() {
            const statusText = root.appCtx.status
            const lower = statusText.toLowerCase()
            if (lower.includes("error") || lower.includes("failed")) {
                root.logError("Status: " + statusText)
            } else if (lower.includes("warning") || lower.includes("please")) {
                root.logWarning("Status: " + statusText)
            } else {
                root.logInfo("Status: " + statusText)
            }
        }
    }

    Connections {
        target: root.appCtx.theme
        function onCurrentIndexChanged() {
            root.logInfo("Theme changed: " + root.appCtx.theme.currentThemeName)
        }
    }

    Connections {
        target: root.appCtx.projects
        function onSelectedProjectChanged() {
            root.logInfo("Selected project changed: " + root.appCtx.projects.selectedProject)
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
            onTogglePinned: {
                root.pinnedTopMost = !root.pinnedTopMost
                root.logInfo("Toggle pin top-most: " + root.pinnedTopMost)
            }
            onToggleDebugPanel: {
                root.debugPanelVisible = !root.debugPanelVisible
            }
            onRequestImport: {
                root.logInfo("Request import dialog")
                projectForImportDialog.open()
            }
            onRequestNewProject: {
                root.logInfo("Request new project dialog")
                inputDialog.title = "新建项目"
                inputDialog.mode = "newProject"
                inputDialog.open()
            }
            onRequestRenameProject: function(index, currentName) {
                if (index <= 0 || currentName === "All Projects" || currentName === "全部项目") {
                    root.logWarning("Rename project rejected: no specific project selected")
                    root.appCtx.notify("请先选择一个具体项目再重命名。")
                    return
                }
                root.logInfo("Request rename project dialog: index " + index + ", name " + currentName)
                root.renameProjectIndex = index
                inputDialog.title = "重命名项目"
                inputDialog.mode = "renameProject"
                dialogInput.text = currentName
                inputDialog.open()
            }
            onRequestDeleteProject: function(index, currentName) {
                if (index <= 0 || currentName === "All Projects" || currentName === "全部项目") {
                    root.logWarning("Delete project rejected: no specific project selected")
                    root.appCtx.notify("请先选择一个具体项目再删除。")
                    return
                }
                root.logInfo("Delete project request: index " + index + ", name " + currentName)
                if (!root.appCtx.deleteProject(index)) {
                    root.logError("Delete project failed: " + currentName)
                }
            }
            onRequestNewCategory: {
                root.logInfo("Request new category dialog")
                inputDialog.title = "新增分类组"
                inputDialog.mode = "newCategory"
                inputDialog.open()
            }
            onRequestRenameCategory: function(index, currentName) {
                if (index < 0) {
                    root.logWarning("Rename category rejected: no category selected")
                    root.appCtx.notify("请先选择要修改的分类组。")
                    return
                }
                root.logInfo("Request rename category dialog: index " + index + ", name " + currentName)
                root.renameCategoryIndex = index
                inputDialog.title = "修改分类组"
                inputDialog.mode = "renameCategory"
                dialogInput.text = currentName
                inputDialog.open()
            }
            onRequestDeleteCategory: function(index, currentName) {
                if (index < 0) {
                    root.logWarning("Delete category rejected: no category selected")
                    root.appCtx.notify("请先选择要删除的分类组。")
                    return
                }
                root.logInfo("Delete category request: index " + index + ", name " + currentName)
                if (!root.appCtx.categories.removeCategory(index)) {
                    root.logError("Delete category failed: " + currentName)
                } else {
                    root.appCtx.notify("分类组已删除: " + currentName)
                }
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
                                onCurrentIndexChanged: {
                                    root.logInfo("View switched: " + (currentIndex === 0 ? "BOM" : "Diff"))
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
                                onTextChanged: {
                                    root.appCtx.bomModel.setFilterKeyword(text)
                                    root.logInfo("Global search changed: \"" + text + "\"")
                                }
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
                                root.logInfo("Global search cleared")
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
                        onDebugLog: function(level, message) {
                            root.appendDebugLog(level, "BOM: " + message)
                        }
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
                    visible: root.debugPanelVisible
                    Layout.fillWidth: true
                    Layout.preferredHeight: 220
                    radius: 12
                    color: root.subtleColor
                    border.color: root.borderColor

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                Layout.fillWidth: true
                                text: "Debug Console"
                                color: root.textColor
                                elide: Text.ElideRight
                                font.bold: true
                            }

                            CheckBox {
                                id: infoLevelCheck
                                text: "Info"
                                checked: root.showInfoLogs
                                implicitHeight: 28
                                implicitWidth: 74
                                leftPadding: 8
                                rightPadding: 8
                                onToggled: root.showInfoLogs = checked
                                background: Rectangle {
                                    radius: 8
                                    border.color: infoLevelCheck.checked ? root.primaryColor : root.borderColor
                                    color: infoLevelCheck.checked
                                        ? Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.14)
                                        : root.subtleColor
                                }
                                indicator: Rectangle {
                                    implicitWidth: 0
                                    implicitHeight: 0
                                    visible: false
                                }
                                contentItem: Text {
                                    text: infoLevelCheck.text
                                    color: root.textColor
                                    leftPadding: 0
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 12
                                }
                            }

                            CheckBox {
                                id: warningLevelCheck
                                text: "Warning"
                                checked: root.showWarningLogs
                                implicitHeight: 28
                                implicitWidth: 96
                                leftPadding: 8
                                rightPadding: 8
                                onToggled: root.showWarningLogs = checked
                                background: Rectangle {
                                    radius: 8
                                    border.color: warningLevelCheck.checked ? root.primaryColor : root.borderColor
                                    color: warningLevelCheck.checked
                                        ? Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.14)
                                        : root.subtleColor
                                }
                                indicator: Rectangle {
                                    implicitWidth: 0
                                    implicitHeight: 0
                                    visible: false
                                }
                                contentItem: Text {
                                    text: warningLevelCheck.text
                                    color: root.textColor
                                    leftPadding: 0
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 12
                                }
                            }

                            CheckBox {
                                id: errorLevelCheck
                                text: "Error"
                                checked: root.showErrorLogs
                                implicitHeight: 28
                                implicitWidth: 82
                                leftPadding: 8
                                rightPadding: 8
                                onToggled: root.showErrorLogs = checked
                                background: Rectangle {
                                    radius: 8
                                    border.color: errorLevelCheck.checked ? root.primaryColor : root.borderColor
                                    color: errorLevelCheck.checked
                                        ? Qt.rgba(root.primaryColor.r, root.primaryColor.g, root.primaryColor.b, 0.14)
                                        : root.subtleColor
                                }
                                indicator: Rectangle {
                                    implicitWidth: 0
                                    implicitHeight: 0
                                    visible: false
                                }
                                contentItem: Text {
                                    text: errorLevelCheck.text
                                    color: root.textColor
                                    leftPadding: 0
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 12
                                }
                            }

                            AppButton {
                                themeColors: root.themeColorsObj()
                                text: "Clear"
                                implicitHeight: 28
                                implicitWidth: 72
                                onClicked: {
                                    root.debugEntries = []
                                    root.debugLogText = ""
                                    root.logInfo("Debug logs cleared")
                                }
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            TextArea {
                                id: debugTextArea
                                readOnly: true
                                wrapMode: TextEdit.NoWrap
                                text: root.debugLogText
                                color: root.textColor
                                selectionColor: root.primaryColor
                                selectedTextColor: "#FFFFFF"
                                font.pixelSize: 12
                                background: Rectangle {
                                    color: Qt.rgba(root.cardColor.r, root.cardColor.g, root.cardColor.b, 0.65)
                                    radius: 8
                                    border.color: root.borderColor
                                }

                                onTextChanged: {
                                    cursorPosition = length
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
