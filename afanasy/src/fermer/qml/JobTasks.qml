import QtQuick 2.7
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0
import TaskStateEnums 1.0

Item{
    id: job_task_
    property var task_minimum_Width: { "frame": 70,
                                      "progress": 80,
                                      "elapsed": 53,
                                      "blade": 100}

    Item{
        id: header_layout
        width:parent.width
        height:35
        anchors.top: parent.top
        RowLayout {
            id: layout
            anchors.fill: parent
            Text {
                Layout.preferredWidth: task_minimum_Width.frame

                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.5
                text: " #"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 13}
            }
            Text {
                Layout.preferredWidth: task_minimum_Width.progress

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.5
                text: "Progress"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 13}
            }
            Text {
                Layout.preferredWidth: task_minimum_Width.elapsed

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.5
                text: "Elapsed"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 13}
            }
            Text {
                Layout.preferredWidth: task_minimum_Width.blade

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.5
                text: "Blade"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 13}
            }
        }
    }

    Component{
        id:task_delegate
        TaskDelegate{
            width: parent.width
            height: task_height
            aligntype: custom_aligntype
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: header_layout.height
        highlightOnFocus:true
        style: ScrollViewStyle {
            transientScrollBars: root.transient_toggl
            scrollBarBackground: Item {
                implicitWidth: 12
                implicitHeight: 26
            }
            handle: Rectangle {
                implicitWidth: 12
                implicitHeight: 26
                color: "white"
                opacity: 0.4
            }
            decrementControl: Rectangle {
                implicitWidth: 15
                implicitHeight: 2
                color: "white"
                opacity: 0.3
            }
            incrementControl: Rectangle {
                implicitWidth: 12
                implicitHeight: 1
                color: "white"
                opacity: 0.3
            }
        }
            ListView {
              id: task_ListView
              anchors.fill: parent
              delegate:task_delegate
              model: TasksModel.tasksModel
              focus: true
              cacheBuffer:40
            }
    }

    InfoDialog {
        id: popTaskOutputDialog
        title: "Task Output"
    }
    InfoDialog {
        id: popTaskInfoDialog
        title: "Task Info"
    }
    InfoDialog {
        id: popTaskLogDialog
        title: "Task Log"
    }
    InfoDialog {
        id: popTaskErrorHostDialog
        title: "Task Error Blades"
    }

    Menu {
       id: task_context_menu
       MenuItem {
           text: "Output"
           onTriggered:{
               popTaskOutputDialog.text="Retrieving running task output from render..."
               popTaskOutputDialog.show()
               popTaskOutputDialog.text=TasksModel.taskOutput(task_ListView.currentItem.v_id,task_ListView.currentItem.v_state)
           }
       }
       MenuItem {
           text: "Log"
           onTriggered:{
               popTaskLogDialog.text=TasksModel.taskLog(task_ListView.currentItem.v_id)
               popTaskLogDialog.show()
           }
       }
       MenuItem {
           text: "Info"
           onTriggered:{
               popTaskInfoDialog.text=TasksModel.taskInfo(task_ListView.currentItem.v_id)
               popTaskInfoDialog.show()
           }
       }
       MenuItem {
           text: "Show Error Blades"
           onTriggered:{
               popTaskErrorHostDialog.text=TasksModel.taskErrorHost(task_ListView.currentItem.v_id)
               popTaskErrorHostDialog.show()
           }
       }
       MenuSeparator { }
       MenuItem {
           text: "Restart"
           onTriggered:{
                TasksModel.taskRestart(task_ListView.currentItem.v_id)
           }
       }
       MenuItem {
           text: "Skip"
           onTriggered:{
                TasksModel.taskSkip(task_ListView.currentItem.v_id)
           }
       }
       style: MenuStyle {
           frame: Rectangle {
               color: "#394c54"
               border.color: "#394c54"
           }
           itemDelegate {
               background: Rectangle {
                   color: "#394c54"
               }
               label: Text {
                   color: "white"
                   opacity: styleData.selected ? 0.9 :0.7
                   text: styleData.text
                   font.family:robotoRegular.name
                   font.pointSize: 10
               }
           }
       }
    }
}
