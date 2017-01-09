import QtQuick 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.0

Button{
    property string image: "icons/nodes.png"
    id: global_b
    property real st_opacity: 0.38
    style: ButtonStyle {
        background: Rectangle {
            opacity: control.hovered ? 0.78 : st_opacity;
            implicitWidth: 40
            implicitHeight: 40
            color: "transparent"
            Image {
                opacity: 1
                anchors.centerIn:parent
                source:global_b.image
            }
        }
    }
}
