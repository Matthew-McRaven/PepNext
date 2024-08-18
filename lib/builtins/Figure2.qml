

/*
 * Copyright (c) 2024 J. Stanley Warford, Matthew McRaven
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
import QtQuick.Controls
import QtQuick.Layouts
import edu.pepp 1.0
import "qrc:/ui/text/editor" as Editor

Item {
    id: wrapper
    required property string title
    required property var payload
    property string curLang: undefined
    property var curElement: undefined
    Component.onCompleted: {
        const el = payload.elements
        const langs = Object.keys(el)
        var defaultElementIndex = 0
        Object.keys(el).map(lang => {
                                languageModel.append({
                                                         "key": lang,
                                                         "value": el[lang].content
                                                     })
                                if (lang === wrapper.payload.copyToElementLanguage)
                                defaultElementIndex = languageModel.count - 1
                            })
        wrapper.curLang = Qt.binding(() => Object.keys(el)[defaultElementIndex])
        wrapper.curElement = Qt.binding(() => payload.elements[wrapper.curLang])
        langSelector.currentIndex = Qt.binding(() => defaultElementIndex)
        langSelector.activated(defaultElementIndex)
    }

    ColumnLayout {
        id: figureLayout
        spacing: 10
        anchors {
            topMargin: 0
            leftMargin: 20
            rightMargin: 20
            bottomMargin: 20
            fill: parent
        }
        ComboBox {
            id: langSelector
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: 35
            Layout.preferredWidth: parent.width
            Layout.fillWidth: true
            textRole: "key"
            valueRole: "value"
            font.pointSize: 22
            model: ListModel {
                id: languageModel
            }
            delegate: ItemDelegate {
                id: delegate
                required property var model
                required property int index
                width: langSelector.width
                contentItem: Text {
                    text: delegate.model[langSelector.textRole]
                    //  Text color in dropdown
                    color: langSelector.highlightedIndex === index ? palette.window : palette.dark
                    font.pointSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                highlighted: langSelector.highlightedIndex === index
            }
            indicator: Canvas {
                id: canvas
                x: langSelector.width - width - langSelector.rightPadding
                y: langSelector.topPadding + (langSelector.availableHeight - height) / 2
                width: 12
                height: 8
                contextType: "2d"

                Connections {
                    target: langSelector
                    function onPressedChanged() {
                        canvas.requestPaint()
                    }
                }

                onPaint: {
                    context.reset()
                    context.moveTo(0, 0)
                    context.lineTo(width, 0)
                    context.lineTo(width / 2, height)
                    context.closePath()
                    context.fillStyle = langSelector.pressed ? Qt.black : "#ff7d33"
                    context.fill()
                }
            }
            contentItem: Text {
                leftPadding: 0
                rightPadding: langSelector.indicator.width + langSelector.spacing
                text: langSelector.displayText
                font: langSelector.font
                color: palette.window
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            onActivated: textArea.text = currentValue
        }
        Editor.ScintillaAsmEdit {
            id: textArea
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
            editorFont: editorFM.font
            language: "Pep/10 ASM"
        }
        Row {
            id: copyRow
            Layout.alignment: Qt.AlignBottom
            Layout.preferredHeight: 20
            Layout.maximumHeight: 30
            Layout.fillWidth: true
            spacing: 10
            //  Copy button logic
            Button {
                id: button
                visible: enabled
                enabled: wrapper.payload.copyToElementLanguage.length > 0
                text: "Copy to New Project"
                anchors.horizontalCenter: copyRow.center
                onClicked: {
                    const lang = wrapper.payload.copyToElementLanguage
                    const text = wrapper.payload.elements[lang].content
                    console.log("Requested to copy:")
                    console.log(text)
                }
            }
            //  Figure title
            Text {
                width: copyRow.width - button.width - copyRow.spacing
                textFormat: Text.RichText
                text: "<div><b>" + wrapper.title + ":</b> Description here"
                wrapMode: Text.WordWrap
            }
        }
    }
    FontMetrics {
        id: editorFM
        font.family: "Courier New"
    }
}
