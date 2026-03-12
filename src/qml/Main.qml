pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import "components"
import "I18n.js" as I18n

ApplicationWindow {
    id: root
    width: minimumWidth
    height: minimumHeight
    visible: true
    title: "Link2BOM"
    minimumWidth: 1100
    minimumHeight: 700

    required property var appCtx
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
    property string uiLanguage: uiSettings.language
    property var textMap: I18n.table(uiLanguage)
    property int activeTabIndex: 0
    property string bomSearchText: ""
    property string diffSearchText: ""
    property string diffGroupMode: "project"
    property string diffViewMode: "list"
    property var diffItems: []
    property var diffStats: ({})
    property bool syncingTopSearch: false
    function tx(key) {
        return I18n.translate(uiLanguage, key)
    }

    function escapeHtml(text) {
        return String(text)
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;")
    }

    function levelColor(level) {
        const key = String(level).toUpperCase()
        if (key === "ERROR") return "#EF4444"
        if (key === "WARNING") return "#F59E0B"
        return darkTheme ? "#93C5FD" : "#2563EB"
    }

    Settings {
        id: uiSettings
        category: "UiGeneral"
        property string language: "zh-CN"
    }

    function rebuildDebugLogText() {
        const lines = []
        for (let index = 0; index < debugEntries.length; ++index) {
            const entry = debugEntries[index]
            const visible = (entry.level === "INFO" && showInfoLogs)
                || (entry.level === "WARNING" && showWarningLogs)
                || (entry.level === "ERROR" && showErrorLogs)
            if (visible) {
                const color = levelColor(entry.level)
                const content = "[" + entry.time + "] [" + entry.level + "] [app] " + entry.message
                lines.push("<span style='color:" + color + "'>" + escapeHtml(content) + "</span>")
            }
        }
        debugLogText = lines.join("<br/>")
    }

    function addDebugEntry(time, level, message) {
        const normalized = String(level).toUpperCase()
        const safeLevel = (normalized === "WARNING" || normalized === "ERROR") ? normalized : "INFO"
        const next = debugEntries.slice()
        next.push({
            "time": time,
            "level": safeLevel,
            "message": String(message)
        })
        if (next.length > debugLogLimit) {
            next.splice(0, next.length - debugLogLimit)
        }
        debugEntries = next
        rebuildDebugLogText()
    }

    function appendDebugLog(level, message) {
        if (message === undefined) {
            message = level
            level = "INFO"
        }
        const normalized = String(level).toUpperCase()
        if (normalized === "WARNING") {
            root.appCtx.logWarning(String(message))
        } else if (normalized === "ERROR") {
            root.appCtx.logError(String(message))
        } else {
            root.appCtx.logInfo(String(message))
        }
    }

    function logInfo(message) { root.appCtx.logInfo(String(message)) }
    function logWarning(message) { root.appCtx.logWarning(String(message)) }
    function logError(message) { root.appCtx.logError(String(message)) }
    function refreshDiffAnalysis() {
        diffItems = root.appCtx.bomModel.analyzeDifferences(diffSearchText, diffGroupMode)
        diffStats = root.appCtx.bomModel.buildAnalytics(diffGroupMode)
    }

    onShowInfoLogsChanged: rebuildDebugLogText()
    onShowWarningLogsChanged: rebuildDebugLogText()
    onShowErrorLogsChanged: rebuildDebugLogText()
    onDebugEntriesChanged: rebuildDebugLogText()
    onUiLanguageChanged: uiSettings.language = uiLanguage
    onDebugPanelVisibleChanged: {
        if (debugPanelVisible) {
            logInfo("Debug panel opened")
            debugPopup.open()
        } else {
            logInfo("Debug panel closed")
            debugPopup.close()
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

    DialogHost {
        id: dialogHost
        anchors.fill: parent
        app: root.appCtx
        themeColors: root.themeColorsObj()
        textColor: root.textColor
        mutedTextColor: root.mutedTextColor
        primaryColor: root.primaryColor
        subtleColor: root.subtleColor
        cardColor: root.cardColor
        borderColor: root.borderColor
        uiLanguage: root.uiLanguage
        textMap: root.textMap
        onLanguageApplied: function(language) {
            root.uiLanguage = language
            root.logInfo("UI language changed: " + root.uiLanguage)
        }
        onInputAccepted: function(mode, value) {
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
        }
    }

    DebugConsole {
        id: debugPopup
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        themeColors: root.themeColorsObj()
        textColor: root.textColor
        borderColor: root.borderColor
        subtleColor: root.subtleColor
        cardColor: root.cardColor
        primaryColor: root.primaryColor
        debugLogText: root.debugLogText
        showInfoLogs: root.showInfoLogs
        showWarningLogs: root.showWarningLogs
        showErrorLogs: root.showErrorLogs
        textMap: root.textMap
        onClosed: root.debugPanelVisible = false
        onInfoLogsToggled: function(checked) { root.showInfoLogs = checked }
        onWarningLogsToggled: function(checked) { root.showWarningLogs = checked }
        onErrorLogsToggled: function(checked) { root.showErrorLogs = checked }
        onClearRequested: {
            root.debugEntries = []
            root.debugLogText = ""
            root.logInfo("Debug logs cleared")
        }
    }

    Connections {
        target: root.appCtx.logRelay
        function onEntryAdded(time, level, message) {
            root.addDebugEntry(time, level, message)
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

    Connections {
        target: root.appCtx.bomModel
        function onModelReset() {
            if (root.activeTabIndex === 1) {
                root.refreshDiffAnalysis()
            }
        }
        function onHeaderDataChanged() {
            if (root.activeTabIndex === 1) {
                root.refreshDiffAnalysis()
            }
        }
    }

    // 1ST REC
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
            uiLanguage: root.uiLanguage
            textMap: root.textMap
            onTogglePinned: {
                root.pinnedTopMost = !root.pinnedTopMost
                root.logInfo("Toggle pin top-most: " + root.pinnedTopMost)
            }
            onOpenSettings: {
                dialogHost.openSettingsDialog()
            }
            onToggleDebugPanel: {
                root.debugPanelVisible = !root.debugPanelVisible
            }
            onRequestImport: function(mode) {
                if (mode === "ocr") {
                    root.logWarning("OCR import is not ready")
                    root.appCtx.notify(root.tx("sidebar.import.ocr.todo"))
                    return
                }
                root.logInfo("Request import dialog: " + mode)
                dialogHost.openProjectImportDialog(mode)
            }
            onRequestExport: {
                root.logInfo("Request CSV export dialog")
                dialogHost.openExportDialog()
            }
            onOpenArchive: {
                root.logInfo("Open archive dialog")
                dialogHost.openArchiveDialog()
            }
            onRequestNewProject: {
                root.logInfo("Request new project dialog")
                dialogHost.openInputDialog("newProject", root.tx("dialog.newProject"), "")
            }
            onRequestRenameProject: function(index, currentName) {
                if (index <= 0 || currentName === "All Projects") {
                    root.logWarning("Rename project rejected: no specific project selected")
                    root.appCtx.notify(root.tx("warn.selectProjectRename"))
                    return
                }
                root.logInfo("Request rename project dialog: index " + index + ", name " + currentName)
                root.renameProjectIndex = index
                dialogHost.openInputDialog("renameProject", root.tx("dialog.renameProject"), currentName)
            }
            onRequestDeleteProject: function(index, currentName) {
                if (index <= 0 || currentName === "All Projects") {
                    root.logWarning("Delete project rejected: no specific project selected")
                    root.appCtx.notify(root.tx("warn.selectProjectDelete"))
                    return
                }
                root.logInfo("Delete project request: index " + index + ", name " + currentName)
                if (!root.appCtx.deleteProject(index)) {
                    root.logError("Delete project failed: " + currentName)
                }
            }
            onRequestNewCategory: {
                root.logInfo("Request new category dialog")
                dialogHost.openInputDialog("newCategory", root.tx("dialog.newCategory"), "")
            }
            onRequestRenameCategory: function(index, currentName) {
                if (index < 0) {
                    root.logWarning("Rename category rejected: no category selected")
                    root.appCtx.notify(root.tx("warn.selectCategoryRename"))
                    return
                }
                root.logInfo("Request rename category dialog: index " + index + ", name " + currentName)
                root.renameCategoryIndex = index
                dialogHost.openInputDialog("renameCategory", root.tx("dialog.renameCategory"), currentName)
            }
            onRequestDeleteCategory: function(index, currentName) {
                if (index < 0) {
                    root.logWarning("Delete category rejected: no category selected")
                    root.appCtx.notify(root.tx("warn.selectCategoryDelete"))
                    return
                }
                root.logInfo("Delete category request: index " + index + ", name " + currentName)
                if (!root.appCtx.categories.removeCategory(index)) {
                    root.logError("Delete category failed: " + currentName)
                } else {
                    root.appCtx.notify(root.tx("notify.categoryDeleted") + currentName)
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 50

                TopBar {
                    id: topBar
                    anchors.fill: parent
                    themeColors: root.themeColorsObj()
                    textColor: root.textColor
                    mutedTextColor: root.mutedTextColor
                    primaryColor: root.primaryColor
                    textMap: root.textMap
                    currentIndex: root.activeTabIndex
                    searchText: root.activeTabIndex === 0 ? root.bomSearchText : root.diffSearchText
                    onToggleDebugRequested: root.debugPanelVisible = !root.debugPanelVisible
                    onTabChanged: function(index) {
                        root.syncingTopSearch = true
                        root.activeTabIndex = index
                        root.syncingTopSearch = false
                        root.logInfo("View switched: " + (index === 0 ? "BOM" : "Diff"))
                        if (index === 1) {
                            root.refreshDiffAnalysis()
                        }
                    }
                    onSearchEdited: function(text) {
                        if (root.syncingTopSearch) {
                            return
                        }
                        if (root.activeTabIndex === 0) {
                            root.bomSearchText = text
                            root.appCtx.bomModel.setFilterKeyword(text)
                            root.logInfo("Global BOM search changed: \"" + text + "\"")
                        } else {
                            root.diffSearchText = text
                            root.refreshDiffAnalysis()
                            root.logInfo("Diff search changed: \"" + text + "\"")
                        }
                    }
                    onClearRequested: {
                        if (root.activeTabIndex === 0) {
                            root.bomSearchText = ""
                            root.appCtx.bomModel.setFilterKeyword("")
                            root.logInfo("Global BOM search cleared")
                        } else {
                            root.diffSearchText = ""
                            root.refreshDiffAnalysis()
                            root.logInfo("Diff search cleared")
                        }
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.activeTabIndex

                BomPane {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    app: root.appCtx
                    themeColors: root.themeColorsObj()
                    uiLanguage: root.uiLanguage
                    textMap: root.textMap
                    onDebugLog: function(level, message) {
                        root.appendDebugLog(level, "BOM: " + message)
                    }
                }

                DiffPane {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    themeColors: root.themeColorsObj()
                    textColor: root.textColor
                    mutedTextColor: root.mutedTextColor
                    primaryColor: root.primaryColor
                    subtleColor: root.subtleColor
                    darkTheme: root.darkTheme
                    textMap: root.textMap
                    groupMode: root.diffGroupMode
                    viewMode: root.diffViewMode
                    diffItems: root.diffItems
                    diffStats: root.diffStats
                    onGroupModeSelected: function(value) {
                        root.diffGroupMode = value
                        root.refreshDiffAnalysis()
                    }
                    onViewModeSelected: function(value) {
                        root.diffViewMode = value
                    }
                }
            }
        }
    }
}

