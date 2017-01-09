import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:metrics_view
    color: "#1d262b"
    width: 1024

    Text {
        anchors.centerIn: parent
        color: "white"
        opacity: 0.45
        text:"Not Implemented Yet"
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize: 18}
    }

    /*
    Rectangle {
        id: blueSquare
        width: 120; height: 120
        //x: box.width - width - 10; y: 10    // making this item draggable, so don't use anchors
        color: "white"

        Text { text: "test"; font.pixelSize: 16; color: "white"; anchors.centerIn: parent }

        MouseArea {
            anchors.fill: parent
            //! [drag]
            drag.target: blueSquare
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.maximumX: metrics_view.width - parent.width
            drag.minimumY: 0
            drag.maximumY: metrics_view.height - parent.width
            //! [drag]
        }
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            samples: 17
            radius:8
        }
    }*/
}
