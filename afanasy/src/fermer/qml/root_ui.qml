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
import QtQuick.Controls 1.0
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
                  PropertyChanges {target: root; update_state: 0}
                  PropertyChanges {target: not_implemented_view; visible: false}
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
                  PropertyChanges {target: root; update_state: 1}
                  PropertyChanges {target: not_implemented_view; visible: false}
            },
            State {
               name: "NodeView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"NodeView"}
                    PropertyChanges {target: side_view; state:"NodeView"}
                    PropertyChanges {target: not_implemented_view; visible: false}
                    PropertyChanges {target: users_view; visible: false}
                    PropertyChanges {target: node_view; visible: true}
                    PropertyChanges {target: metrics_view; visible: false}
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
                    PropertyChanges {target: not_implemented_view; visible: false}
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
                    PropertyChanges {target: root; update_state: 4}
                    PropertyChanges {target: not_implemented_view; visible: false}
            },
            State {
                name: "TerminalView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"TerminalView"}
                    PropertyChanges {target: side_view; state:"TerminalView"}
                    PropertyChanges {target: not_implemented_view; visible: true}
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

    signal jobClicked

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

    function timeChanged()  {
        if (General.serverExist()){
            if (update_state==0){
                if (jobs_ListView.currentIndex!=-1){
                    JobsModel.updateInteraction(filtered_text)
                    JobsModel.setShowAllJobs(show_all_jobs)
                    //console.log("gg "+JobsModel.areJobsDone().length)
                    if (root.side_state=="Tasks"){
                        TasksModel.updateTasksByJobID(jobs_ListView.currentItem.v_job_id)
                    }
                    //--- Title State Info
                    window_root.title=jobs_ListView.currentItem.v_progress+"%"+" - "+jobs_ListView.currentItem.v_block_name+"  ::  AFermer  ::  Render Manager"

                    if (JobState.OFFLINE==jobs_ListView.currentItem.v_state){
                        window_root.title="Stop  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.ERROR==jobs_ListView.currentItem.v_state){
                        window_root.title="Error  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.READY==jobs_ListView.currentItem.v_state){
                        window_root.title="Waiting  ::  AFermer  ::  Render Manager"
                    }
                }
                else{
                    window_root.title="No Jobs"
                }
            }
            if (update_state==1){
                BladesModel.updateInteraction(filtered_text)
            }
            if (update_state==2){
                JobsModel.updateInteraction(filtered_text)
                //BladesModel.updateInteraction(filtered_text)
                JobsModel.updateGroupNodeSize()
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
    JobsView{
        id:jobs_view
        width: root.width-side_view.width-18
        height: root.height-85


        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.bottom
        anchors.topMargin: 0

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
                inputMethodHints:Qt.ImhDigitsOnly

                color: "white"
                Keys.onReturnPressed: {
                    popJobSetHostsMask.close();
                    JobsModel.setHostMask(set_host_mask_text_input_dialog.text)
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
                        JobsModel.setHostMask(set_host_mask_text_input_dialog.text)
                    }
                }
            }
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
        width: jobs_view.width
        height: root.height-85


        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.bottom
        anchors.topMargin: 0

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
        BladeContextMenu{
            id: blade_context_menu
        }
    }

    NodeView{
        id:node_view
        width:jobs_view.width+18
        height: root.height-85

        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.bottom
    }

    UsersView{
        id:users_view
        width: root.width-side_view.width-18
        height: root.height-85

        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.bottom
        anchors.topMargin: 0

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
    }

    MetricsView{
        id:metrics_view
        width:jobs_view.width
        height: 400

        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.top
        anchors.topMargin: 50
    }


    Item{
        id:not_implemented_view
        width: 200
        height:100

        anchors.centerIn: blades_view


        Text {
            anchors.centerIn: parent
            color: "white"
            opacity: 0.45
            text:"Not Implemented Yet"
            font.letterSpacing:1.2
            font { family: robotoRegular.name; pixelSize: 18}
        }
    }

    onJobClicked: {
        side_view.jobClicked.call()
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
    SideView{
        id: side_view
        width: parent.width/4
        height: parent.height
        x:parent.width-parent.width/4
    }

    MainMenu{
        id: main_menu
    }

    UsageBar{
        width:blades_view.width
        anchors.left: blades_view.left
        anchors.bottom:  blades_view.bottom
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
            color: "#1d262b"
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
                color: "#1d262b"
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
           text: "Supervisor Mode (not impl.)"
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
       MenuSeparator { }
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
