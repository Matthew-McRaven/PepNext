import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Labs
//  Native menu for apple, linux, and windows
import "./"

Labs.MenuBar {
    id: wrapper

    required property var project
    required property Actions actions
    property alias saveAsModel: saveAsInstantiator.model
    property alias printModel: printInstantiator.model
    property alias closeModel: closeInstantiator.model

    function indexOf(menu, menuItem) {
        for (var i = 0; i < menu.data.length; i++) {
            //  See if item is same as list
            if (menu.data[i] === menuItem) {
                return i
            }
        }
        return menu.data.length
    }
    Labs.Menu {
        id: fileMenu
        title: qsTr("&File")
        Labs.MenuItem {
            id: new_
            text: actions.file.new_.text
            onTriggered: actions.file.new_.trigger()
            icon.source: actions.file.new_.icon.source
            shortcut: actions.file.new_.shortcut
        }
        Labs.MenuItem {
            text: actions.file.open.text
            onTriggered: actions.file.open.trigger()
            icon.source: actions.file.open.icon.source
            shortcut: actions.file.open.shortcut
        }
        Labs.Menu {
            id: recentMenu
            title: "Recent Files"
            // Use blank icon to force menu items to line up. Do not use image provider for a Menu item, since
            // this icon is rendered before the image provider's paint engine is set up.
            icon.source: "qrc:/icons/blank.svg"

            // As such, the width of the icon may be wrong, so use the width of a different (working) icon.
            //icon.width: new_.icon.width
            Instantiator {
                model: 5
                delegate: Labs.MenuItem {
                    text: `${modelData}.pep`
                    onTriggered: openRecent(modelData)
                    icon.source: "image://icons/blank.svg"
                }

                onObjectAdded: (i, obj) => recentMenu.insertItem(i, obj)
                onObjectRemoved: (i, obj) => recentMenu.removeItem(obj)
            }
        }

        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.file.save.text
            onTriggered: actions.file.save.trigger()
            icon.source: actions.file.save.icon.source
            shortcut: actions.file.save.shortcut
        }
        Labs.MenuSeparator {
            id: _saveAsPrev
        }
        Instantiator {
            id: saveAsInstantiator
            model: 2
            delegate: Labs.MenuItem {
                text: "Save as " + modelData
                onTriggered: saveAs(modelData)
                // Use blank icon to force menu items to line up.
                icon.source: "image://icons/blank.svg"
            }
            onObjectAdded: function (i, obj) {
                const m = fileMenu
                //  Insert under parent
                m.insertItem(i + indexOf(m, _saveAsPrev) + 1, obj)
            }
            onObjectRemoved: (i, obj) => fileMenu.removeItem(obj)
        }
        Labs.MenuSeparator {
            id: _printPrev
        }
        Instantiator {
            id: printInstantiator
            model: 3
            delegate: Labs.MenuItem {
                text: "Print " + modelData
                onTriggered: actions.file.print_(modelData).trigger()
                // Use blank icon to force menu items to line up.
                icon.source: "image://icons/blank.svg"
            }
            onObjectAdded: function (i, obj) {
                const m = fileMenu
                //  Insert under parent, note that inserted item shifted list down by 1
                m.insertItem(i + indexOf(m, _printPrev), obj)
            }
            onObjectRemoved: (i, obj) => fileMenu.removeItem(obj)
        }
        Labs.MenuSeparator {
            id: _closePrev
        }
        Instantiator {
            id: closeInstantiator
            model: 3
            delegate: Labs.MenuItem {
                text: "Close " + modelData
                onTriggered: actions.file.close(modelData).trigger()
                // Use blank icon to force menu items to line up.
                icon.source: "image://icons/blank.svg"
            }
            onObjectAdded: function (i, obj) {
                const m = fileMenu
                //  Insert under parent, note that inserted items shifted list down by 2
                m.insertItem(i + indexOf(m, _closePrev) - 1, obj)
            }
            onObjectRemoved: (i, obj) => fileMenu.removeItem(obj)
        }
        MenuItem {
            text: actions.file.closeAll.text
            onTriggered: actions.file.closeAll.trigger()
            icon.source: actions.file.closeAll.icon.source
        }
        MenuItem {
            text: actions.file.closeAllButCurrent.text
            onTriggered: actions.file.closeAllButCurrent.trigger()
            icon.source: actions.file.closeAllButCurrent.icon.source
        }

        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.edit.prefs.text
            onTriggered: actions.edit.prefs.trigger()
            icon.source: actions.edit.prefs.icon.source
        }
        Labs.MenuItem {
            text: actions.file.quit.text
            onTriggered: actions.file.quit.trigger()
            icon.source: actions.file.quit.icon.source
            shortcut: actions.file.quit.shortcut
        }
    }
    Labs.Menu {
        title: qsTr("&Edit")
        Labs.MenuItem {
            text: actions.edit.undo.text
            onTriggered: actions.edit.undo.trigger()
            icon.source: actions.edit.undo.icon.source
            shortcut: actions.edit.undo.shortcut
        }
        Labs.MenuItem {
            text: actions.edit.redo.text
            onTriggered: actions.edit.redo.trigger()
            icon.source: actions.edit.redo.icon.source
            shortcut: actions.edit.redo.shortcut
        }
        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.edit.cut.text
            onTriggered: actions.edit.cut.trigger()
            icon.source: actions.edit.cut.icon.source
            shortcut: actions.edit.cut.shortcut
        }
        Labs.MenuItem {
            text: actions.edit.copy.text
            onTriggered: actions.edit.copy.trigger()
            icon.source: actions.edit.copy.icon.source
            shortcut: actions.edit.copy.shortcut
        }
        Labs.MenuItem {
            text: actions.edit.paste.text
            onTriggered: actions.edit.paste.trigger()
            icon.source: actions.edit.paste.icon.source
            shortcut: actions.edit.paste.shortcut
        }
        // Formatting magic!
    }
    Labs.Menu {
        id: build
        title: qsTr("&Build")
        Labs.MenuItem {
            text: actions.build.formatObject.text
            onTriggered: actions.build.formatObject.trigger()
            enabled: actions.build.formatObject.enabled
        }
        Labs.MenuItem {
            text: actions.build.loadObject.text
            onTriggered: actions.build.loadObject.trigger()
            enabled: actions.build.loadObject.enabled
            visible: enabled
            icon.source: actions.build.loadObject.icon.source
            shortcut: actions.build.loadObject.shortcut
        }
        Labs.MenuItem {
            text: actions.build.assemble.text
            onTriggered: actions.build.assemble.trigger()
            enabled: actions.build.assemble.enabled
            visible: enabled
            icon.source: actions.build.assemble.icon.source
            shortcut: actions.build.assemble.shortcut
        }
        Labs.MenuItem {
            text: actions.build.assembleThenFormat.text
            onTriggered: actions.build.assembleThenFormat.trigger()
            enabled: actions.build.assembleThenFormat.enabled
            visible: enabled
            icon.source: actions.build.assembleThenFormat.icon.source
        }
        Labs.MenuItem {
            text: actions.build.execute.text
            onTriggered: actions.build.execute.trigger()
            enabled: actions.build.execute.enabled
            icon.source: actions.build.execute.icon.source
            shortcut: actions.build.execute.shortcut
        }
        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.help.about.text
            onTriggered: actions.help.about.trigger()
        }
    }
    Labs.Menu {
        title: qsTr("&Debug")
        Labs.MenuItem {
            text: actions.debug.start.text
            onTriggered: actions.debug.start.trigger()
            enabled: actions.debug.start.enabled
            icon.source: actions.debug.start.icon.source
            shortcut: actions.debug.start.shortcut
        }
        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.debug.continue_.text
            onTriggered: actions.debug.continue_.trigger()
            enabled: actions.debug.continue_.enabled
            icon.source: actions.debug.continue_.icon.source
        }
        Labs.MenuItem {
            text: actions.debug.pause.text
            onTriggered: actions.debug.pause.trigger()
            enabled: actions.debug.pause.enabled
            icon.source: actions.debug.pause.icon.source
            shortcut: actions.debug.pause.shortcut
        }
        Labs.MenuItem {
            text: actions.debug.stop.text
            onTriggered: actions.debug.stop.trigger()
            enabled: actions.debug.stop.enabled
            icon.source: actions.debug.stop.icon.source
        }
        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: actions.debug.stepInto.text
            onTriggered: actions.debug.stepInto.trigger()
            enabled: actions.debug.stepInto.enabled
            icon.source: actions.debug.stepInto.icon.source
        }
        Labs.MenuItem {
            text: actions.debug.stepOver.text
            onTriggered: actions.debug.stepOver.trigger()
            enabled: actions.debug.stepOver.enabled
            icon.source: actions.debug.stepOver.icon.source
        }
        Labs.MenuItem {
            text: actions.debug.stepOut.text
            onTriggered: actions.debug.stepOut.trigger()
            enabled: actions.debug.stepOut.enabled
            icon.source: actions.debug.stepOut.icon.source
        }
        Labs.MenuSeparator {}
        Labs.MenuItem {
            text: qsTr("&Remove All Breakpoints")
            onTriggered: actions.debug.removeAllBreakpoints.trigger()
        }
    }
    Labs.Menu {
        title: qsTr("&Simulator")
        Labs.MenuItem {
            text: actions.sim.clearCPU.text
            onTriggered: actions.sim.clearCPU.trigger()
        }
        Labs.MenuItem {
            text: actions.sim.clearMemory.text
            onTriggered: actions.sim.clearMemory.trigger()
        }
    }
    Labs.Menu {
        title: qsTr("&View")
        Labs.MenuItem {
            text: actions.view.fullscreen.text
            onTriggered: actions.view.fullscreen.trigger()
        }
        // Dynamic magic to mode switch!
    }
}
