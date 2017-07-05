import QtQuick 2.7

Rectangle {
    anchors.fill: parent
    color: "#32434a"
    Text{
        id: loading
        anchors.centerIn: parent
        text:"Loading ..."
        color: "white"
        opacity: 0.45
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize: 18}
    }
}
