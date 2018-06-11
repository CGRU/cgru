import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import JobStateEnums 1.0
import BladeStateEnums 1.0


Rectangle {
    id: node_view
    color: "transparent"

    signal linesUpdate
    onLinesUpdate: {
        lines.getContext("2d").reset()
        lines.requestPaint()
    }

    property real zoomCoeff: 1
    property real zoomStep: 0.3
    property int offsetX:20
    property int offsetY: 20
    property int offsetX1:120
    property int offsetY1: 120

    focus: visible ? true : false

    function debug(x) {
        return x;
    }

    Shortcut {
        sequence: "F"
        onActivated: {
            nodeContainer.x=0
            nodeContainer.y=0
            lines.getContext("2d").reset()
            lines.requestPaint()
        }
    }
    Shortcut {
        sequence: "R"
        onActivated: {
            JobsModel.arangeNodes()
            BladesModel.arangeNodes()
            lines.getContext("2d").reset()
            lines.requestPaint()
        }
    }

    Item{
        id:nodeContainer
        width: parent.width * zoomCoeff
        height: parent.height * zoomCoeff

        Repeater {
            id: group_node_repeater
            model:JobsModel.jobsModel

            Rectangle {
                property string item_color: Qt.rgba(group_rand_red/2,group_rand_green,group_rand_blue,1)
                color: item_color

                width: group_node_width*zoomCoeff
                height: group_node_height*zoomCoeff
                opacity: 0.3
                x:group_node_posex*zoomCoeff
                y:group_node_posey*zoomCoeff
                visible:block_number==1 ? true : false
                Text {
                    text: job_name;
                    font.pixelSize: 12*zoomCoeff;
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top:parent.top
                    anchors.topMargin: 2
                    color: "white";
                    opacity: 1

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
            }
        }

        Repeater {
            id: job_node_repeater
            model:JobsModel.jobsModel

            Rectangle {
                id: job_node
                //layer.enabled: true
                /*
                layer.effect: DropShadow {
                    transparentBorder: true
                    samples: 3
                    radius:1* zoomCoeff
                    //horizontalOffset: 1* zoomCoeff
                    //verticalOffset: 1* zoomCoeff
                }*/

                width: 150* zoomCoeff
                height: 40* zoomCoeff
                border.width: 2*zoomCoeff
                border.color: selected ? '#546065' :"#424b4f"
                radius: 40*zoomCoeff
                color: selected ? '#546065' :"#424b4f"
                x:node_posex*zoomCoeff
                y:node_posey*zoomCoeff

                Text {
                    text: block_name;
                    font.pixelSize: 11*zoomCoeff;
                    color: "white";
                    anchors.centerIn: parent
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
                Text {
                    text: job_name;
                    font.pixelSize: 8*zoomCoeff;
                    color: "white";
                    anchors.left: parent.left
                    anchors.top:parent.top
                    anchors.leftMargin: 10
                    anchors.topMargin: 3
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Light
                    visible: group_size==1 ? true : false
                }
                Item{
                    anchors.left: parent.left
                    anchors.leftMargin: parent.radius/3
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin:parent.height/10
                    width: parent.width
                    height: 2*zoomCoeff

                    Rectangle {
                        id: progress_fill
                        width: parent.width*0.8
                        height: 2*zoomCoeff
                        color:"white"
                        opacity: 0.27
                        layer.enabled: false
                    }
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        height:2*zoomCoeff
                        width: JobState.RUNNING==job_state ? progress_fill.width*(progress/100) : progress_fill.width
                        color:JobState.DONE==job_state ? "#009688"
                                      : JobState.RUNNING==job_state ? "#81ccc4"
                                      : JobState.OFFLINE==job_state ? "#fe9400"
                                      : JobState.READY==job_state ? "#fe9400"
                                      : depends.lenght!= 0 ? "#5e90ff"
                                      : JobState.ERROR==job_state ? "#f14c22" : "white"
                    }
                }
                Rectangle {
                   id: circle_input
                   property real p_height: parent.height/10
                   anchors.horizontalCenter: parent.horizontalCenter
                   //anchors.top:parent.top
                   width: p_height<parent.height?p_height:parent.height
                   height: p_height
                   color: depends ? "#00b2a1" : "white"
                   border.color: depends ? "#00b2a1" : "white"
                   border.width: 1
                   radius: width*0.5
                   opacity: 0.5
                   visible: group_size==1 ? false : true
                }
                Rectangle {
                   id: circle_out
                   property real p_height: parent.height/10
                   anchors.horizontalCenter: parent.horizontalCenter
                   anchors.bottom: parent.bottom
                   width: p_height<parent.height?p_height:parent.height
                   height: p_height
                   color: "white"
                   border.color: "white"
                   border.width: 1
                   radius: width*0.5
                   opacity: 0.5
                   visible: group_size==1 ? false : true
                }
                Rectangle {
                   id: circle_out_blades
                   property real p_height: parent.height/8
                   anchors.verticalCenter: parent.verticalCenter
                   anchors.right: parent.right
                   width: p_height<parent.height?p_height:parent.height
                   height: p_height
                   color: job_state==JobState.RUNNING ? "#00b2a1" : "white"
                   border.color: job_state==JobState.RUNNING ? "#00b2a1" : "white"
                   border.width: 1
                   radius: width*0.5
                   opacity: 0.5
                }

                MouseArea {
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onReleased: {
                        //JobsModel.setNodePos(parent.x/zoomCoeff,parent.y/zoomCoeff,index);
                        if (mouse.button === Qt.RightButton) {
                            JobsModel.contextSelected(index)
                            job_context_menu.multiselected=JobsModel.multiselected()
                            job_context_menu.popup()
                        }
                    }
                    onPressed: {
                        if ((mouse.button == Qt.LeftButton) &! (mouse.modifiers & Qt.ControlModifier))
                        {
                            JobsModel.setSelected(index)
                            jobs_ListView.currentIndex=index
                            side_view.state="JobView"
                            if (root.side_state=="Tasks"){
                                  TasksModel.updateTasksByJobID(job_id)
                            }
                        }
                    }
                    onClicked: {
                        if (mouse.button == Qt.LeftButton){
                            lines.getContext("2d").reset()
                            lines.requestPaint()
                        }
                        if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ControlModifier))
                        {
                            JobsModel.addToSelected(index)
                        }
                    }
                    onPositionChanged: {
                        JobsModel.passUpdate()
                        JobsModel.setNodePos(parent.x/zoomCoeff,parent.y/zoomCoeff,index);
                        lines.getContext("2d").reset()
                        lines.requestPaint()
                        //JobsModel.dragSelection(mouse.x*zoomCoeff,mouse.y*zoomCoeff)
                    }

                }
            }
        }
        Repeater {
            id: blade_node_repeater
            model:BladesModel.bladesModel

            //PropertyAnimation {id: animateCPU; target: cpu_led; properties: "color"; to: loaded_cpu; duration: 100}


            Rectangle {
                width: 150* zoomCoeff
                height: 35* zoomCoeff
                color: selected ? '#546065' :"#424b4f"
                x:node_posex*zoomCoeff
                y:node_posey*zoomCoeff

                SequentialAnimation {
                    loops : -1
                    running: loaded_cpu>1 ? true : false
                    NumberAnimation { target: cpu_led; property: "opacity"; to: 1; duration: 100 }
                    NumberAnimation { target: cpu_led; property: "opacity"; to: 0; duration: 100 }
                    NumberAnimation { target: cpu_led; property: "opacity"; to: 0; duration: Math.max(1000-(loaded_cpu*10),0) }
                }
                SequentialAnimation {
                    loops : -1
                    running: loaded_hdd>0 ? true : false
                    NumberAnimation { target: hdd_led; property: "opacity"; to: 1; duration: 100 }
                    NumberAnimation { target: hdd_led; property: "opacity"; to: 0; duration: 100 }
                    NumberAnimation { target: hdd_led; property: "opacity"; to: 0; duration: loaded_hdd>49 ? 20 : 1000-(loaded_hdd*20) }
                }

                Text {
                    text: machine_name;
                    font.pixelSize: 11*zoomCoeff;
                    color: "white";
                    anchors.centerIn: parent
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
                Rectangle {
                   property real p_height: parent.height/8
                   anchors.bottom: parent.bottom
                   width: p_height<parent.height?p_height:parent.height
                   height: p_height
                   color: state_machine==BladeState.BUSY ? "#00b2a1" : "white"
                   border.color: state_machine==BladeState.BUSY ? "#00b2a1" : "white"
                   border.width: 1
                   radius: width*0.5
                   opacity: 0.5
                   //visible: group_size==1 ? false : true
                }
                Rectangle {
                    id: cpu_led_off
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 35*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#353d40"
                    border.color: "#2b3033"
                    border.width: 1
                    radius: width*0.5
                    opacity: 0.9
                }
                Rectangle {
                    id: cpu_led
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 35*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#fa9201"
                    border.color: "#fa9201"
                    border.width: 1
                    radius: width*0.5
                    opacity: 0
                }
                Text {
                    text: "C";
                    font.pixelSize: 5*zoomCoeff;
                    color: "white";
                    anchors.bottom: cpu_led.top
                    anchors.left: cpu_led.left
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
                Rectangle {
                    id: memory_led_off
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 25*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#353d40"
                    border.color: "#2b3033"
                    border.width: 1
                    radius: width*0.5
                    opacity: 0.9
                }
                Rectangle {
                    id: memory_led
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 25*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#fa9201"
                    border.color: "#fa9201"
                    border.width: 1
                    radius: width*0.5
                    opacity: loaded_mem[0]/100
                }
                Text {
                    text: "M";
                    font.pixelSize: 5*zoomCoeff;
                    color: "white";
                    anchors.bottom: memory_led.top
                    anchors.left: memory_led.left
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
                Rectangle {
                    id: hdd_led_off
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 15*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#353d40"
                    border.color: "#353d40"
                    border.width: 1
                    radius: width*0.5
                    opacity: 0.9
                }
                Rectangle {
                    id: hdd_led
                    property real p_height: parent.height/8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 15*zoomCoeff
                    width: p_height<parent.height?p_height:parent.height
                    height: p_height
                    color: "#fa9201"
                    border.color: "#fa9201"
                    border.width: 1
                    radius: width*0.5
                    opacity: 0
                }
                Text {
                    text: "H";
                    font.pixelSize: 5*zoomCoeff;
                    color: "white";
                    anchors.bottom: hdd_led.top
                    anchors.left: hdd_led.left
                    opacity: 0.7

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                }
                Rectangle {
                    anchors.right:parent.right
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    width: 1
                    height: parent.height*0.8
                    color: state_machine==BladeState.BUSY ? "#fa9201"
                                                          : state_machine==BladeState.OFFLINE ? "#82898d"
                                                          : state_machine==BladeState.READY ? "#80cbc4"
                                                          : state_machine==BladeState.NIMBY ? "#5e90ff"
                                                          : state_machine==BladeState.BIG_NIMBY ? "#5e90ff"
                                                          : state_machine==BladeState.DIRTY ? "red"
                                                          : "#ffffff"
                    opacity: 0.9
                    layer.enabled: true
                }
                MouseArea {
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis

                    onPressed: {
                        if ((mouse.button == Qt.LeftButton) &! (mouse.modifiers & Qt.ControlModifier))
                        {
                            side_view.state="BladeView"
                            blades_ListView.currentIndex=index
                            BladesModel.setSelected(index)
                        }
                    }
                    onClicked: {
                        if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ControlModifier))
                        {
                            BladesModel.addToSelected(index)
                        }
                    }
                    onPositionChanged: {
                        BladesModel.setNodePos(parent.x/zoomCoeff,parent.y/zoomCoeff,index);
                        lines.getContext("2d").reset()
                        lines.requestPaint()
                    }

                }
            }
        }
    }

    Rectangle {
        id: rectangleSelection
        color: "white"
        border.color: "#00b2a1"
        opacity: 0.25
        visible: false
    }

    Canvas {
        id: lines
        property int x1:0
        property int y1:0
        property int x2:150*zoomCoeff
        property int y2:150*zoomCoeff

        property int canvasMargin: 20
        property int inPath: 0
        property int r: 0
        property int g: 178
        property int b: 161

        width: parent.width
        height: parent.height
        //x:nodeContainer.x
        //y:nodeContainer.y
        onPaint: {
            var ctx = getContext("2d");
            ctx.lineWidth = 2;

            for (var i=0;i<JobsModel.totalJobs();i++){
                //var cHeight = height;
                //var cWidth = width;
                ctx.strokeStyle = Qt.rgba(0,0.69,0.62,0.7)
                for (var d=0;d<JobsModel.getJobDepends(i);d++){
                    var startX = JobsModel.getDependNodePosx(i,d)*zoomCoeff+nodeContainer.x
                    var startY = JobsModel.getDependNodePosy(i,d)*zoomCoeff+nodeContainer.y
                    var endX = 0
                    var endY = 0
                    //console.log(" call "+d)
                    ctx.beginPath()

                    endX = JobsModel.getNodePosX(i)*zoomCoeff+nodeContainer.x
                    endY = JobsModel.getNodePosY(i)*zoomCoeff+nodeContainer.y
                    ctx.moveTo(startX+75* zoomCoeff, startY+38* zoomCoeff)
                    ctx.bezierCurveTo(startX, startY, endX*1.01, endY*1.01, endX+75* zoomCoeff, endY+2* zoomCoeff)
                    ctx.stroke()
                }
            }
            //console.log(" BladesModel "+BladesModel.totalBlades())

            for (var ib=0;ib<BladesModel.totalBlades();ib++){
                var startBladeX =(BladesModel.getNodePosx(ib)+1)*zoomCoeff+nodeContainer.x
                var startBladeY = (BladesModel.getNodePosy(ib)+33)*zoomCoeff+nodeContainer.y
                ctx.moveTo(startBladeX, startBladeY)
                //console.log(" BladesModel.getJobDependencies(ib) "+ib+" "+BladesModel.getJobDependencies(ib))
                for (var b=0;b<BladesModel.getJobDependencies(ib);b++){
                    var endBladeX = (BladesModel.getJobPosx(ib,b)+148)*zoomCoeff+nodeContainer.x
                    var endBladeY = (BladesModel.getJobPosy(ib,b)+20)*zoomCoeff+nodeContainer.y
                    ctx.strokeStyle = Qt.rgba(0,0.69,0.62,BladesModel.getJobNodeIsSelected(ib,b)+0.3)
                    ctx.beginPath()
                    ctx.bezierCurveTo(startBladeX,startBladeY,endBladeX*1.1,endBladeY*1.1,endBladeX,endBladeY)
                    ctx.stroke()
                }
            }

        }
    }


    MouseArea {
        id: mouse_area

        property real xStart
        property real yStart

        property real graphContainer_xStart
        property real graphContainer_yStart

        property bool drawingSelection: false
        property bool selectMode: true
        property bool moveMode: false

        property int mouse_posx: 0
        property int mouse_posy: 0

        z: -1
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton |Qt.RightButton
        onPressed: {
            xStart = mouse.x
            yStart = mouse.y
            graphContainer_xStart = nodeContainer.x
            graphContainer_yStart = nodeContainer.y

            rectangleSelection.x = mouse.x;
            rectangleSelection.y = mouse.y;
            rectangleSelection.width = 1;
            rectangleSelection.height = 1;
            selectMode = mouse_area.pressedButtons & Qt.MiddleButton ? false : true
            moveMode = mouse_area.pressedButtons & Qt.MiddleButton ? true : false
            if( selectMode ) {
                rectangleSelection.visible = true;
                drawingSelection = true;
            }
        }
        onReleased: {
            if(moveMode){
                moveMode=false
                var xOffset = mouse.x - xStart
                var yOffset = mouse.y - yStart
                node_view.offsetX += xOffset
                node_view.offsetY += yOffset
            }
            if( selectMode ) {
                rectangleSelection.visible = false;
            }
        }
        onPositionChanged: {
            mouse_posx=mouse.x
            mouse_posy=mouse.y
            if( rectangleSelection.visible ) {
                if( mouse.x < xStart ) {
                    rectangleSelection.x = mouse.x
                    rectangleSelection.width = xStart - mouse.x;
                }
                else {
                    rectangleSelection.width = mouse.x - xStart;
                }
                if( mouse.y < yStart ) {
                    rectangleSelection.y = mouse.y
                    rectangleSelection.height = yStart - mouse.y;
                }
                else {
                    rectangleSelection.height = mouse.y - yStart;
                }

                JobsModel.drawSelection((xStart-node_view.offsetX)/zoomCoeff,
                                        (yStart-node_view.offsetY)/zoomCoeff,
                                        (mouse.x-node_view.offsetX)/zoomCoeff,
                                        (mouse.y-node_view.offsetY)/zoomCoeff)
                BladesModel.drawSelection((xStart-node_view.offsetX)/zoomCoeff,
                                        (yStart-node_view.offsetY)/zoomCoeff,
                                        (mouse.x-node_view.offsetX)/zoomCoeff,
                                        (mouse.y-node_view.offsetY)/zoomCoeff)
            }

            if( moveMode ) {
                var xOffset = mouse.x - xStart
                var yOffset = mouse.y - yStart
                nodeContainer.x = graphContainer_xStart + xOffset
                nodeContainer.y = graphContainer_yStart + yOffset

                JobsModel.passUpdate()
                lines.getContext("2d").reset()
                lines.requestPaint()
            }
        }
        onClicked: {
            if ((mouse.button === Qt.LeftButton))
            {
                if ( JobsModel.isDrawSelection() == false){
                    JobsModel.clearSelected()
                    BladesModel.clearSelected()
                }
            }
        }

        onWheel:{
            //console.log(" max "+node_view.zoomCoeff.y)
            if(wheel.angleDelta.y > 0){
                node_view.zoomCoeff += node_view.zoomStep
                //nodeContainer.x += mouse_posx
                //nodeContainer.y += mouse_posy
            }else{
                if(node_view.zoomCoeff - node_view.zoomStep >= 0.2)
                    node_view.zoomCoeff -= node_view.zoomStep
                    //nodeContainer.x -= mouse_posx
                    //nodeContainer.y -= mouse_posy
            }
            //console.log(" onWheel ")
            JobsModel.passUpdate()
            lines.getContext("2d").reset()
            lines.requestPaint()

            //blueSquare.x = ((node_view.width * mouseRatioX) - (nodeContainer.width * mouseRatioX)) + node_view.offsetX  -node_view.zoomCoeff
            //blueSquare.y = ((node_view.height * mouseRatioY) - (nodeContainer.height * mouseRatioY )) + node_view.offsetY  -node_view.zoomCoeff
            //console.log(" max "+((node_view.width * mouseRatioX) - (nodeContainer.width * mouseRatioX))+ node_view.offsetX-node_view.zoomCoeff)
            //nodeContainer.x = ((node_view.width * mouseRatioX) - (nodeContainer.width * mouseRatioX)) + node_view.offsetX  -node_view.zoomCoeff
            //nodeContainer.y = ((node_view.height * mouseRatioY) - (nodeContainer.height * mouseRatioY )) + node_view.offsetY  -node_view.zoomCoeff
        }
    }
}
