import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0  
import QtQuick.Controls.Styles 1.4

Window {
    id: popInfoDialog
    title: "Task Info"
    width: 400
    height: 600
    flags: Qt.Dialog
    modality: Qt.NonModal
    color: "#394c54"
    property string text: "None"
    property string returnValue: ""

    TextArea {
        anchors.fill: parent
        text: popInfoDialog.text
        style: TextAreaStyle {
            textColor: "white"
            selectionColor: "steelblue"
            selectedTextColor: "white"
            backgroundColor: "#344249"
        }
    }
    Rectangle {
        height: 20
        width:50
        color: "#344249"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottomMargin: 2
        Text {
            text: "Close"
            anchors.centerIn: parent
            color: "white"
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: {
                popInfoDialog.close();
            }
        }
    }
}
