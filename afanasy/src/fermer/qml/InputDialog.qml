import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0  
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

Window {
    id: popInputDialog
    title: "Task Info"
    width: 250
    height: 100
    flags: Qt.Dialog
    modality: Qt.NonModal
    color: "#394c54"

    Rectangle {
        id: input_bar
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width*0.85
        height: 20
        color: "white"
        opacity: 0.3
    }


    Rectangle {
        id:closeButton
        height: 25
        width:59
        color: "#374a52"

        anchors.bottom: parent.bottom
        anchors.right: parent.horizontalCenter
        anchors.bottomMargin: 10
        opacity:0.85
        layer.enabled: true
        layer.effect: DropShadow {
        transparentBorder: true
        samples: 12
        radius:4
        }
        Text {
            text: "Close"
            anchors.centerIn: parent
            color: "white"
        }

        MouseArea {
            id: mouseAreaClose
            anchors.fill: parent
            onClicked: {
                popInputDialog.close();
            }
        }
    }

}
