import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Item{
    id: job_timegraph_
    property var job_selected_ids: JobsModel.getSelectedIds() //2097404//11534588
    property int tasks_size: TasksModel.tasksSize(job_selected_ids[0])
    property int max:TasksModel.maxTasksTimes(job_selected_ids)
    property real task_h: tasks_size*task_height

    Connections {
        target: jobs_ListView.currentItem
        onJobClicked: {
            console.log(" job_timegraph_ "+job_timegraph_.visible)
            console.log(" tasks_size "+tasks_size)
            if (job_timegraph_.visible==true) {
//                job_selected_ids=JobsModel.getSelectedIds()
//                tasks_size=TasksModel.tasksSize(job_selected_ids[0])
//                task_h=tasks_size*task_height
//                canvas_graph.getContext("2d").reset()
//                canvas_graph.requestPaint()
            }
        }
    }

    ScrollView {
        id: scroll_graph
        anchors.fill: parent
        //width: parent.width
        //height: task_h
        //contentItem :fit_work
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

        Item{
            id: header_layout_label
            width:50
            height:25
            Text {
                id: n_label
                anchors.left: parent.left
                anchors.centerIn: parent
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.5
                text: "indexes"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 11}
            }
        }
        Item{
            id: fit_work
            height: task_h+header_layout_label.height
            width: 0

                ColumnLayout {
                    id: frames_id
                    spacing: 0;
                    width: 50
                    height:task_h
                    anchors.left: parent.left
                    anchors.top:parent.top
                    anchors.topMargin: 25

                    Repeater {
                        model: tasks_size;
                        Text { text: index//TasksModel.tasksFrame(job_selected_ids[0],index)//index
                          font.pointSize: 9
                          opacity: 0.8
                          color: "white"
                          anchors.horizontalCenter: parent.horizontalCenter
                          horizontalAlignment : Text.AlignLeft
                           }
                    }

                }
                Rectangle{
                    id:divider
                    width: 1
                    height: task_h
                    anchors.left: frames_id.right
                    color: "white"
                    opacity: 0.2
                    anchors.top:parent.top
                    anchors.topMargin: 20
                }

                Item{
                    id: canvas_size
                    width: job_timegraph_.width-divider.x-25
                    height:task_h
                    anchors.left: divider.right
                    anchors.leftMargin: 5
                    anchors.top: parent.top
                    anchors.topMargin: task_height/2+25
                    anchors.rightMargin: 5

                    Canvas {
                        id: canvas_graph
                        anchors.fill: parent
                        opacity: 0.95
                        antialiasing: true;
                        renderStrategy: Canvas.Cooperative
                        property var ctx
                        property int size

                        property color fillStyle: "#81ccc4"

                        onPaint: {
                            for (var i_ids=0;i_ids<TasksModel.sizeSelected(job_selected_ids);i_ids++){
                                ctx = canvas_graph.getContext("2d");
                                var job_selected_id=job_selected_ids[i_ids]
                                var max_val=TasksModel.maxTaskTimes(job_selected_id)
                                var tasks_size=TasksModel.tasksSize(job_selected_id)
                                var gradient = ctx.createLinearGradient(canvas_graph.width,0,0,0)
                                var random_color=TasksModel.randomColor(job_selected_id)

                                ctx.strokeStyle = Qt.rgba(random_color[0],random_color[1],random_color[2],1);
                                ctx.lineWidth = 1;

                                //ctx.fillStyle = canvas_graph.fillStyle;
                                ctx.moveTo(canvas_graph.width_, canvas_graph.height_);
                                ctx.beginPath()

                                gradient.addColorStop(0, Qt.rgba(random_color[0],random_color[1],random_color[2],0.6))//1-(max_val/max)
                                gradient.addColorStop(1, "transparent")
                                ctx.lineTo(0,0);
                                //console.log(" max "+max)

                                for (var i = 0; i < tasks_size; i++) {
                                    //console.log(" tasktime "+i)

                                    //ctx.lineTo(canvas_graph.width*TasksModel.taskTimes(11534588,i)/max, canvas_graph.height/(size-i));
                                    ctx.lineTo(canvas_graph.width*(TasksModel.taskTimes(job_selected_id,i)/max),
                                               task_height*i);
                                    //console.log(" canvas_memory "+TasksModel.taskTimes(job_selected_ids,i))
                                }
                                ctx.lineTo(0, (tasks_size-1)*task_height);

                                ctx.closePath();
                                ctx.fillStyle = gradient
                                ctx.fill();
                                ctx.stroke();
                            }
                        }
                        function clamp(v1,max) {
                            if(v1<max){
                                return max
                            }
                            else {
                                return v1
                            }
                        }
                    }
                }
        }

    }
}
