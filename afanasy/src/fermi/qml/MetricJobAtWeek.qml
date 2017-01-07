import QtQuick 2.5
import QtQuick.Layouts 1.1

Item {

    Text {
        id: text_id
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white"
        opacity: 0.55
        text: "New Jobs At Week (DEMO VIEW)"
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize: 15}
    }
    Item{
        id: metric_jobs
        width: parent.width/2.5
        height:parent.height/1.3
        anchors.horizontalCenter:parent.horizontalCenter
        anchors.top: text_id.bottom
        Text {
            anchors.left: parent.left
            color: "white"
            opacity: 0.55
            text: "6"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -10
            color: "white"
            opacity: 0.55
            text: "3"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 17
            color: "white"
            opacity: 0.55
            text: "0"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            color: "white"
            opacity: 0.55
            text: "07.01"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 20
            color: "white"
            opacity: 0.55
            text: "06.27"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Text {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: "white"
            opacity: 0.55
            text: "07.04"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 11}
        }
        Canvas {
            id: canvas_jobs
            anchors.fill: parent
            antialiasing: true
            smooth: true
            opacity: 0.9
            property real width_ : parent.width
            property real height_ : parent.height-17
            property color fillStyle: "#73f4d4" // red

            onPaint: {
                var ctx = canvas_jobs.getContext("2d");

                ctx.strokeStyle = "#73f4d4";
                ctx.lineWidth = 2;

                var gradient = ctx.createLinearGradient(0,0,0,canvas_jobs.height)
                gradient.addColorStop(0, "rgba(115, 244,212, 0.5)")
                gradient.addColorStop(0.9, "transparent")

                ctx.lineTo(canvas_jobs.width_, canvas_jobs.height_);
                ctx.bezierCurveTo(canvas_jobs.width,canvas_jobs.height_/2,canvas_jobs.width*0.97,canvas_jobs.height_/2,canvas_jobs.width*0.95,canvas_jobs.height_*0.9)
                ctx.bezierCurveTo(canvas_jobs.width*0.95,canvas_jobs.height_*0.9,canvas_jobs.width*0.95,canvas_jobs.height_*0.9,canvas_jobs.width*0.9,canvas_jobs.height_*0.9)
                ctx.bezierCurveTo(canvas_jobs.width*0.9,canvas_jobs.height_*0.9,canvas_jobs.width*0.9,canvas_jobs.height_*0.75,canvas_jobs.width*0.7,canvas_jobs.height_*0.5)
                ctx.bezierCurveTo(canvas_jobs.width*0.7,canvas_jobs.height_*0.5,canvas_jobs.width*0.6,canvas_jobs.height_*0.25,canvas_jobs.width*0.5,canvas_jobs.height_*0.1)
                ctx.bezierCurveTo(canvas_jobs.width*0.5,canvas_jobs.height_*0.1,canvas_jobs.width*0.2,canvas_jobs.height_*0.1,canvas_jobs.width*0.032,canvas_jobs.height_*0.3)
                ctx.lineTo(17, canvas_jobs.height_);

                ctx.closePath();
                ctx.fillStyle = gradient
                ctx.fill();
                ctx.stroke();

            }
        }
        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 15
            width: 1
            height: parent.height
            color:"white"
            opacity: 0.8
            layer.enabled: true
        }
        Rectangle {
            anchors.bottom:parent.bottom
            anchors.bottomMargin: 15
            width: parent.width
            height: 1
            color:"white"
            opacity: 0.8
            layer.enabled: true
        }

    }
    Rectangle {
        id: jobs_info_line
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter
        width: 80
        height: 1
        color:"#73f4d4"
        opacity: 0.9
        layer.enabled: true
    }
    Text {
        anchors.left: jobs_info_line.right
        anchors.leftMargin: 5
        anchors.topMargin: -8
        anchors.top:jobs_info_line.top
        color: "white"
        opacity: 0.55
        text: "Jobs"
        font.letterSpacing:1.2
        font { family: robotoRegular.name; pixelSize:12 }
    }
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color:"white"
        opacity: 0.12
        layer.enabled: true
    }
}
