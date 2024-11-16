import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import edu.pepp 1.0

Rectangle {
    id: root
    color: palette.base

    TextMetrics {
        id: tm
        font.family: "Courier Prime"
        text: "W" // Dummy value to get width of widest character

        //  Calculate widths and height based on current font
        //  All column and line sizing are determined in this
        //  block.
        property double addressWidth: tm.width * 8
        property double valueWidth: tm.width * 8
        property double lineHeight: tm.height + 4 // Allow space around text
        property double boldBorderWidth: 4
    }
    // Create C++ items using the magic of QQmlPropertyList and DefaultProperty
    ActivationModel {
        id: activationModel
        ActivationRecord {
            active: false
            RecordLine {
                address: 0
                value: "10"
                status: ChangeType.Allocated
                name: "a"
            }
            RecordLine {
                address: 2
                value: "20"
                status: ChangeType.Modified
                name: "b"
            }
        }
        ActivationRecord {
            active: true
            RecordLine {
                address: 0x4a
                value: "30"
                name: "c1"
            }
            RecordLine {
                address: 0x4b
                value: "32"
                name: "c2"
            }
        }
        ActivationRecord {
            active: true
            RecordLine {
                address: 6
                value: "40"
                status: ChangeType.Modified
                name: "d"
            }
            RecordLine {
                address: 7
                value: "50"
                name: "e"
            }
            RecordLine {
                address: 9
                value: "60"
                status: ChangeType.Modified
                name: "f"
            }
        }
    }

    ScrollView {
        id: sv

        anchors.fill: parent
        topPadding: 8
        bottomPadding: 0
        contentWidth: column.width // The important part
        contentHeight: Math.max(
                           column.implicitHeight,
                           root.height - sv.topPadding - sv.bottomPadding) // Same
        clip: true // Prevent drawing column outside the scrollview borders
        spacing: 0

        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: column
            anchors.fill: parent
            width: Math.max(implicitWidth, root.width)
            Label {
                Layout.leftMargin: tm.addressWidth + (tm.valueWidth - implicitWidth) / 2
                Layout.alignment: Qt.AlignHCenter & Qt.AlignVCenter
                Layout.bottomMargin: -10
                text: "Globals"
                visible: globals.implicitHeight > 0
                Layout.preferredHeight: visible ? implicitHeight : 0
                font.family: tm.font.family
                font.pointSize: tm.font.pointSize * 1.5
                font.bold: true
            }

            MemoryStack {
                id: globals
                Layout.fillHeight: false
                // Because of negative spacing inside, top rect clips tab bar. Add margin to avoid clipping.
                Layout.topMargin: 4
                Layout.preferredHeight: implicitHeight
                Layout.preferredWidth: globals.childrenRect.width
                Layout.bottomMargin: 15

                //  Font and dimensions - Globals
                font: tm.font
                implicitAddressWidth: tm.addressWidth
                implicitValueWidth: tm.valueWidth
                implicitLineHeight: tm.lineHeight
                boldBorderWidth: tm.boldBorderWidth

                visible: activationModel
                itemModel: activationModel
            }
            Label {
                Layout.leftMargin: tm.addressWidth + (tm.valueWidth - implicitWidth) / 2
                Layout.topMargin: -5
                Layout.bottomMargin: -5
                Layout.alignment: Qt.AlignHCenter & Qt.AlignVCenter
                text: "Heap"
                visible: heap.implicitHeight > 0
                Layout.preferredHeight: visible ? implicitHeight : 0
                font.family: tm.font.family
                font.pointSize: tm.font.pointSize * 1.5
                font.bold: true
            }
            MemoryStack {
                id: heap
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight

                //  Font and dimensions - Heap
                font: tm.font
                implicitAddressWidth: tm.addressWidth
                implicitValueWidth: tm.valueWidth
                implicitLineHeight: tm.lineHeight
                boldBorderWidth: tm.boldBorderWidth

                itemModel: activationModel
            }
            Item {
                Layout.fillHeight: true

                Layout.preferredHeight: 41
                Layout.minimumHeight: 41
            }

            MemoryStack {
                id: stack
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight

                //  Font and dimensions - Stack
                font: tm.font
                implicitAddressWidth: tm.addressWidth
                implicitValueWidth: tm.valueWidth
                implicitLineHeight: tm.lineHeight
                boldBorderWidth: tm.boldBorderWidth

                itemModel: activationModel
            }
            StackGroundGraphic {
                id: graphic
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignHCenter & Qt.AlignBottom
                Layout.leftMargin: tm.addressWidth - tm.boldBorderWidth / 2
                // Honestly not sure why I need -1 here, but if I don't include the offset it looks wrong.
                Layout.preferredWidth: tm.valueWidth + tm.boldBorderWidth - 1
                //  Force graphic to be same width as value column
                graphicWidth: tm.valueWidth + tm.boldBorderWidth - 1
                Layout.preferredHeight: 20
            }
            Label {
                Layout.topMargin: -10
                Layout.alignment: Qt.AlignHCenter & Qt.AlignTop
                Layout.leftMargin: tm.addressWidth + (tm.valueWidth - implicitWidth) / 2
                text: "Stack"
                font.family: tm.font.family
                font.pointSize: tm.font.pointSize * 1.5
                font.bold: true
            }
        } //  ColumnLayout
    } //  ScrollView
}
