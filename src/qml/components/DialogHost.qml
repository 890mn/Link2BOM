pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: root
    required property var app
    required property var themeColors
    required property color textColor
    required property color mutedTextColor
    required property color primaryColor
    required property color subtleColor
    required property color cardColor
    required property color borderColor
    required property string uiLanguage
    required property var textMap
    signal languageApplied(string language)
    signal inputAccepted(string mode, string value)

    function txSafe(key, fallback) {
        if (root.textMap && root.textMap[key] !== undefined) {
            return root.textMap[key]
        }
        return fallback
    }

    function openProjectImportDialog(mode) {
        importMode = mode === undefined ? "lcsc" : mode
        projectForImportDialog.open()
    }

    function openInputDialog(mode, titleText, currentText) {
        inputDialog.mode = mode
        inputDialog.titleText = titleText
        dialogInput.text = currentText === undefined ? "" : currentText
        inputDialog.open()
    }

    function openSettingsDialog() {
        settingsDialog.open()
    }

    function openArchiveDialog() {
        refreshArchiveSlots()
        archiveDialog.open()
    }

    function refreshArchiveSlots() {
        archiveSlots = root.app.archive.listSlots()
        if (activeArchiveIndex < 0 || activeArchiveIndex >= archiveSlots.length) {
            activeArchiveIndex = 0
        }
    }

    function defaultArchiveName(index) {
        if (index <= 0) {
            return ""
        }
        return root.uiLanguage === "en-US"
            ? "Save" + index
            : "存档" + index
    }

    function archiveTitle(slotData) {
        if (!slotData) {
            return ""
        }
        if (slotData.hasData || slotData.index === 0) {
            return slotData.title
        }
        return defaultArchiveName(slotData.index)
    }

    function defaultArchiveDir() {
        if (!archiveSlots || archiveSlots.length === 0) {
            return ""
        }
        const path = String(archiveSlots[0].path || "")
        const slash = Math.max(path.lastIndexOf("/"), path.lastIndexOf("\\"))
        return slash > 0 ? path.slice(0, slash) : path
    }

    property string activeProjectForImport: ""
    property string importMode: "lcsc"
    property var archiveSlots: []
    property int activeArchiveIndex: 0

    Popup {
        id: projectForImportDialog
        modal: true
        focus: true
        width: 420
        implicitHeight: importDialogContent.implicitHeight + 20
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        background: Rectangle {
            radius: 12
            color: root.subtleColor
            border.color: root.borderColor
        }

        ColumnLayout {
            id: importDialogContent
            anchors.fill: parent
            spacing: 10

            Label {
                text: root.txSafe("dialog.selectImportProject", "Select Import Project")
                color: root.textColor
                font.bold: true
            }

            ComboBox {
                id: projectCombo
                Layout.fillWidth: true
                model: root.app.projects.model
                textRole: "display"
                implicitHeight: 36
                font.pixelSize: 13
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: 24
                    text: projectCombo.displayText
                    color: root.textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            TextField {
                id: newProjectField
                Layout.fillWidth: true
                placeholderText: root.txSafe("dialog.newProjectOr", "Or create new project")
                implicitHeight: 36
                color: root.textColor
                placeholderTextColor: root.mutedTextColor
                selectionColor: root.primaryColor
                selectedTextColor: "#FFFFFF"
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.cancel", "Cancel")
                    onClicked: projectForImportDialog.close()
                }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.ok", "OK")
                    accent: true
                    onClicked: {
                        const created = newProjectField.text.trim()
                        const target = created.length > 0 ? created : projectCombo.currentText
                        if (!target || target === "All Projects") {
                            root.app.notify(root.txSafe("warn.selectProject", "Please select a project"))
                            return
                        }
                        if (created.length > 0) {
                            root.app.projects.addProject(created)
                        }
                        root.activeProjectForImport = target
                        fileDialog.open()
                        newProjectField.clear()
                        projectForImportDialog.close()
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: importMode === "lcsc"
            ? root.txSafe("dialog.selectLichuangFile", "Select LCSC export file")
            : root.txSafe("dialog.selectGenericFile", "Select spreadsheet file")
        nameFilters: ["Spreadsheet Files (*.xlsx *.xls *.csv)", "All Files (*.*)"]
        onAccepted: {
            if (importMode === "lcsc") {
                root.app.io.importLichuang(selectedFile, root.activeProjectForImport)
            } else {
                root.app.io.importGeneric(selectedFile, root.activeProjectForImport)
            }
        }
    }

    FileDialog {
        id: exportFileDialog
        title: root.txSafe("dialog.selectExportCsvFile", "Select CSV export file")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "csv"
        nameFilters: ["CSV Files (*.csv)", "All Files (*.*)"]
        onAccepted: root.app.io.exportCsv(selectedFile)
    }

    function openExportDialog() {
        exportFileDialog.open()
    }

    Popup {
        id: archiveDialog
        modal: true
        focus: true
        width: 520
        implicitHeight: archiveContent.implicitHeight + 20
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        background: Rectangle {
            radius: 12
            color: root.subtleColor
            border.color: root.borderColor
        }

        ColumnLayout {
            id: archiveContent
            anchors.fill: parent
            spacing: 10

            Label {
                text: root.txSafe("archive.title", "Local Archives")
                color: root.textColor
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("archive.load", "Load")
                    Layout.fillWidth: true
                    onClicked: {
                        if (root.app.archive.loadSlot(activeArchiveIndex)) {
                            refreshArchiveSlots()
                        }
                    }
                }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("archive.save", "Save")
                    accent: true
                    Layout.fillWidth: true
                    onClicked: {
                        const defaultName = defaultArchiveName(activeArchiveIndex)
                        const label = archiveNameField.text.trim() || defaultName
                        const path = archivePathField.text.trim()
                        root.app.archive.saveSlot(activeArchiveIndex, label, path)
                        refreshArchiveSlots()
                        archiveNameField.clear()
                        archivePathField.clear()
                    }
                }
            }

            Label {
                text: root.txSafe("archive.name", "Save name")
                color: root.textColor
            }

            TextField {
                id: archiveNameField
                Layout.fillWidth: true
                placeholderText: defaultArchiveName(activeArchiveIndex)
                implicitHeight: 36
                color: root.textColor
                placeholderTextColor: root.mutedTextColor
                selectionColor: root.primaryColor
                selectedTextColor: "#FFFFFF"
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            Label {
                text: root.txSafe("archive.path", "Save path")
                color: root.textColor
            }

            TextField {
                id: archivePathField
                Layout.fillWidth: true
                placeholderText: defaultArchiveDir()
                implicitHeight: 36
                color: root.textColor
                placeholderTextColor: root.mutedTextColor
                selectionColor: root.primaryColor
                selectedTextColor: "#FFFFFF"
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            Repeater {
                model: archiveSlots
                delegate: Item {
                    id: slotRow
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 56

                    RowLayout {
                        anchors.fill: parent
                        spacing: 8

                        Rectangle {
                            id: slotCard
                            Layout.fillWidth: true
                            implicitHeight: 56
                            radius: 10
                            color: root.cardColor
                            border.color: activeArchiveIndex === slotRow.modelData.index ? root.primaryColor : root.borderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Label { text: archiveTitle(slotRow.modelData); color: root.textColor; font.bold: true; elide: Text.ElideRight }
                                    Item {
                                        id: subtitleClip
                                        Layout.fillWidth: true
                                        height: 16
                                        clip: true
                                        property bool hovered: subtitleHover.hovered
                                        property real overflow: Math.max(0, subtitleText.implicitWidth - width)

                                        Text {
                                            id: subtitleText
                                            text: slotRow.modelData.subtitle
                                            color: root.mutedTextColor
                                            font.pixelSize: 12
                                            elide: Text.ElideRight
                                            x: 0
                                            y: 0
                                        }

                                        SequentialAnimation {
                                            id: subtitleMarquee
                                            running: subtitleClip.hovered && subtitleClip.overflow > 0
                                            loops: Animation.Infinite
                                            NumberAnimation {
                                                target: subtitleText
                                                property: "x"
                                                from: 0
                                                to: -(subtitleClip.overflow + 12)
                                                duration: Math.max(1200, subtitleClip.overflow * 16)
                                                easing.type: Easing.InOutSine
                                            }
                                            PauseAnimation { duration: 700 }
                                            NumberAnimation {
                                                target: subtitleText
                                                property: "x"
                                                to: 0
                                                duration: 600
                                                easing.type: Easing.InOutQuad
                                            }
                                            PauseAnimation { duration: 500 }
                                        }

                                        HoverHandler {
                                            id: subtitleHover
                                            onHoveredChanged: {
                                                if (!hovered) {
                                                    subtitleText.x = 0
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: activeArchiveIndex = slotRow.modelData.index
                            }
                        }

                        Rectangle {
                            id: deleteCard
                            visible: slotRow.modelData.canDelete
                            Layout.preferredWidth: slotRow.modelData.canDelete ? 92 : 0
                            implicitHeight: 56
                            radius: 10
                            color: root.cardColor
                            border.color: root.borderColor

                            Label {
                                anchors.centerIn: parent
                                text: root.txSafe("common.delete", "Delete")
                                color: root.textColor
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (root.app.archive.deleteSlot(slotRow.modelData.index)) {
                                        refreshArchiveSlots()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        onOpened: refreshArchiveSlots()
    }

    Popup {
        id: inputDialog
        modal: true
        focus: true
        property string mode: ""
        property string titleText: ""
        width: 420
        implicitHeight: inputDialogContent.implicitHeight + 20
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        background: Rectangle {
            radius: 12
            color: root.subtleColor
            border.color: root.borderColor
        }

        ColumnLayout {
            id: inputDialogContent
            anchors.fill: parent
            spacing: 10

            Label {
                text: inputDialog.titleText
                color: root.textColor
                font.bold: true
            }

            TextField {
                id: dialogInput
                Layout.fillWidth: true
                placeholderText: root.txSafe("dialog.inputName", "Please enter a name")
                implicitHeight: 36
                color: root.textColor
                placeholderTextColor: root.mutedTextColor
                selectionColor: root.primaryColor
                selectedTextColor: "#FFFFFF"
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.cancel", "Cancel")
                    onClicked: inputDialog.close()
                }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.ok", "OK")
                    accent: true
                    onClicked: {
                        root.inputAccepted(inputDialog.mode, dialogInput.text.trim())
                        dialogInput.clear()
                        inputDialog.close()
                    }
                }
            }
        }
    }

    Popup {
        id: settingsDialog
        modal: true
        focus: true
        width: 340
        implicitHeight: settingsContent.implicitHeight + 20
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        parent: Overlay.overlay
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10
        property string pendingLanguage: root.uiLanguage

        background: Rectangle {
            radius: 12
            color: root.subtleColor
            border.color: root.borderColor
        }

        ColumnLayout {
            id: settingsContent
            anchors.fill: parent
            spacing: 10

            Label {
                text: root.txSafe("settings.title", "Settings")
                color: root.textColor
                font.bold: true
            }

            Label {
                text: root.txSafe("settings.language", "Language")
                color: root.textColor
            }

            ComboBox {
                id: languageCombo
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
                implicitHeight: 36
                font.pixelSize: 13
                model: [
                    { "label": root.txSafe("settings.lang.zh", "Chinese"), "value": "zh-CN" },
                    { "label": root.txSafe("settings.lang.en", "English"), "value": "en-US" }
                ]
                Component.onCompleted: currentIndex = root.uiLanguage === "en-US" ? 1 : 0
                onActivated: settingsDialog.pendingLanguage = currentValue
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: 24
                    text: languageCombo.displayText
                    color: root.textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    radius: 10
                    color: root.cardColor
                    border.color: root.borderColor
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.cancel", "Cancel")
                    onClicked: settingsDialog.close()
                }

                AppButton {
                    themeColors: root.themeColors
                    text: root.txSafe("common.ok", "OK")
                    accent: true
                    onClicked: {
                        root.languageApplied(settingsDialog.pendingLanguage)
                        settingsDialog.close()
                    }
                }
            }
        }

        onOpened: {
            languageCombo.currentIndex = root.uiLanguage === "en-US" ? 1 : 0
            pendingLanguage = root.uiLanguage
        }
    }
}
