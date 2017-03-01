import QtQuick 2.7

Rectangle {
    anchors.fill: parent
    color: "#1d262b"
    Text{
        id: label_no_connect
        anchors.centerIn: parent
        text:"No Connection To Server"
        color: "white"
        opacity: 0.45
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize: 18}
    }
    Text{
        anchors.top: label_no_connect.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: General.serverIP()
        color: "white"
        opacity: 0.45
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize: 18}
    }

}
