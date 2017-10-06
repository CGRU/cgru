import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0  
import QtQuick.Controls.Styles 1.4

Window {
    id: popInfoDialog
    title: "About Fermer"
    width: 220
    height: 250
    flags: Qt.Dialog
    modality: Qt.NonModal
    color: "#394c54"

    Text{
        anchors.top:parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        color:"white"
        opacity: 0.8
        text: "QT:           "+BladesModel.qt_version()+
              "\nFermer:    "+General.version()
    }
    Image {
        anchors.centerIn: parent
        source:"icons/logo.png"
        sourceSize.width: 100
        sourceSize.height: 126
    }/*
    Text{
        anchors.bottom:parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        color:"white"
        opacity: 0.8
        text: "Developed In 'Platige Image'\n        www.platige.com"
    }*/
}
