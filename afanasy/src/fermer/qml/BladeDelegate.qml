import QtQuick 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import BladeStateEnums 1.0


Rectangle {
    id: delegateItem
    property string v_base_os:base_os
    property int    v_state:state_machine
    property string v_name:machine_name
    property string v_blades_group:blades_group
    property real v_loaded_cpu:state_machine==BladeState.OFFLINE ? 0 : loaded_cpu
    property real v_loaded_mem:state_machine==BladeState.OFFLINE ? 0 : loaded_mem[0]
    property real v_loaded_net:state_machine==BladeState.OFFLINE ? 0 : loaded_net
    property string v_working_time:working_time
    property string v_ip_address:ip_address
    property string v_blade_id:blade_id
    property string v_avalible_performance_slots:avalible_performance_slots
    property string v_jobsIn: ""+jobsIn
    property int v_hdd_busy: loaded_hdd
    property int v_max_tasks: max_tasks


    MouseArea {
          id: mouseArea
          anchors.fill: parent
          hoverEnabled: true
          acceptedButtons: Qt.LeftButton | Qt.RightButton
          onClicked: {
              delegateItem.ListView.view.currentIndex = index
          }
          onReleased: {
                  if (mouse.button === Qt.RightButton) {
                      delegateItem.ListView.view.currentIndex = index
                      blade_context_menu.popup()
                  }
          }
      }

      property real loadedcpu : v_loaded_cpu*1.96;
      property real angle : 0;
      PropertyAnimation {id: animateCPU; target: delegateItem; properties: "angle"; to: loadedcpu; duration: 1000}

      onAngleChanged: canvas_cpu.requestPaint ()
      Timer {interval: 2000; repeat: true; running: true; onTriggered: canvas_cpu.requestPaint (),animateCPU.start() }

      property var aligntype: Text.AlignLeft

      width: parent.width
      height: 45

      color:index===delegateItem.ListView.view.currentIndex ? "#344249" :  mouseArea.containsMouse ? "#344249" : "#2b373d"

      Rectangle {
          anchors.bottom: parent.bottom
          width: parent.width
          height: 1
          color:"white"
          opacity: 0.12
          layer.enabled: true
      }

      RowLayout {
          id: layout
          anchors.fill: parent
          anchors.topMargin: -(parent.height-42)

          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                id: base_os_Item
                Layout.preferredWidth: blades_view.blade_minimum_Width.header_base*1

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: base_os
          }
          Item {
              Layout.preferredWidth: blades_view.blade_minimum_Width.state*1
              height: 30
              Rectangle {
                  anchors.centerIn: parent
                  width: 1
                  height: 30
                  color: state_machine==BladeState.BUSY ? "#fa9201"
                                                        : state_machine==BladeState.OFFLINE ? "#82898d"
                                                        : state_machine==BladeState.READY ? "#80cbc4"
                                                        : state_machine==BladeState.NIMBY ? "#bc5eff"
                                                        : state_machine==BladeState.BIG_NIMBY ? "#5e90ff"
                                                        : state_machine==BladeState.DIRTY ? "red"
                                                        : "#ffffff"
                  opacity: 0.9
                  layer.enabled: true
              }
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.name*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: machine_name
          }
          /*
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.blades_group*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: blades_group
          }*/
          Canvas {
              id: canvas_cpu;
              width: 50
              height:50
              opacity: 0.95
              antialiasing: true;
              smooth: true;
              renderStrategy: Canvas.Cooperative

              onPaint: {
                  var ctx = getContext("2d");
                  ctx.reset();

                  var centreX = width / 2;
                  var centreY = height / 2;

                  ctx.beginPath();
                  ctx.moveTo(centreX, centreY);

                  ctx.beginPath();
                  ctx.fillStyle = "#344249"
                  ctx.arc(centreX, centreY, width / 4, 0, Math.PI * 2, false);
                  ctx.lineTo(centreX, centreY);
                  ctx.fill();
                  ctx.closePath();

                  ctx.beginPath();
                  ctx.fillStyle = "#fe9400";
                  ctx.arc(centreX, centreY, width / 4,-1.57,(angle-45) * Math.PI / 100, false);//angle
                  ctx.lineTo(centreX, centreY);
                  ctx.fill();
                  ctx.closePath();

                  ctx.beginPath();
                  ctx.fillStyle = "#2b373d"
                  ctx.moveTo(centreX, centreY);
                  ctx.arc(centreX, centreY, width / 7.5, 0, Math.PI * 2, false);
                  ctx.lineTo(centreX, centreY);
                  ctx.fill();
              }
          }
          Canvas {
              id: canvas_memory
              opacity: 0.95
              width: 80
              height:delegateItem.height/1.6
              antialiasing: true;
              renderStrategy: Canvas.Cooperative
              property var ctx

              property color fillStyle: "#fe9400"

              onPaint: {
                  // Get the drawing context

                  ctx = canvas_memory.getContext("2d");

                  ctx.strokeStyle = "#fe9400";
                  ctx.lineWidth = 1;
                  ctx.strokeRect(0, 0, canvas_memory.width, canvas_memory.height);



                  ctx.fillStyle = canvas_memory.fillStyle;
                  ctx.moveTo(canvas_memory.width_, canvas_memory.height_);
                  ctx.beginPath()


                  ctx.lineTo(canvas_memory.width, canvas_memory.height);
                  if (state_machine!=BladeState.OFFLINE)
                  {
                      for (var i = 0; i < 25; i++) {
                          ctx.lineTo(widthPos(i,canvas_memory.width,canvas_memory.width), heightPos(i,canvas_memory.height,canvas_memory.height));
                      }
                  }
                  ctx.lineTo(0, canvas_memory.height);


                  ctx.closePath();
                  ctx.fill();
                  ctx.stroke();
              }
              function widthPos(element,width_,start_pos) {
                  var pos=start_pos-(width_*(1/24*element))
                  return pos;
              }
              function heightPos(element,height_,start_pos) {
                  var val_=loaded_mem[element];
                  var pos=start_pos-(height_*(val_/100))
                  return pos;
              }
              Timer {interval: 1000; repeat: true; running: true; onTriggered: {canvas_memory.getContext("2d").reset();canvas_memory.requestPaint ()} }
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.network*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: v_loaded_net+" Kb/s"
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.slots*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: performance_slots
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.av_slots*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: avalible_performance_slots
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.eplased*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: working_time
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: blades_view.blade_minimum_Width.address*1
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: ip_address
          }
      }
}
