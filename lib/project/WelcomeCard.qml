import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: wrapper
    required property int architecture
    required property int abstraction
    property bool enabled: true
    property alias text: title.text
    property alias description: description.text
    property real textPadding: 5
    property alias source: image.source

    width: 165
    height: 200
    // We cannot use button, since we want to control the foreground image, which interferes with `hover`.
    // Since `hover` is RO on a button, we can't set it from the MouseArea.
    // Therefore, we need to imitate the behaviors/styling of Button in place.
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: infoOverlay.state = "hovered"
        onExited: infoOverlay.state = "unhovered"
        onClicked: {
            // Magic function that exists in Welcome.qml and is exposed to us.
            if (wrapper.enabled)
                addProject(architecture, abstraction, "", false)
        }
    }
    color: wrapper.enabled ? palette.button : palette.button.darker(1.4)
    WelcomeTitle {
        id: title
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        bottomPadding: wrapper.textPadding
    }
    // Placeholder for image showing current project type.
    Image {
        id: image
        fillMode: Image.PreserveAspectFit
        anchors {
            right: parent.right
            top: title.bottom
            left: parent.left
            bottom: parent.bottom
            // Ensure that the image does not clip the title text
            bottomMargin: title.height + title.topPadding + title.bottomPadding
            margins: wrapper.textPadding
        }
        verticalAlignment: Image.AlignTop
        horizontalAlignment: Image.AlignHCenter
        source: "image://icons/blank.svg"
    }

    // "Popup" with persistent tile and description shown on hover,
    Item {
        id: infoOverlay

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        // Must nest background to prevent making text more opaque
        Rectangle {
            anchors.fill: parent
            // TODO: when blur is implemented, replace this with a blur effect
            opacity: 1
            color: wrapper.color
        }

        // Must clip or description text will overflow into buttons below.
        clip: true
        state: "unhovered"
        states: [
            State {
                name: "unhovered"
                PropertyChanges {
                    target: infoOverlay
                    height: 0
                }
            },
            State {
                name: "hovered"
                PropertyChanges {
                    target: infoOverlay
                    height: parent.height - title.height
                }
            }
        ]
        transitions: Transition {
            NumberAnimation {
                properties: "height"
            }
        }
        Column {
            anchors.fill: parent
            anchors.margins: wrapper.textPadding
            // Hidden by default when not hovered.
            Rectangle {
                id: bar
                border.color: palette.text
                height: 5
                border.width: height
                width: parent.width
            }
            Text {
                id: description
                text: "Describes the usecase of this project type..."
                color: palette.text
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }
}
