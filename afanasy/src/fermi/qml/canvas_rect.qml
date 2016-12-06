import QtQuick 2.5
Rectangle {
    id: flashingblob
    width: 800; height: 600
    color: "black"
    opacity: 1.0

    property var m_values: [25,5,50,10,100,90,80,70,60,0,0,0,0,0,15,18]


    Canvas {
        id: canvas_memory
        anchors.fill: parent
        antialiasing: true
        smooth: true
        property real width_ : parent.width/2
        property real height_ : parent.height/2
        property color fillStyle: "#fe9400" // red
        
        onPaint: {
            // Get the drawing context
            var ctx = canvas_memory.getContext("2d");

            ctx.strokeStyle = "#fe9400";
            ctx.lineWidth = 2;
            ctx.strokeRect(canvas_memory.width_/2, canvas_memory.height_/2, canvas_memory.width_, canvas_memory.height_);


            ctx.fillStyle = canvas_memory.fillStyle;
            ctx.moveTo(canvas_memory.width_, canvas_memory.height_);
            ctx.beginPath()


            console.log(" parse"+widthPos(0,width_))
            ctx.lineTo(canvas_memory.width_*1.5, canvas_memory.height_*1.5);

            
            ctx.lineTo(widthPos(0,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(0,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(1,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(1,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(2,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(2,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(3,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(3,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(4,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(4,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(5,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(5,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(6,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(6,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(7,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(7,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(8,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(8,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(9,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(9,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(10,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(10,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(11,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(11,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(12,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(12,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(13,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(13,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(14,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(14,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(15,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(15,canvas_memory.height_,canvas_memory.height_*1.5));
            ctx.lineTo(widthPos(16,canvas_memory.width_,canvas_memory.width_*1.5), heightPos(16,canvas_memory.height_,canvas_memory.height_*1.5));
            
            ctx.lineTo(canvas_memory.width_/2, canvas_memory.height_*1.5);

            ctx.closePath();
            ctx.fill();
        }

        function widthPos(element,width_,start_pos) {
            var pos=start_pos-(width_*(1/15*element))
            return pos;
        }
        function heightPos(element,height_,start_pos) {
            var val_=m_values[element];
            var pos=start_pos-(height_*(val_/100))
            return pos;
        }
    }
}