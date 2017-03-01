import QtQuick 2.7
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import JobStateEnums 1.0
 

Rectangle {
    id: delegateItem
    property string v_progress:progress
    property string v_elapsed:elapsed
    property int    v_state:job_state
    property string v_user_name:user_name
    property string v_job_name:job_name
    property string v_slots:slots
    property string v_started:started
    property string v_software:software
    property string v_job_id:job_id
    property var    v_blades:blades
    property string v_block_name:block_name
    property string v_blades_mask:blades_mask
    property int    v_priority: priority
    //property int    v_blades_length:blades_length
    property int    v_group_size: group_size
    property string v_approx_time:approx_time
    property string v_depends:depends

      MouseArea {
          id: mouseArea
          anchors.fill: parent
          acceptedButtons: Qt.LeftButton | Qt.RightButton
          hoverEnabled: true
          onClicked: {
              if ((mouse.button == Qt.LeftButton) &! (mouse.modifiers & Qt.ControlModifier) &!(mouse.modifiers & Qt.ShiftModifier))
              {
                  delegateItem.ListView.view.currentIndex = index
                  JobsModel.setSelected(index)
                  if (root.side_state=="Tasks"){
                        TasksModel.updateTasksByJobID(v_job_id)

                  }
              }

              if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ControlModifier))
              {
                  JobsModel.addToSelected(index)
              }
              if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ShiftModifier))
              {
                  JobsModel.addToSelected(index)
                  JobsModel.shiftSelected()
              }
              jobClicked()
          }
          onReleased: {
                  if (mouse.button === Qt.RightButton) {
                      delegateItem.ListView.view.currentIndex = index
                      JobsModel.contextSelected(index)
                      job_context_menu.multiselected=JobsModel.multiselected()
                      //job_context_menu.size_selected=35//JobsModel.sizeSelected()
                      job_context_menu.popup()
                  }
          }
      }


      property var aligntype: Text.AlignLeft
      property string item_color: group_size>1 ? Qt.rgba(0.168+(group_rand_red/40),0.215+(group_rand_green/40),0.239+(group_rand_blue/35),1) : Qt.rgba(0.168,0.215,0.239,1)
      property string item_active_color: group_size>1 ? Qt.rgba(0.168+(group_rand_red/40)+0.035,0.215+(group_rand_green/40)+0.043,0.239+(group_rand_blue/35)+0.046,1) : Qt.rgba(0.203,0.258,0.286,1)
      property string ico_path: "icons/soft/"

      width: parent.width
      anchors.right:parent.right
      color: selected ? item_active_color
                     : mouseArea.containsMouse ? item_active_color
                     : item_color
      Rectangle {
          id: bottom_line
          anchors.right:parent.right
          anchors.bottom: parent.bottom
          width: group_size==1 ? parent.width : parent.width*0.98
          height: 1
          color:"white"
          opacity: 0.12
          layer.enabled: true
      }
      Rectangle {
          anchors.right:bottom_line.left
          anchors.bottom: parent.bottom
          width: parent.width
          height: 1
          color:"white"
          opacity: 0.15
          visible: block_number==(group_size-1)
      }
      Rectangle {
          id: vertical_center
          anchors.left: bottom_line.left
          anchors.bottom: parent.bottom
          width: 1
          height: parent.height
          color:"white"
          opacity: 0.18
          layer.enabled: true
          visible: true
      }
      Rectangle {
          anchors.right: bottom_line.left
          anchors.bottom: parent.bottom
          width: 1
          height: parent.height/2
          anchors.rightMargin: 10
          color:"white"
          opacity: 0.18
          layer.enabled: true
          visible: block_number==(group_size-1) ? false : true
      }
      Rectangle {
          anchors.right: bottom_line.left
          anchors.top: parent.top
          width: 1
          height: parent.height/2
          anchors.rightMargin: 10
          color:"white"
          opacity: 0.18
          layer.enabled: true
          visible: block_number==0 ? false : true
      }
      Rectangle {
          anchors.right:vertical_center.right

          anchors.verticalCenter: parent.verticalCenter
          width: 11
          height: 1
          color:"white"
          opacity: 0.18
          layer.enabled: true
          visible: true
      }
      //---------------------------------------------------------

      RowLayout {
          id: layout
          anchors.fill: parent
          anchors.topMargin: -(parent.height-42)

          Item{
              Layout.preferredWidth: jobs_view.job_minimum_Width.progress
              anchors.verticalCenter: parent.verticalCenter
              anchors.left:parent.left
              anchors.leftMargin: group_size>1 ? 30 : 20
              width: 90
              height: 3

              Rectangle {
                  id: progress_fill
                  width: parent.width*0.8
                  height: 2
                  color:"white"
                  opacity: 0.27
                  layer.enabled: false
              }
              Rectangle {
                  anchors.verticalCenter: parent.verticalCenter
                  height:2
                  width: JobState.RUNNING==v_state ? progress_fill.width*(progress/100) : progress_fill.width
                  //width:progress_fill.width
                  color:JobState.DONE==v_state ? "#009688"
                                : JobState.RUNNING==v_state ? "#81ccc4"
                                : JobState.OFFLINE==v_state ? "#375753"
                                : JobState.READY==v_state ? "#fe9400"
                                : JobState.ERROR==v_state ? "#f14c22" : "white"
              }
              Text{
                  //anchors.horizontalCenter: parent.horizontalCenter
                  anchors.top: parent.bottom
                  anchors.topMargin: 5
                  font.family:robotoRegular.name
                  font.letterSpacing: 1.5
                  font.pointSize: 8
                  color:"#f14c22"
                  text:"Avoiding Blades:"+avoiding_blades
                  visible:avoiding_blades
              }
          }

          Text {
                id: elapsed_time_from_start
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: jobs_view.job_minimum_Width.elapsed

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: elapsed
          }

          Item {
                id: item_user_name
                width:45
                height:45
                anchors.verticalCenter: parent.verticalCenter
                Layout.preferredWidth: jobs_view.job_minimum_Width.user
                Rectangle {
                   id: circle_name
                   anchors.verticalCenter: parent.verticalCenter
                   width: 33<parent.height?33:parent.height
                   height: 33
                   color: user_color
                   border.color: user_color
                   border.width: 1
                   radius: width*0.5
                   opacity: 0.4
                }
                Text {
                    layer.enabled: false
                    anchors.centerIn: circle_name
                    color: "white"
                    text: user_name[0]+user_name[1]
                    font.capitalization: Font.AllUppercase
                    font.family:robotoRegular.name
                    font.pointSize: 12
                    opacity: 0.8
                }
          }
          Item{
              Layout.preferredWidth: jobs_view.job_minimum_Width.job_name
              Text {
                    layer.enabled: true
                    color: "white"
                    opacity: 0.8
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter:parent.horizontalCenter
                    anchors.horizontalCenterOffset: aligntype===Text.AlignHCenter ? 0
                                                  : aligntype===Text.AlignRight ? v_block_name.length
                                                  : aligntype===Text.AlignLeft ? -v_block_name.length : 0

                    font.family:robotoRegular.name
                    font.weight: Font.Normal
                    font.pointSize: 10.5
                    font.letterSpacing: 2
                    clip:true
                    width: parent.width
                    text: swap_jobs_name ? job_name : block_name
              }
              Text {
                    layer.enabled: true
                    color: "white"
                    opacity: 0.6
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -15
                    font.family:robotoRegular.name
                    font.letterSpacing: 1.7
                    font.pointSize: 8
                    clip:true
                    width: parent.width
                    text: swap_jobs_name ? block_name : job_name
              }
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: jobs_view.job_minimum_Width.approximate_time

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: approx_time
          }

          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: jobs_view.job_minimum_Width.slots

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: slots
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: jobs_view.job_minimum_Width.priority

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: priority
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: jobs_view.job_minimum_Width.started

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: started
          }
          Item{
              Layout.preferredWidth: jobs_view.job_minimum_Width.software
              Image {
                  id: soft_icon
                  anchors.centerIn: parent
                  sourceSize.width: 35
                  sourceSize.height: 35
                  opacity: 0.8
                  source:ico_path+software+".svg"
              }
              state:software
          }
      }

}
