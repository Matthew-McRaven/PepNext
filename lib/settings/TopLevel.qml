import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as Ui
import edu.pepp 1.0

Rectangle {
    implicitHeight: layout.childrenRect.height
    implicitWidth: layout.childrenRect.width
    clip: true
    NuAppSettings {
        id: settings
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        Ui.CategoryList {
            id: categoryList
            Layout.minimumWidth: 30
            Layout.preferredWidth: categoryList.contentWidth
            Layout.fillHeight: true
            Layout.margins: 3
            model: settings.categories
            onCurrentCategoryChanged: {
                const props = {
                    "category": categoryList.currentCategory
                }
                if (categoryList.currentCategory)
                    loader.setSource(categoryList.currentCategory.source, props)
            }
        }
        Loader {
            id: loader
            Layout.fillHeight: true
            Layout.fillWidth: true
            source: "UnimplementedCategoryDelegate.qml"
        }
    }
}
