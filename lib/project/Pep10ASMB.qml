import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/edu/pepp/text/editor" as Text
import "qrc:/edu/pepp/memory/stack" as Stack
import "qrc:/edu/pepp/memory/hexdump" as Memory
import "qrc:/edu/pepp/memory/io" as IO
import "qrc:/edu/pepp/cpu" as Cpu
import "qrc:/edu/pepp/symtab" as SymTab
import edu.pepp 1.0

Item {
    id: wrapper
    required property var project
    required property var actions
    required property string mode
    NuAppSettings {
        id: settings
    }

    Component.onCompleted: {
        // Must connect and disconnect manually, otherwise project may be changed underneath us, and "save" targets wrong project.
        // Do not need to update on mode change, since mode change implies loss of focus of objEdit.
        userAsmEdit.editingFinished.connect(save)
        osAsmEdit.editingFinished.connect(save)
        project.errorsChanged.connect(displayErrors)
        project.listingChanged.connect(fixListings)
        onProjectChanged.connect(fixListings)
        project.charInChanged.connect(() => batchInput.setInput(project.charIn))
        // Connect editor BPs to project
        userAsmEdit.editor.modifyLine.connect(project.onModifyUserSource)
        osAsmEdit.editor.modifyLine.connect(project.onModifyOSSource)
        userList.editor.modifyLine.connect(project.onModifyUserList)
        osList.editor.modifyLine.connect(project.onModifyOSList)
        // Connect project to editors
        project.overwriteEditors.connect(onOverwriteEditors)
        project.modifyUserSource.connect(userAsmEdit.editor.onLineAction)
        project.modifyOSSource.connect(osAsmEdit.editor.onLineAction)
        project.modifyUserList.connect(userList.editor.onLineAction)
        project.modifyOSList.connect(osList.editor.onLineAction)
        // Update breakpoints on switch
        project.clearListingBreakpoints.connect(
                    userList.editor.onClearAllBreakpoints)
        project.clearListingBreakpoints.connect(
                    osList.editor.onClearAllBreakpoints)
        project.requestSourceBreakpoints.connect(
                    userAsmEdit.editor.onRequestAllBreakpoints)
        project.requestSourceBreakpoints.connect(
                    osAsmEdit.editor.onRequestAllBreakpoints)
        project.switchTo.connect(wrapper.onSwitchTo)
        if (project)
            fixListings()
        // Can't modify our mode directly because it would break binding with parent.
        // i.e., we can't be notified if editor is entered ever again.
        wrapper.actions.debug.start.triggered.connect(
                    wrapper.requestModeSwitchToDebugger)
        wrapper.actions.build.execute.triggered.connect(
                    wrapper.requestModeSwitchToDebugger)
        onOverwriteEditors()
    }
    // Will be called before project is changed on unload, so we can disconnect save-triggering signals.
    Component.onDestruction: {
        userAsmEdit.editingFinished.disconnect(save)
        osAsmEdit.editingFinished.disconnect(save)
        if (project) {
            userAsmEdit.editor.modifyLine.disconnect(project.onModifyUserSource)
            osAsmEdit.editor.modifyLine.disconnect(project.onModifyOSSource)
            userList.editor.modifyLine.disconnect(project.onModifyUserList)
            osList.editor.modifyLine.disconnect(project.onModifyOSList)
            project.errorsChanged.disconnect(displayErrors)
            project.listingChanged.connect(fixListings)
            project.overwriteEditors.disconnect(onOverwriteEditors)
            project.modifyUserSource.disconnect(userAsmEdit.editor.onLineAction)
            project.modifyOSSource.disconnect(osAsmEdit.editor.onLineAction)
            project.modifyUserList.disconnect(userList.editor.onLineAction)
            project.modifyOSList.disconnect(osList.editor.onLineAction)
            project.clearListingBreakpoints.disconnect(
                        userList.editor.onClearAllBreakpoints)
            project.clearListingBreakpoints.disconnect(
                        osList.editor.onClearAllBreakpoints)
            project.requestSourceBreakpoints.disconnect(
                        userAsmEdit.editor.onRequestAllBreakpoints)
            project.requestSourceBreakpoints.disconnect(
                        osAsmEdit.editor.onRequestAllBreakpoints)
            project.switchTo.disconnect(wrapper.onSwitchTo)
        }
        onProjectChanged.disconnect(fixListings)

        wrapper.actions.debug.start.triggered.disconnect(
                    wrapper.requestModeSwitchToDebugger)
        wrapper.actions.build.execute.triggered.disconnect(
                    wrapper.requestModeSwitchToDebugger)
    }
    signal requestModeSwitchTo(string mode)
    function requestModeSwitchToDebugger() {
        wrapper.requestModeSwitchTo("debugger")
    }
    function getLexerLangauge() {
        switch (project?.architecture) {
        case Architecture.PEP9:
            return "Pep/9 ASM"
        case Architecture.PEP10:
            return "Pep/10 ASM"
        default:
            return "Pep/10 ASM"
        }
    }

    function onSwitchTo(os) {
        textSelector.currentIndex = Qt.binding(() => os ? 1 : 0)
    }

    function displayErrors() {
        userAsmEdit.addEOLAnnotations(project.assemblerErrors)
    }
    function fixListings() {
        if (!project)
            return
        if (userList) {
            const curURO = userList.readOnly
            userList.readOnly = false
            userList.text = project.userList ?? ""
            userList.addListingAnnotations(project.userListAnnotations)
            userList.readOnly = curURO
        }
        if (osList) {
            const curORO = osList.readOnly
            osList.readOnly = false
            osList.text = project.osList
            osList.addListingAnnotations(project.osListAnnotations)
            osList.readOnly = curORO
        }
    }
    // TODO: replace preAssemble someday...
    function syncEditors() {
        save()
    }
    function save() {
        // Supress saving messages when there is no project.
        if (project === null)
            return
        if (!userAsmEdit.readOnly) {
            project.userAsmText = userAsmEdit.text
        }
        if (!osAsmEdit.readOnly) {
            project.osAsmText = osAsmEdit.text
        }
    }

    function preAssemble() {
        if (project === null)
            return
        project.userAsmText = userAsmEdit.text
        project.osAsmText = osAsmEdit.text
    }
    function onOverwriteEditors() {
        osAsmEdit.readOnly = false
        userAsmEdit.text = project?.userAsmText ?? ""
        osAsmEdit.text = project?.osAsmText ?? ""
        osAsmEdit.readOnly = Qt.binding(
                    () => !project.abstraction === Abstraction.OS4)
    }

    SplitView {
        id: split
        anchors.fill: parent
        orientation: Qt.Horizontal
        handle: Item {
            implicitWidth: 4
            Rectangle {
                implicitWidth: 4
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height
                // TODO: add color for handle
                color: palette.base
            }
        }

        Item {
            SplitView.minimumWidth: 100
            SplitView.fillWidth: true
            ComboBox {
                id: textSelector
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                model: ["User", "OS"]
            }
            FontMetrics {
                id: editorFM
                font: settings.extPalette.baseMono.font
            }

            SplitView {
                orientation: Qt.Vertical
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: textSelector.bottom
                anchors.bottom: parent.bottom
                StackLayout {
                    visible: mode == "editor"
                    currentIndex: textSelector.currentIndex
                    SplitView.fillHeight: true
                    Text.ScintillaAsmEdit {
                        id: userAsmEdit
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        height: parent.height
                        editorFont: editorFM.font
                        language: wrapper.getLexerLangauge()
                    }
                    Text.ScintillaAsmEdit {
                        id: osAsmEdit
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        height: parent.height
                        editorFont: editorFM.font
                        language: wrapper.getLexerLangauge()
                    }
                }
                StackLayout {
                    visible: mode == "debugger"
                    currentIndex: textSelector.currentIndex
                    SplitView.fillHeight: true
                    Text.ScintillaAsmEdit {
                        id: userList
                        readOnly: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        height: parent.height
                        text: project?.userList ?? ""
                        editorFont: editorFM.font
                        language: wrapper.getLexerLangauge()
                    }
                    Text.ScintillaAsmEdit {
                        id: osList
                        readOnly: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        height: parent.height
                        text: project?.osList ?? ""
                        editorFont: editorFM.font
                        language: wrapper.getLexerLangauge()
                    }
                }
                ColumnLayout {
                    visible: mode == "debugger"
                    SplitView.minimumHeight: 100
                    TabBar {
                        Layout.fillWidth: true
                        Layout.fillHeight: false
                        id: debugTabBar
                        visible: mode == "debugger"
                        TabButton {
                            text: qsTr("Object Code")
                        }
                        TabButton {
                            text: qsTr(`Symbol Table: ${textSelector.currentText}`)
                        }
                    }
                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: debugTabBar.currentIndex
                        clip: true
                        Text.ObjTextEditor {
                            id: objView
                            readOnly: true
                            // text is only an initial binding, the value diverges from there.
                            text: project?.objectCodeText ?? ""
                        }
                        SymTab.SymbolViewer {
                            id: symTab
                            model: textSelector.currentIndex
                                   === 0 ? project?.userSymbols : project?.osSymbols
                        }
                    }
                }
            }
        }

        SplitView {
            visible: mode === "debugger"
            SplitView.minimumWidth: Math.max(registers.implicitWidth,
                                             batchInput.implicitWidth,
                                             batchOutput.implicitWidth) + 20
            orientation: Qt.Vertical
            Cpu.RegisterView {
                id: registers
                SplitView.minimumHeight: registers.implicitHeight + 20
                SplitView.maximumHeight: registers.implicitHeight + 20
                registers: project?.registers ?? null
                flags: project?.flags ?? null
            }
            IO.Labeled {
                SplitView.minimumHeight: batchInput.minimumHeight
                SplitView.preferredHeight: (parent.height - registers.height) / 2
                id: batchInput
                width: parent.width
                label: "Input"
                property bool ignoreTextChange: false
                Component.onCompleted: {
                    onTextChanged.connect(() => {
                                              if (!ignoreTextChange)
                                              project.charIn = text
                                          })
                }
                function setInput(input) {
                    ignoreTextChange = true
                    batchInput.text = input
                    ignoreTextChange = false
                }
            }
            IO.Labeled {
                SplitView.minimumHeight: batchOutput.minimumHeight
                SplitView.preferredHeight: (parent.height - registers.height) / 2
                id: batchOutput
                width: parent.width
                label: "Output"
                text: project?.charOut ?? ""
            }
        }
        Item {
            SplitView.minimumWidth: 340
            visible: mode === "debugger"
            TabBar {
                id: memoryTab
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                TabButton {
                    text: qsTr("Hex Dump")
                }
                TabButton {
                    visible: settings.general.showDebugComponents
                    enabled: visible
                    text: qsTr("Stack Trace")
                }
            }
            StackLayout {
                anchors {
                    top: memoryTab.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                currentIndex: memoryTab.currentIndex
                Loader {
                    id: loader
                    Component.onCompleted: {
                        const props = {
                            "memory": project.memory,
                            "mnemonics": project.mnemonics
                        }
                        // Construction sets current address to 0, which propogates back to project.
                        // Must reject changes in current address until component is fully rendered.
                        con.enabled = false
                        setSource("qrc:/edu/pepp/memory/hexdump/MemoryDump.qml",
                                  props)
                    }
                    asynchronous: true
                    onLoaded: {
                        loader.item.scrollToAddress(project.currentAddress)
                        con.enabled = true
                    }
                }
                Stack.StackTrace {}
            }
        }
    }
    Connections {
        id: con
        enabled: false
        target: loader.item
        function onCurrentAddressChanged() {
            project.currentAddress = loader.item.currentAddress
        }
    }
}
