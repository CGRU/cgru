import QtQuick 2.7
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import JobStateEnums 1.0
 

Rectangle {
    id: delegateItem
    property string v_user_machine:user_machine
    property string v_user_name:user_name
    property string v_user_id:user_id
    property string v_priority:priority
    property string v_tasks_size:tasks_size

      MouseArea {
          id: mouseArea
          anchors.fill: parent
          acceptedButtons: Qt.LeftButton | Qt.RightButton
          hoverEnabled: true
          onClicked: {
                delegateItem.ListView.view.currentIndex = index
                UsersModel.setSelected(index)
          }
          onReleased: {
                if (mouse.button === Qt.RightButton) {
                    delegateItem.ListView.view.currentIndex = index
                    user_context_menu.popup()
                }
          }
      }


      property var aligntype: Text.AlignLeft

      width: parent.width
      anchors.right:parent.right
      color: index===delegateItem.ListView.view.currentIndex ? "#344249" :  mouseArea.containsMouse ? "#344249" : "#2b373d"

      RowLayout {
          id: layout
          anchors.fill: parent
          anchors.topMargin: -(parent.height-42)

          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: users_view.user_minimum_Width.user_machine_ip

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: user_machine
          }
          Item {
                id: item_user_name
                width:45
                height:45
                anchors.verticalCenter: parent.verticalCenter
                Layout.preferredWidth: users_view.user_minimum_Width.user
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
              Layout.preferredWidth: users_view.user_minimum_Width.tasks_running
              height: parent.height
              property int max_width: width/6
              Text {
                    layer.enabled: true
                    color: "white"
                    opacity: 0.8

                    anchors.top:parent.top
                    anchors.topMargin: 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.family:robotoRegular.name
                    font.pointSize: 10
                    text: tasks_size
              }
              Rectangle {
                  anchors.bottom: parent.bottom
                  anchors.bottomMargin: 2
                  width: parent.width
                  height: 24
                  border.color: Qt.rgba(0.99,0.57,0,0.2)//"#fe9400"
                  border.width : 1
                  color: "transparent"
                  visible: tasks_size ? true : false
              }
              Row {
                  anchors.bottom: parent.bottom
                  anchors.bottomMargin: 4
                  spacing: 3
                  Repeater {
                      model:tasks_size+1>(parent.parent.max_width) ? parent.parent.max_width : tasks_size
                      Rectangle {
                          width: 3; height: 20
                          color: "#fe9400"
                          opacity: 0.8
                      }
                  }
              }
              Text {
                    layer.enabled: true
                    color: "white"
                    opacity: 0.8

                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 4
                    font.family:robotoRegular.name
                    font.pointSize: 14
                    text: ">"
                    visible: tasks_size+1>(parent.parent.max_width) ? true : false
              }
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: users_view.user_minimum_Width.jobs_size

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: jobs_size
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: users_view.user_minimum_Width.priority

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: priority
          }
      }

}
