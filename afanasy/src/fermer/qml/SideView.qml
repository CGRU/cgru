import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: side_view
    color: "#32434a"
    layer.effect: DropShadow {
        transparentBorder: true
        samples: 17
        radius:2
    }
    property alias terminal_output:terminal_side_view
    property int button_bar: 33
    property int top_marg_data: (parent.height-button_bar)-15

    states: [
        State {
          name: "JobView"
              PropertyChanges {target: jobs_side_view; visible: true}
              PropertyChanges {target: blades_side_view; visible: false}
              PropertyChanges {target: users_side_view; visible: false}
              PropertyChanges {target: terminal_side_view; visible: false}
        },
        State {
              name: "BladeView"
              PropertyChanges {target: jobs_side_view; visible: false}
              PropertyChanges {target: blades_side_view; visible: true}
              PropertyChanges {target: users_side_view; visible: false}
              PropertyChanges {target: terminal_side_view; visible: false}
        },
        State {
              name: "NodeView"
              PropertyChanges {target: jobs_side_view; visible: false}
              PropertyChanges {target: blades_side_view; visible: false}
              PropertyChanges {target: users_side_view; visible: false}
              PropertyChanges {target: terminal_side_view; visible: false}
        },
        State {
              name: "UsersView"
              PropertyChanges {target: jobs_side_view; visible: false}
              PropertyChanges {target: blades_side_view; visible: false}
              PropertyChanges {target: users_side_view; visible: true}
              PropertyChanges {target: terminal_side_view; visible: false}
        },
        State {
              name: "MetricView"
              PropertyChanges {target: jobs_side_view; visible: false}
              PropertyChanges {target: blades_side_view; visible: false}
              PropertyChanges {target: users_side_view; visible: false}
              PropertyChanges {target: terminal_side_view; visible: false}
        },
        State {
              name: "TerminalView"
              PropertyChanges {target: jobs_side_view; visible: false}
              PropertyChanges {target: blades_side_view; visible: false}
              PropertyChanges {target: users_side_view; visible: false}
              PropertyChanges {target: terminal_side_view; visible: true}
        }
    ]
    Item{
        id: jobs_side_view
        anchors.fill: parent

        state:"Info"
        states: [
            State {
              name: "Info"
                  PropertyChanges {target: job_info_line; visible: true}
                  PropertyChanges {target: job_tasks_line; visible: false}
                  PropertyChanges {target: job_timegraph_line; visible: false}

                  PropertyChanges {target: job_info_; visible: true}
                  PropertyChanges {target: job_task_; visible: false}
                  PropertyChanges {target: job_timegraph_; visible: false}
            },
            State {
                  name: "Tasks"
                  PropertyChanges {target: job_info_line; visible: false}
                  PropertyChanges {target: job_tasks_line; visible: true}
                  PropertyChanges {target: job_timegraph_line; visible: false}

                  PropertyChanges {target: job_info_; visible: false}
                  PropertyChanges {target: job_task_; visible: true}
                  PropertyChanges {target: job_timegraph_; visible: false}
            },
            State {
                  name: "Timegraph"
                  PropertyChanges {target: job_info_line; visible: false}
                  PropertyChanges {target: job_tasks_line; visible: false}
                  PropertyChanges {target: job_timegraph_line; visible: true}

                  PropertyChanges {target: job_info_; visible: false}
                  PropertyChanges {target: job_task_; visible: false}
                  PropertyChanges {target: job_timegraph_; visible: true}
            }
        ]
        Rectangle {
            width:parent.width
            height:button_bar
            anchors.topMargin: 5
            anchors.top: parent.top

            Layout.fillWidth: true
            layer.enabled: true
            layer.effect: DropShadow {
            transparentBorder: true
            samples: 17
            radius:5
            }

            color:side_view.color
            Item{
                id: job_info
                anchors.top:parent.top
                height:parent.height
                width: parent.width/3
                Text {
                    anchors.centerIn: parent
                    text: "INFO"
                    font.pixelSize: 13
                    font.family: robotoRegular.name
                    font.weight: Font.Thin
                    color: "white"
                    opacity:0.9
                   }
                Rectangle {
                    id: job_info_line
                    anchors.bottom: parent.bottom
                    height: 1
                    width:parent.width
                    color:"#fe9400"
                    opacity: 0.9
                    layer.enabled: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        jobs_side_view.state = "Info"
                        root.side_state="Info"
                    }
                }
            }
            Item{
                id: job_tasks
                anchors.top:parent.top
                anchors.left: job_info.right
                height:parent.height
                width: parent.width/3
                Text {
                    anchors.centerIn: parent
                    text: "TASKS"
                    font.pixelSize: 13
                    font.family: robotoRegular.name
                    font.weight: Font.Thin
                    color: "white"
                    opacity:0.9
                   }
                Rectangle {
                    id: job_tasks_line
                    anchors.bottom: parent.bottom
                    height: 1
                    width:parent.width
                    color:"#fe9400"
                    opacity: 0.9
                    layer.enabled: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        jobs_side_view.state = "Tasks"
                        root.side_state="Tasks"
                        TasksModel.updateTasksByJobID(jobs_ListView.currentItem.v_job_id)
                    }
                }
            }
            Item{
                id:  job_timegraph
                anchors.top:parent.top
                anchors.left: job_tasks.right
                height:parent.height
                width: parent.width/3
                Text {
                    anchors.centerIn: parent
                    text: "TIME GRAPH"
                    font.pixelSize: 13
                    font.family: robotoRegular.name
                    font.weight: Font.Thin
                    color: "white"
                    opacity:0.9
                   }
                Rectangle {
                    id: job_timegraph_line
                    anchors.bottom: parent.bottom
                    height: 1
                    width:parent.width
                    color:"#fe9400"
                    opacity: 0.9
                    layer.enabled: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        jobs_side_view.state = "Timegraph"
                        root.side_state="Timegraph"
                    }
                }
            }
        }
        Loader {
            id: job_info_
            width: parent.width
            height: top_marg_data
            anchors.bottom: parent.bottom
            source: jobs_ListView.currentIndex!=-1 ? "JobInfo.qml" : ""
        }
        Loader {
            id: job_task_
            width: parent.width
            height: top_marg_data
            anchors.bottom: parent.bottom
            source: jobs_side_view.state=="Tasks" ? "JobTasks.qml" : ""
        }
        Loader {
            id: job_timegraph_
            width: parent.width
            height: top_marg_data
            anchors.bottom: parent.bottom
            source: jobs_side_view.state=="Timegraph" ? "JobTimeGraph.qml" : ""
        }
    }
    Item{
        id: blades_side_view
        width: parent.width
        height: top_marg_data
        anchors.top: parent.top
        Rectangle {
            id: button_info
            width:parent.width
            height:33
            anchors.topMargin: 5
            anchors.top: parent.top

            Layout.fillWidth: true
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                samples: 17
                radius:7
            }

            color:side_view.color

            Text {
                   text: "INFO"
                   anchors.centerIn: parent
                   font.pixelSize: 13
                   font.family: robotoRegular.name
                   font.weight: Font.Thin
                   color: "white"
                   opacity:0.9
               }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color:"#fe9400"
                opacity: 0.9
                layer.enabled: true
            }


            MouseArea {
                anchors.fill: parent
                onClicked: parent.clicked()
            }
        }

        Loader {
            id: blades_info
            //anchors.fill: parent
            source: blades_ListView.currentIndex!=-1 ? "BladesInfo.qml" : ""
        }
    }
    Item{
        id: users_side_view
        width: parent.width
        height: top_marg_data
        anchors.top: parent.top
        Rectangle {
            width:parent.width
            height:33
            anchors.topMargin: 5
            anchors.top: parent.top

            Layout.fillWidth: true
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                samples: 17
                radius:7
            }

            color:side_view.color

            Text {
                   text: "INFO"
                   anchors.centerIn: parent
                   font.pixelSize: 13
                   font.family: robotoRegular.name
                   font.weight: Font.Thin
                   color: "white"
                   opacity:0.9
               }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color:"#fe9400"
                opacity: 0.9
                layer.enabled: true
            }


            MouseArea {
                anchors.fill: parent
                onClicked: parent.clicked()
            }
        }

        Loader {
            id: users_info
            anchors.fill: parent
            source: users_ListView.currentIndex!=-1 ? "UsersInfo.qml" : ""
        }
    }
    Item{
        id: terminal_side_view
        anchors.fill: parent
        Rectangle {
            id:terminal_side_view_header
            width:parent.width
            height:33
            anchors.topMargin: 5
            anchors.top: parent.top

            Layout.fillWidth: true
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                samples: 17
                radius:7
            }

            color:side_view.color

            Text {
                   text: "OUTPUT"
                   anchors.centerIn: parent
                   font.pixelSize: 13
                   font.family: robotoRegular.name
                   font.weight: Font.Thin
                   color: "white"
                   opacity:0.9
               }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color:"#fe9400"
                opacity: 0.9
                layer.enabled: true
            }
        }
        TextArea {
            id: terminal_side_text_output
            anchors.top: terminal_side_view_header.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 5
            width: terminal_side_view.width-10
            height: root.height-33-85
            font.letterSpacing: 1.5
            selectByMouse: true
            font.weight: Font.Light
            font {family: robotoRegular.name; pixelSize: 12}
            wrapMode: TextEdit.WrapAnywhere
            text:python_output
            style: TextAreaStyle {
                textColor: Qt.rgba(1,1,1,0.7)
                selectionColor: Qt.rgba(0.88,0.51,0,0.5)
                selectedTextColor: "white"
                backgroundColor: "#32434a"
            }
        }

    }
} 
