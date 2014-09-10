import QtQuick 2.0
import QtQuick.Controls 1.2
import Machinekit.Application 1.0
import QtQuick.Window 2.0

ListView {
    implicitWidth: 200
    implicitHeight: 200
    verticalLayoutDirection: ListView.BottomToTop
    model: notificationModel
    delegate: notificationDelegate

    function addNotification (type, text)
    {
        notificationModel.append({"type": type, "text": text})
    }

    SystemPalette {
        id: systemPalette
    }

    Component {
        id: notificationDelegate
        Rectangle {

            //height: width * 0.4
            width: parent.width
            height: textLabel.height
            radius: Screen.logicalPixelDensity * 2
            color: systemPalette.light
            border.color: systemPalette.mid
            border.width: 1

            Label {
                id: textLabel
                anchors.left: typeImage.right
                anchors.right: parent.right
                anchors.margins: Screen.logicalPixelDensity * 5
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                text: model.text
            }

            Image {
                id: typeImage
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width * 0.13
                anchors.margins: Screen.logicalPixelDensity * 5
                fillMode: Image.PreserveAspectFit
                source: {
                    switch (model.type) {
                    case ApplicationError.NmlError:
                    case ApplicationError.OperatorError:
                        return "qrc:Machinekit/Application/Controls/icons/dialog-error"
                    case ApplicationError.NmlDisplay:
                    case ApplicationError.OperatorDisplay:
                        return "qrc:Machinekit/Application/Controls/icons/dialog-warning"
                    case ApplicationError.NmlText:
                    case ApplicationError.OperatorText:
                        return "qrc:Machinekit/Application/Controls/icons/dialog-information"
                    default:
                        return ""
                    }
                }
                visible: source != ""
            }

            MouseArea {
                id: notificationMouseArea
                anchors.fill: parent
                onClicked: notificationModel.remove(model.index)
                hoverEnabled: true
            }

            opacity: (notificationMouseArea.containsMouse || (notificationModel.count === (model.index+1))) ? 1.0 : 0.6
        }
    }

    ListModel {
        id: notificationModel
    }
}