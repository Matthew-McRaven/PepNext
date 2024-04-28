

/*
 * Copyright (c) 2023-2024 J. Stanley Warford, Matthew McRaven
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels
import "qrc:/ui/about" as About
import "qrc:/ui/help" as Help
import "qrc:/ui/memory/hexdump" as Memory
import "qrc:/ui/cpu" as Cpu
import "qrc:/ui/text/editor" as Editor
import "qrc:/ui/project" as Project
import "qrc:/ui/preferences" as Pref
import edu.pepp 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Pepp IDE")

    property variant currentProject: null
    property string mode: "welcome"
    function switchToProject(index) {
        console.log(`project: switching to ${index}`)
        projectBar.currentIndex = index
    }
    function closeProject(index) {
        console.log(`project: closed ${index}`)
        // TODO: add logic to save project before closing or reject change entirely.
        pm.removeRows(index, 1)
    }

    Component.onCompleted: {
        // Allow welcome mode to create a new project, and switch to it on creation.
        welcome.addProject.connect(pm.onAddProject)
        welcome.addProject.connect(() => switchToProject(pm.count - 1))
    }

    ProjectModel {
        id: pm
        function onAddProject(arch, level, feats) {
            // Attach a delegate to the project which can render its edit/debug modes. Since it is a C++ property,
            // binding changes propogate automatically.
            pm.pep10ISA(pep10isaComponent) // C++
        }
    }
    ListModel {
        id: defaultModel
        ListElement {
            display: "Welcome"
        }
    }

    // Provide a default font for menu items.
    FontMetrics {
        id: menuFont
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Menu {
                title: "nested"
                Action {
                    text: qsTr("&Save Object")
                }
                Action {
                    text: qsTr("Save Object &As...")
                }
            }

            Action {
                text: qsTr("&New...")
            }
            Action {
                text: qsTr("&Open...")
            }

            MenuSeparator {}
            Action {
                text: qsTr("&Quit")
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action {
                text: qsTr("Cu&t")
            }
            Action {
                text: qsTr("&Copy")
            }
            Action {
                text: qsTr("&Paste")
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("&About")
                onTriggered: aboutDialog.open()
            }
            Action {
                text: qsTr("Preferences")
                onTriggered: preferencesDialog.open()
            }
        }
    }
    Item {
        // Intersection of header and mode select.
        // Make transparent, influenced by Qt Creator Style.
        id: headerSpacer
        anchors.top: parent.top
        anchors.left: parent.left
        width: sidebar.width
        height: header.height
    }
    Item {
        id: header
        anchors.top: parent.top
        anchors.left: headerSpacer.right
        anchors.right: parent.right
        // Must explicitly set height to avoid binding loop; only account for tab bar if visibile.
        height: toolbar.height + (projectSelect.visible ? projectSelect.height : 0)
        ToolBar {
            id: toolbar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    text: qsTr("‹")
                    font: menuFont.font
                }
                Label {
                    text: "Title"
                    font: menuFont.font
                    elide: Label.ElideRight
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                }
                ToolButton {
                    text: qsTr("⋮")
                    font: menuFont.font
                }
                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                }
            }
        }

        Flickable {
            id: projectSelect
            clip: true
            visible: pm.count > 0
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: toolbar.bottom
            contentWidth: projectBar.width + addProjectButton.width
            contentHeight: projectBar.height
            height: contentHeight
            // Use a row layout to handle proper sizing of tab bar and buttons.
            Row {
                TabBar {
                    id: projectBar
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onCurrentIndexChanged: {
                        // I don't want the + be current. Removing a selected item from the model seems to
                        // select the previous index rather than the next.
                        window.currentProject = pm.data(
                                    pm.index(currentIndex, 0),
                                    ProjectModel.ProjectRole)
                    }
                    Repeater {
                        model: pm
                        anchors.fill: parent
                        delegate: TabButton {
                            // required property bool isPlus
                            text: index
                            font: menuFont.font
                            // TODO: Set to equal the width of the text + 2 spaces.
                            width: Math.max(100, projectSelect.width / 6)
                            Button {
                                text: "X"
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                height: Math.max(20, parent.height - 2)
                                width: height
                                onClicked: window.closeProject(index)
                            }
                        }
                    }
                }
                Button {
                    id: addProjectButton
                    text: "+"
                    Layout.fillHeight: true
                    width: plusTM.width
                    font: menuFont.font
                    height: projectBar.height // Border may clip into main area if unset.
                    TextMetrics {
                        id: plusTM
                        font: addProjectButton.font
                        text: `+${' '.repeat(10)}`
                    }
                    background: Rectangle {
                        color: "transparent"
                        // TODO: use "theme"'s hover color.
                        border.color: addProjectButton.hovered ? "blue" : "transparent"
                        border.width: 2
                        radius: 4
                    }
                    onClicked: {
                        // TODO: Use window.currentProject.env to create a new project with same features.
                        pm.onAddProject("pep/10", "isa3", "")
                        window.switchToProject(pm.count - 1)
                    }
                }
            }
        }
    }

    Column {
        id: sidebar
        anchors.top: headerSpacer.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 100
        Repeater {
            // If there is no current project, display a Welcome mode.
            model: window.currentProject ? window.currentProject.modes(
                                               ) : defaultModel
            delegate: Project.SideButton {
                text: model.display ?? "ERROR"
                Component.onCompleted: {
                    // Triggers window.modeChanged, which will propogate to all relevant components.
                    onClicked.connect(() => {
                                          window.mode = text.toLowerCase()
                                      })
                }
            }
        }
    }
    // Make sidebar buttons mutually-exclusive.
    ButtonGroup {
        buttons: sidebar.children
    }

    StackLayout {
        id: mainArea
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: sidebar.right
        Project.Welcome {
            id: welcome
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Help.HelpView {
            id: help
            Layout.fillHeight: true
            Layout.fillWidth: true
            abstraction: currentProject?.abstraction ?? Abstraction.NONE
            architecture: currentProject?.architecture ?? Architecture.NONE
        }
        Loader {
            id: projectLoader
            Layout.fillHeight: true
            Layout.fillWidth: true
            // TODO: Will need to switch to "source" with magic for passing mode & model.
            sourceComponent: currentProject?.delegate ?? null
            property alias model: window.currentProject
        }
        Component.onCompleted: {
            window.modeChanged.connect(onModeChanged)
            onModeChanged()
        }
        function onModeChanged() {
            console.log(`Changed to ${window.mode}`)
            switch (window.mode.toLowerCase()) {
            case "welcome":
                mainArea.currentIndex = 0
                break
            case "help":
                mainArea.currentIndex = 1
                break
            default:
                mainArea.currentIndex = 2
                // TODO: update loader delegate for selected mode.
                break
            }
        }
    }

    // Helpers to render central component via Loader.
    Component {
        id: pep10isaComponent
        Project.Pep10ISA {
            anchors.fill: parent
            mode: window.mode
        }
    }


    /*
     * Top-level dialogs
     */
    About.AboutDialog {
        id: aboutDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }
    Dialog {
        id: preferencesDialog
        title: qsTr("Preferences")
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        width: 700 // TODO: prevent binding loop on preferences size.
        height: 700
        contentItem: Pref.Preferences {
            id: prefs
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.header.bottom
            anchors.bottom: parent.footer.top
        }
        standardButtons: Dialog.Close
    }
}
