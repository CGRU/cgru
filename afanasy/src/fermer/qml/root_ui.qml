/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import QtQuick.Window 2.1
import JobStateEnums 1.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.1

Item {
    id: root
    Component.onDestruction: {
        settings.custom_aligntype_s = item_align.state
        settings.view_states = item_states.state
    }

    Item {
        id:item_states
        state:settings.view_states
        states: [
            State {
              name: "JobView"
                  PropertyChanges {target: blades_view; visible: false}
                  PropertyChanges {target: main_menu; state:"JobView"}
                  PropertyChanges {target: side_view; state:"JobView"}
                  PropertyChanges {target: users_view; visible: false}
                  PropertyChanges {target: node_view; visible: false}
                  PropertyChanges {target: metrics_view; visible: false}
                  PropertyChanges {target: jobs_view; visible: true}
                  PropertyChanges {target: terminal_view; visible: false}
                  PropertyChanges {target: root; update_state: 0}
            },
            State {
              name: "BladeView"
                  PropertyChanges {target: blades_view; visible: true}
                  PropertyChanges {target: main_menu; state:"BladeView"}
                  PropertyChanges {target: side_view; state:"BladeView"}
                  PropertyChanges {target: users_view; visible: false}
                  PropertyChanges {target: node_view; visible: false}
                  PropertyChanges {target: metrics_view; visible: false}
                  PropertyChanges {target: jobs_view; visible: false}
                  PropertyChanges {target: terminal_view; visible: false}
                  PropertyChanges {target: root; update_state: 1}
            },
            State {
               name: "NodeView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"NodeView"}
                    PropertyChanges {target: side_view; state:"NodeView"}
                    PropertyChanges {target: users_view; visible: false}
                    PropertyChanges {target: node_view; visible: true}
                    PropertyChanges {target: metrics_view; visible: false}
                    PropertyChanges {target: terminal_view; visible: false}
                    PropertyChanges {target: jobs_view; visible: false}
                    PropertyChanges {target: root; update_state: 2}
            },
            State {
                name: "UsersView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"UsersView"}
                    PropertyChanges {target: side_view; state:"UsersView"}
                    PropertyChanges {target: users_view; visible: true}
                    PropertyChanges {target: node_view; visible: false}
                    PropertyChanges {target: metrics_view; visible: false}
                    PropertyChanges {target: jobs_view; visible: false}
                    PropertyChanges {target: terminal_view; visible: false}
                    PropertyChanges {target: root; update_state: 3}
            },
            State {
                name: "MetricView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"MetricView"}
                    PropertyChanges {target: side_view; state:"MetricView"}
                    PropertyChanges {target: users_view; visible: false}
                    PropertyChanges {target: node_view; visible: false}
                    PropertyChanges {target: metrics_view; visible: true}
                    PropertyChanges {target: jobs_view; visible: false}
                    PropertyChanges {target: terminal_view; visible: false}
                    PropertyChanges {target: root; update_state: 4}
            },
            State {
                name: "TerminalView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"TerminalView"}
                    PropertyChanges {target: side_view; state:"TerminalView"}
                    PropertyChanges {target: terminal_view; visible: true}
                    PropertyChanges {target: node_view; visible: false}
                    PropertyChanges {target: users_view; visible: false}
                    PropertyChanges {target: metrics_view; visible: false}
                    PropertyChanges {target: jobs_view; visible: false}
                    PropertyChanges {target: root; update_state: 5}
            }
        ]
    }
    Settings {
        id: settings
        category: "Fermer"
        property alias notify_toggl: root.notify_toggl
        property alias show_all_jobs: root.show_all_jobs
        property alias swap_jobs_name: root.swap_jobs_name
        property alias transient_toggl: root.transient_toggl
        property alias farm_usage_toggl: root.farm_usage_toggl
        property alias supervisor_mode: root.supervisor_mode
        property string custom_aligntype_s: "AlignCenter"
        property string view_states: "JobView"
        property string python_edit: "import af\nimport afermer"
        property string notif_color: "#1d262b"
    }
    property bool notify_toggl:true
    property var custom_aligntype: Text.AlignLeft
    property bool transient_toggl:true
    property bool farm_usage_toggl: false
    property bool supervisor_mode: false
    property string side_state: "Info"
    property int update_state: 99
    property string filtered_text: ""
    property int task_height: 24
    property alias job_context_menu: job_context_menu_.item
    property bool swap_jobs_name: false
    property bool show_all_jobs: false
    property bool expand_jobs_goups: false
    property int selcted_id
    property var job_state
    property real mult_size: 1.0

    property string python_output: "empty"

    property int totalJobs
    property int doneJobs
    property int waitingJobs
    property int runningJobs
    property int errorJobs
    Item {
        id:item_align
        state:settings.custom_aligntype_s
        states: [
            State {
              name: "AlignCenter"
                  PropertyChanges {target: root; custom_aligntype: Text.AlignHCenter}
                  PropertyChanges {target: main_menu; center_opacity:0.78}
                  PropertyChanges {target: main_menu; left_opacity:0.38}
                  PropertyChanges {target: main_menu; right_opacity:0.38}
            },
            State {
              name: "AlignLeft"
                  PropertyChanges {target: root; custom_aligntype: Text.AlignLeft}
                  PropertyChanges {target: main_menu; center_opacity:0.38}
                  PropertyChanges {target: main_menu; left_opacity:0.78}
                  PropertyChanges {target: main_menu; right_opacity:0.38}
            },
            State {
              name: "AlignRight"
                  PropertyChanges {target: root; custom_aligntype: Text.AlignRight}
                  PropertyChanges {target: main_menu; center_opacity:0.38}
                  PropertyChanges {target: main_menu; left_opacity:0.38}
                  PropertyChanges {target: main_menu; right_opacity:0.78}
            }
        ]
    }
    /*
    WorkerScript {
        id: myWorker
        source: "timeChange.js"

        onMessage: console.log("gg "+messageObject.reply)
    }*/
    function timeChanged()  {
        if (General.serverExist()){
            if (update_state==0){
                selcted_id=JobsModel.getSelectedIds()[0]
                if (selcted_id!=0){
                    JobsModel.updateInteraction(filtered_text)
                    JobsModel.setShowAllJobs(show_all_jobs)
                    //console.log("gg "+JobsModel.areJobsDone().length)

                    //console.log("selcted_id "+selcted_id)
                    if (root.side_state=="Tasks"){
                        TasksModel.updateTasksByJobID(selcted_id)
                    }
                    //--- Title State Info
                    window_root.title=JobsModel.getCurrentItemProgress()+"%"+" - "+JobsModel.getCurrentItemName()+"  ::  AFermer  ::  Render Manager"

                    job_state=JobsModel.getCurrentItemState()
                    if (JobState.OFFLINE==job_state){
                        window_root.title="Stop  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.ERROR==job_state){
                        window_root.title="Error  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.READY==job_state){
                        window_root.title="Waiting  ::  AFermer  ::  Render Manager"
                    }
                    //-------------------------------------
                }
                else{
                    window_root.title="No Jobs Selected"
                }
            }
            if (update_state==1){
                //BladesModel.updateInteraction(filtered_text)
            }
            if (update_state==2){
                JobsModel.updateInteraction(filtered_text)
                BladesModel.updateInteraction(filtered_text)
                JobsModel.updateGroupNodeSize()
                node_view.linesUpdate.call()
                //JobsModel.arangeNodes()
            }
            if (update_state==3){
                UsersModel.updateInteraction(filtered_text)
            }

            if(notify_toggl){
                if (JobsModel.areJobsDone().length){
                    notifyer.custom_text=JobsModel.areJobsDone()
                    notifyer.show()
                }
            }

        }
        else{
            background.source = "NoConnection.qml"
        }

        BladesModel.updateInteraction(filtered_text)

        totalJobs=JobsModel.totalJobs();
        doneJobs=JobsModel.doneJobs();
        waitingJobs=JobsModel.readyJobs();
        runningJobs=JobsModel.runningJobs();
        errorJobs=JobsModel.errorJobs();
    }
    Timer  {
        id: elapsedTimer
        interval: 1000;
        running: true;
        repeat: true;
        onTriggered: timeChanged()
    }

    Component {
        id: comp_spliter

        Rectangle {
            color: "white"
            width: 1
            height: 1
            opacity: 0.3
        }
    }

    SplitView {
        orientation: Qt.Horizontal
        height:root.height-main_menu.height+5
        handleDelegate:comp_spliter
        width:root.width
        anchors.top:main_menu.bottom
        anchors.topMargin: -5
        Item{
            width: root.width/1.35
            height: parent.height
            JobsView{
                id:jobs_view
                width: parent.width
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5

                property var job_minimum_Width: {"progress": 130,
                                                 "elapsed": 100,
                                                 "user": 35,
                                                 "job_name": root.width-side_view.width-750,
                                                 "slots": 40,
                                                 "priority": 60,
                                                 "started": 100,
                                                 "software": 100,
                                                 "approximate_time": 70}
                property int selection

                Component{
                    id:job_delegate
                    JobDelegate{
                        aligntype: custom_aligntype
                        //height: 45
                        width: parent.width
                    }
                }
                ScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 35
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
                            implicitWidth: 12
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
                          id: jobs_ListView
                          anchors.fill: parentss
                          delegate:job_delegate
                          model: JobsModel.jobsModel
                          focus: true
                          cacheBuffer:40
                        }
                }


                InputDialog {
                    id: popJobSetPriority
                    title: "Set Priority"

                    TextInput {
                        id: set_priority_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetPriority.close();
                            JobsModel.setPriority(set_priority_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetPriority.close();
                                JobsModel.setPriority(set_priority_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetHostsMask
                    title: "Set Hosts Mask"

                    TextInput {
                        id: set_host_mask_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetHostsMask.close();
                            JobsModel.setBladeMask(set_host_mask_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetHostsMask.close();
                                JobsModel.setBladeMask(set_host_mask_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetHostsMaskExclude
                    title: "Set Hosts Mask Exclude"

                    TextInput {
                        id: set_host_mask_exclude_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetHostsMaskExclude.close();
                            JobsModel.setBladeMaskExclude(set_host_mask_exclude_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetHostsMaskExclude.close();
                                JobsModel.setBladeMaskExclude(set_host_mask_exclude_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetDependMask
                    title: "Set Depend Mask"

                    TextInput {
                        id: set_depend_mask_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetDependMask.close();
                            JobsModel.setJobDependMask(set_depend_mask_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetDependMask.close();
                                JobsModel.setJobDependMask(set_depend_mask_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetErrorAvoidBlade
                    title: "Set Error Avoid Blade"

                    TextInput {
                        id: set_error_avoid_blade_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetErrorAvoidBlade.close();
                            JobsModel.setErrorAvoidBlade(set_error_avoid_blade_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetErrorAvoidBlade.close();
                                JobsModel.setErrorAvoidBlade(set_error_avoid_blade_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetErrorRetries
                    title: "Set Error Retries"

                    TextInput {
                        id: set_error_retries_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetErrorRetries.close();
                            JobsModel.setTasksErrorRetries(set_error_retries_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetErrorRetries.close();
                                JobsModel.setTasksErrorRetries(set_error_retries_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetErrorForgiveTime
                    title: "Set Error Forgive Time"

                    TextInput {
                        id: set_error_forgive_time_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetErrorForgiveTime.close();
                            JobsModel.setTasksErrorForgiveTime(set_error_forgive_time_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetErrorForgiveTime.close();
                                JobsModel.setTasksErrorForgiveTime(set_error_forgive_time_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetMaxRunTime
                    title: "Set Tasks Max Run Time"

                    TextInput {
                        id: set_max_run_time_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetMaxRunTime.close();
                            JobsModel.setTaskMaxRunTime(set_max_run_time_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetMaxRunTime.close();
                                JobsModel.setTaskMaxRunTime(set_max_run_time_text_input_dialog.text)
                            }
                        }
                    }
                }

                InputDialog {
                    id: popJobSetMaxRunningTask
                    title: "Set Max Running Tasks"

                    TextInput {
                        id: set_max_running_task_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetMaxRunningTask.close();
                            JobsModel.setTasksRunningMax(set_max_running_task_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetMaxRunningTask.close();
                                JobsModel.setTasksRunningMax(set_max_running_task_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetMaxRunningTaskPerBlade
                    title: "Set Max Running Task Per Blade"

                    TextInput {
                        id: set_max_running_task_per_blade_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetMaxRunningTaskPerBlade.close();
                            JobsModel.setTasksRunningPerBlades(set_max_running_task_per_blade_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetMaxRunningTaskPerBlade.close();
                                JobsModel.setTasksRunningPerBlades(set_max_running_task_per_blade_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetSlots
                    title: "Set Slots"

                    TextInput {
                        id: set_slots_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetSlots.close();
                            JobsModel.setJobSlots(set_slots_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetSlots.close();
                                JobsModel.setJobSlots(set_slots_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetLifeTime
                    title: "Set Life Time"

                    TextInput {
                        id: set_life_time_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetLifeTime.close();
                            JobsModel.setJobLifeTime(set_life_time_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetLifeTime.close();
                                JobsModel.setJobLifeTime(set_life_time_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetWaitTime
                    title: "Set Wait Time"

                    TextInput {
                        id: set_wait_time_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetLifeTime.close();
                            JobsModel.setJobWaitTime(set_wait_time_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetWaitTime.close();
                                JobsModel.setJobWaitTime(set_wait_time_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetAnnotation
                    title: "Set Annotation"

                    TextInput {
                        id: set_annotation_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetAnnotation.close();
                            JobsModel.setJobSetAnnotation(set_annotation_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetAnnotation.close();
                                JobsModel.setJobSetAnnotation(set_annotation_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetOS
                    title: "Set OS Needed"

                    TextInput {
                        id: set_os_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetOS.close();
                            JobsModel.setJobOS(set_os_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetOS.close();
                                JobsModel.setJobOS(set_os_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetMemory
                    title: "Set Need Memory"

                    TextInput {
                        id: set_memory_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetMemory.close();
                            JobsModel.setJobNeedMemory(set_memory_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetMemory.close();
                                JobsModel.setJobNeedMemory(set_memory_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetHDD
                    title: "Set Need HDD"

                    TextInput {
                        id: set_hdd_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetHDD.close();
                            JobsModel.setJobNeedHDD(set_hdd_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetHDD.close();
                                JobsModel.setJobNeedHDD(set_hdd_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popJobSetPostCommand
                    title: "Set Post Command"

                    TextInput {
                        id: set_post_command_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true

                        color: "white"
                        Keys.onReturnPressed: {
                            popJobSetPostCommand.close();
                            JobsModel.setJobPostCommand(set_post_command_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popJobSetPostCommand.close();
                                JobsModel.setJobPostCommand(set_post_command_input_dialog.text)
                            }
                        }
                    }
                }
                InfoDialog {
                    id: popJobFolderIsNotExistDialog
                    title: "Folder Is Not Exist"
                }
                InfoDialog {
                    id: popJobLogDialog
                    title: "Job Log"
                }
                InfoDialog {
                    id: popJobErrorDialog
                    title: "Error Blades"
                }
                Loader {
                    id: job_context_menu_
                    anchors.fill: parent
                    source: jobs_ListView.currentIndex!=-1 ? "JobContextMenu.qml" : ""
                }
            }


            BladesView{
                id:blades_view

                width: parent.width
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5

                property var blade_minimum_Width: { "header_base": 100,
                                                                              "state": 45,
                                                                              "name": 120,
                                                                                "blades_group": 80,
                                                                                "cpu": 40,
                                                                                "memory": 90,
                                                                                "network": 90,
                                                                                "slots": 40,
                                                                                "av_slots": 65,
                                                                                "eplased": 90,
                                                                                "address": 120}


                Component{
                    id:blade_delegate
                    BladeDelegate{
                        aligntype: custom_aligntype
                        width: parent.width
                        height: 45
                    }
                }
                ScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 35
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
                            implicitWidth: 12
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
                          id: blades_ListView
                          anchors.fill: parent
                          highlightFollowsCurrentItem: false
                          delegate:blade_delegate
                          model: BladesModel.bladesModel
                          cacheBuffer:40
                        }
                }
                InputDialog {
                    id: popBladeSetMaxSlots
                    title: "Set Max Slots"

                    TextInput {
                        id: set_max_slots_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"

                        Keys.onReturnPressed: {
                            popBladeSetMaxSlots.close();
                            BladesModel.actCapacity(blades_ListView.currentItem.v_blade_id,set_max_slots_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popBladeSetMaxSlots.close();
                                BladesModel.actCapacity(blades_ListView.currentItem.v_blade_id,set_max_slots_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popBladeSetMaxTasks
                    title: "Set Max Tasks"

                    TextInput {
                        id: set_max_tasks_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"

                        Keys.onReturnPressed: {
                            popBladeSetMaxTasks.close();
                            BladesModel.actMaxTasks(blades_ListView.currentItem.v_blade_id,set_max_tasks_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popBladeSetMaxTasks.close();
                                BladesModel.actMaxTasks(blades_ListView.currentItem.v_blade_id,set_max_tasks_input_dialog.text)
                            }
                        }
                    }
                }
                InfoDialog {
                    id: popBladeLogDialog
                    title: "Blade Log"
                }
                BladeContextMenu{
                    id: blade_context_menu
                }
            }

            NodeView{
                id:node_view

                width: parent.width
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            UsersView{
                id:users_view

                width: parent.width
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5

                property var user_minimum_Width: {"user_machine_ip": 140,
                                                 "user": 45,
                                                 "tasks_running": root.width-side_view.width-400,
                                                 "jobs_size": 85,
                                                 "priority": 80}
                property int selection

                Component{
                    id:user_delegate
                    UserDelegate{
                        aligntype: custom_aligntype
                        height: 45

                    }
                }
                ScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 35
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
                            implicitWidth: 12
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
                          id: users_ListView
                          anchors.fill: parentss
                          delegate:user_delegate
                          model: UsersModel.usersModel
                          focus: true
                          cacheBuffer:40
                        }
                }
                UserContextMenu{
                    id: user_context_menu
                }
                ColorDialog {
                    id: color_dialog
                    visible: false
                    modality: Qt.WindowModal
                    title: "Choose a color"
                    onAccepted: {
                        UsersModel.setUserColor(users_ListView.currentItem.v_user_name,color_dialog.color)
                    }
                }
                InfoDialog {
                    id: popUserLogDialog
                    title: "User Log"
                }
                InputDialog {
                    id: popUserSetPriority
                    title: "Set Priority"

                    TextInput {
                        id: set_user_priority_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"
                        Keys.onReturnPressed: {
                            popUserSetPriority.close();
                            UsersModel.setPriority(set_user_priority_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popUserSetPriority.close();
                                UsersModel.setPriority(set_user_priority_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popUserMaxRunningTasks
                    title: "Set Max Running Tasks"

                    TextInput {
                        id: set_user_max_running_tasks_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"
                        Keys.onReturnPressed: {
                            popUserMaxRunningTasks.close();
                            UsersModel.setMaxRunningTask(set_user_max_running_tasks_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popUserMaxRunningTasks.close();
                                UsersModel.setMaxRunningTask(set_user_max_running_tasks_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popUserBladesMask
                    title: "Set Blades Mask"

                    TextInput {
                        id: set_user_blades_mask_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"
                        Keys.onReturnPressed: {
                            popUserBladesMask.close();
                            UsersModel.setBladeMask(set_user_blades_mask_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popUserBladesMask.close();
                                UsersModel.setBladeMask(set_user_blades_mask_text_input_dialog.text)
                            }
                        }
                    }
                }
                InputDialog {
                    id: popUserBladesMaskExlude
                    title: "Set Exlude Blades Mask"

                    TextInput {
                        id: set_user_exlude_blades_mask_text_input_dialog
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.85
                        height: 20
                        text: "99"
                        opacity: 0.9
                        selectByMouse: true
                        inputMethodHints:Qt.ImhDigitsOnly

                        color: "white"
                        Keys.onReturnPressed: {
                            popUserBladesMaskExlude.close();
                            UsersModel.setBladeExcludeMask(set_user_exlude_blades_mask_text_input_dialog.text)
                        }
                    }
                    Rectangle {
                        height: 25
                        width:59
                        color: "#374a52"

                        anchors.bottom: parent.bottom
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 10
                        anchors.bottomMargin: 10
                        opacity:0.85
                        layer.enabled: true
                        layer.effect: DropShadow {
                        transparentBorder: true
                        samples: 12
                        radius:4
                        }
                        Text {
                            text: "Ok"
                            anchors.centerIn: parent
                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                popUserBladesMaskExlude.close();
                                UsersModel.setBladeExcludeMask(set_user_exlude_blades_mask_text_input_dialog.text)
                            }
                        }
                    }
                }
            }

            MetricsView{
                id:metrics_view

                width: parent.width
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            TerminalView{
                id:terminal_view

                width: parent.width
                height: parent.height-5
                anchors.top:parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
            }
        }/*
        Rectangle {
            id: side_view
            width: 200
            height: parent.height
            //anchors.left: parent.right
            //anchors.leftMargin: 5
            //x:side_view.x-5

            color: "lightblue"
            Text {
                text: "View 1"
                anchors.centerIn: parent
            }
        }
*/
        SideView{
            id: side_view
            width: 300
            height: parent.height
            Layout.minimumWidth: 200
            //x:parent.width-parent.width/4
        }
    }


    /*
    Loader {
        id: side_view
        width: parent.width/4
        height: parent.height
        x:parent.width-parent.width/4
        //source: jobs_ListView.currentIndex!=-1 ? "SideView.qml" : ""
        source: "SideView.qml"
    }
*/

    MainMenu{
        id: main_menu
    }

    UsageBar{
        width:blades_view.width+10
        anchors.left: parent.left
        anchors.bottom:  parent.bottom
        visible: farm_usage_toggl
    }



    About {
        id: popAboutDialog
    }
    Window {
        id: notifyer
        title: "AFermer Info"
        width: 300
        height: 70
        x:Screen.width - width-20
        y:Screen.height
        flags:Qt.WindowStaysOnTopHint|Qt.FramelessWindowHint
        modality: Qt.NonModal
        color: "transparent"
        property string custom_text:''
        Rectangle{
            anchors.fill: parent
            color: settings.notif_color
            opacity: 0.8
            layer.enabled: true
            ScrollView {
                width: 270
                height: 50
                anchors.top:parent.top
                anchors.topMargin: 10
                anchors.left: parent.left
                anchors.leftMargin: 10
                highlightOnFocus:true
                style: ScrollViewStyle {
                    transientScrollBars: root.transient_toggl
                }
                Text{
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    font.family: robotoRegular.name
                    font.weight: Font.Light
                    font.letterSpacing:1.2
                    color:   "white"
                    text:    notifyer.custom_text
                }
            }
            Text{
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 20
                color:   "white"
                text:    "Jobs Are Done"
            }
            Rectangle {
                height: 20
                width:50
                color: settings.notif_color
                layer.enabled: true
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.bottomMargin: 2
                Text {
                    text: "Close"
                    anchors.centerIn: parent
                    color: "white"
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        JobsModel.clearNotify()
                        notifyer.close();
                    }
                }
            }
        }
    }

    ColorDialog {
        id: id_notif_color_dialog
        visible: false
        modality: Qt.WindowModal
        title: "Choose a color"
        onAccepted: {
            settings.notif_color=id_notif_color_dialog.color
        }
    }


    InputDialog {
        title: "popSizeMultDialog"
        id: size_mult_dialog

        TextInput {
            id: set_size_mult_input_dialog
            anchors.top: parent.top
            anchors.topMargin: 28
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width*0.85
            height: 20
            text: "99"
            opacity: 0.9
            selectByMouse: true

            color: "white"
            Keys.onReturnPressed: {
                root.mult_size=set_size_mult_input_dialog.text
            }
        }
        Rectangle {
            height: 25
            width:59
            color: "#374a52"

            anchors.bottom: parent.bottom
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 10
            anchors.bottomMargin: 10
            opacity:0.85
            layer.enabled: true
            layer.effect: DropShadow {
            transparentBorder: true
            samples: 12
            radius:4
            }
            Text {
                text: "Ok"
                anchors.centerIn: parent
                color: "white"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.mult_size=set_size_mult_input_dialog.text
                }
            }
        }
    }

    Menu {
       id: main_contextMenu
       title: "More Stuff"
       style: MenuStyle {
           frame: Rectangle {
               color: "#293238"
           }
           itemDelegate {
               background: Rectangle {
                   color: styleData.selected ? "#353f47" : "#293238"//"#191e22"
               }
               label: Text {
                   font.pixelSize: 14
                   font.family: robotoRegular.name
                   font.weight: Font.Light
                   font.letterSpacing:1.2
                   color:   "white"
                   text:    styleData.text
                   opacity: styleData.selected ? 1 : 0.9
               }
           }
       }
       MenuItem {
           id: supervisormode_menuitem
           text: "Supervisor Mode (not fully impl.)"
           checkable: true
           checked: supervisor_mode
           onToggled: {
               supervisor_mode=supervisormode_menuitem.checked
           }
       }
       MenuSeparator { }
       MenuItem {
           id: showalljobs_menuitem
           text: "Show All Jobs"
           checkable: true
           checked:show_all_jobs
           onToggled: {
               show_all_jobs=showalljobs_menuitem.checked
               JobsModel.setShowAllJobs(show_all_jobs)
               JobsModel.updateInteraction(filtered_text)
           }
           //checked:  ? false : true
       }
       MenuItem {
           id: farm_usage_menuitem
           checkable: true
           text: "Farm Usage Bar"
           checked:farm_usage_toggl
           onToggled: {
               farm_usage_toggl=farm_usage_menuitem.checked
           }
       }
       MenuItem {
           id: scrollbar_transient_menuitem
           checkable: true
           text: "ScrollBar Transient"
           checked:transient_toggl
           onToggled: {
               transient_toggl=scrollbar_transient_menuitem.checked
               //console.log(" rowCount():"+scrollbar_transient_menuitem.checked)
           }
       }
       MenuItem {
           id: swap_jobs_names_menuitem
           checkable: true
           text: "Swap Jobs Names"
           checked:swap_jobs_name
           onToggled: {
               swap_jobs_name=swap_jobs_names_menuitem.checked
           }
       }
       MenuItem {
           id: expand_jobs_goups_menuitem
           checkable: true
           text: "Expand Jobs Group (not impl.)"
           checked:expand_jobs_goups
           onToggled: {
               expand_jobs_goups=swap_jobs_names_menuitem.checked
           }
       }
       MenuItem {
           id: size_multiplier_menuitem
           text: "Size Multiplier (not impl.)"
           onTriggered:{
                set_size_mult_input_dialog.text=root.mult_size
                size_mult_dialog.show()
           }
       }
       MenuSeparator { }
       MenuItem {
           id: notif_my_jobs_complete_menuitem
           checkable: true
           text: "Notifier My Job Completed "
           checked: notify_toggl
           onTriggered: {
               notify_toggl=notif_my_jobs_complete_menuitem.checked
           }
       }
       MenuItem {
           id: notif_blades_memory_overflow_menuitem
           checkable: true
           text: "Notifier Blades Memory Overflow (not impl.)"
       }
       MenuItem {
           id: notif_color_menuitem
           text: "Notifier Color"
           onTriggered: {
                id_notif_color_dialog.open()
           }
       }
       MenuSeparator { }
       /*
       MenuItem {
           checkable: true
           text: "Show Afanasy Jobs (not impl.)"
       }
       MenuItem {
           checkable: true
           text: "Show Afanasy Blades (not impl.)"
       }
       MenuItem {
           checkable: true
           text: "Show Qube Jobs (not impl.)"
       }
       MenuItem {
           checkable: true
           text: "Show Qube Blades (not impl.)"
       }
       MenuSeparator { }*/
       MenuItem {
           text: "Help (not impl.)"
       }
       MenuItem {
           text: "About"
           onTriggered:{
               popAboutDialog.show()
           }
       }

    }
}
