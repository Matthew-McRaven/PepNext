import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import edu.pepp 1.0

Item {
    id: root
    NuAppSettings {
        id: settings
    }
    FontMetrics {
        id: fm
        font.pointSize: 48
    }

    signal goForward
    ListView {
        anchors.fill: parent
        leftMargin: 20
        topMargin: 20
        bottomMargin: 20
        spacing: 20
        model: ListModel {
            ListElement {
                name: "Pep/10"
                description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla nec odio nec turpis luctus tincidunt. Sed auctor, justo nec ultricies tincidunt, elit purus tincidunt purus, nec ultricies odio odio vel nunc."
                complete: true
                archCode: Architecture.PEP10
            }
            ListElement {
                name: "RISC-V"
                description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla nec odio nec turpis luctus tincidunt. Sed auctor, justo nec ultricies tincidunt, elit purus tincidunt purus, nec ultricies odio odio vel nunc."
                complete: false
                archCode: Architecture.RISCV
            }
            ListElement {
                name: "Pep/9"
                description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla nec odio nec turpis luctus tincidunt. Sed auctor, justo nec ultricies tincidunt, elit purus tincidunt purus, nec ultricies odio odio vel nunc."
                complete: true
                archCode: Architecture.PEP9
            }
            ListElement {
                name: "Pep/8"
                description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla nec odio nec turpis luctus tincidunt. Sed auctor, justo nec ultricies tincidunt, elit purus tincidunt purus, nec ultricies odio odio vel nunc."
                complete: false
                archCode: Architecture.PEP8
            }
        }
        delegate: Item {
            id: delegate
            implicitWidth: Math.min(layout.implicitWidth, parent?.width ?? 100)
            implicitHeight: visible ? layout.implicitHeight : 0
            visible: model.complete || settings.general.showDebugComponents
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    settings.general.defaultArch = model.archCode
                    root.goForward()
                }
            }
            RowLayout {
                id: layout
                anchors.fill: parent
                Button {
                    text: model.name
                    Layout.minimumWidth: fm.averageCharacterWidth * 6
                    font: fm.font
                    display: AbstractButton.TextUnderIcon
                    onReleased: {
                        settings.general.defaultArch = model.archCode
                        root.goForward()
                    }
                }
                Text {
                    text: model.description
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    font {
                        family: fm.font.family
                        pointSize: 24
                    }
                }
            }
        }
    }
}
