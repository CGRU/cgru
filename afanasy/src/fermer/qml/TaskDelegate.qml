import QtQuick 2.7
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import TaskStateEnums 1.0


Rectangle {
    id: delegateItem

    property real v_progress:progress
    property int v_id:id
    property var aligntype: Text.AlignLeft
    property var v_state: status

      MouseArea {
          id: mouseArea
          anchors.fill: parent
          acceptedButtons: Qt.LeftButton | Qt.RightButton
          hoverEnabled: true
          onClicked: {
              delegateItem.ListView.view.currentIndex = index
          }
          onReleased: {
                  if (mouse.button === Qt.RightButton) {
                      delegateItem.ListView.view.currentIndex = index
                      task_context_menu.popup()
                  }
          }
          InfoDialog {
              id: popTaskOutputDialog
              title: "Task Output"
          }
          onDoubleClicked: {
              popTaskOutputDialog.text="Retrieving running task output from render..."
              popTaskOutputDialog.show()
              popTaskOutputDialog.text=TasksModel.taskOutput(v_id,status)
          }
      }


      color:index===delegateItem.ListView.view.currentIndex ? "#394c54" :  mouseArea.containsMouse ? "#394c54" : "#344249"

      Rectangle {
          anchors.bottom: parent.bottom
          width: parent.width
          height: 1
          color:"white"
          opacity: 0.05
          layer.enabled: true
      }

      RowLayout {
          id: layout
          anchors.fill: parent
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: task_minimum_Width.frame

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: frame
          }
          Item{
              Layout.preferredWidth: task_minimum_Width.progress
              anchors.verticalCenter: parent.verticalCenter
              width: 100
              height: 2

              Rectangle {
                  id: progress_fill
                  width: parent.width
                  height: 2
                  color:"white"
                  opacity: 0.27
                  layer.enabled: false
              }
              Rectangle {
                  height:2
                  width:TaskState.RUNNING==status ? progress_fill.width*(progress/100) : progress_fill.width
                  color:TaskState.DONE==status ? "#009688"
                                : TaskState.RUNNING==status ? "#81ccc4"
                                : TaskState.SKIPPED==status ? "#fe9400"
                                : TaskState.READY==status ? "#fe9400"
                                : TaskState.ERROR==status ? "#f14c22" : "blue"
              }
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: task_minimum_Width.elapsed

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: elapsed
          }
          Text {
                layer.enabled: true
                color: "white"
                opacity: 0.8
                Layout.preferredWidth: task_minimum_Width.blade

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : aligntype
                font.family:robotoRegular.name
                font.pointSize: 10.5
                text: blade
          }
      }
}
