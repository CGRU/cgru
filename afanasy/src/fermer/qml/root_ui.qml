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

Item {
    id: root
    visible: false
    Component.onCompleted:{
        visible=true
    }
    Item {
        id:item_states
        state:"JobView"
        states: [
            State {
              name: "JobView"
                  PropertyChanges {target: blades_view; visible: false}
                  PropertyChanges {target: main_menu; state:"JobView"}
                  PropertyChanges {target: side_view; state:"JobView"}
                  PropertyChanges {target: not_implemented_view; visible: false}
                  PropertyChanges {target: node_view; visible: false}
                  PropertyChanges {target: metrics_view; visible: false}
                  PropertyChanges {target: jobs_view; visible: true}
                  PropertyChanges {target: root; update_state: 0}
            },
            State {
              name: "BladeView"
                  PropertyChanges {target: blades_view; visible: true}
                  PropertyChanges {target: main_menu; state:"BladeView"}
                  PropertyChanges {target: side_view; state:"BladeView"}
                  PropertyChanges {target: not_implemented_view; visible: false}
                  PropertyChanges {target: node_view; visible: false}
                  PropertyChanges {target: metrics_view; visible: false}
                  PropertyChanges {target: jobs_view; visible: false}
                  PropertyChanges {target: root; update_state: 1}
            },
            State {
               name: "NodeView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"NodeView"}
                    PropertyChanges {target: side_view; state:"NodeView"}
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
                    PropertyChanges {target: not_implemented_view; visible: true}
                    PropertyChanges {target: node_view; visible: false}
                    PropertyChanges {target: metrics_view; visible: false}
                    PropertyChanges {target: jobs_view; visible: false}
            },
            State {
                name: "MetricView"
                    PropertyChanges {target: blades_view; visible: false}
                    PropertyChanges {target: main_menu; state:"MetricView"}
                    PropertyChanges {target: side_view; state:"MetricView"}
                    PropertyChanges {target: not_implemented_view; visible: false}
                    PropertyChanges {target: node_view; visible: false}
                    PropertyChanges {target: metrics_view; visible: true}
                    PropertyChanges {target: jobs_view; visible: false}
                    PropertyChanges {target: root; update_state: 4}
            }
        ]
    }
    property var custom_aligntype: Text.AlignLeft
    property bool transient_toggl:false
    property bool farm_usage_toggl: true
    property string side_state: "Info"
    property int update_state: 99
    property string filtered_text: ""
    property int task_height: 24

    signal jobClicked

    property int totalJobs
    property int doneJobs
    property int waitingJobs
    property int runningJobs
    property int errorJobs
    Item {
        id:item_align
        state:"AlignCenter"
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
        if (update_state==0){
            JobsModel.updateInteraction(filtered_text)
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
            //--------------------
        }
        if (update_state==1){
            BladesModel.updateInteraction(filtered_text)
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
                                         "job_name": 230,
                                         "slots": 40,
                                         "priority": 60,
                                         "started": 100,
                                         "software": 100,
                                         "approximate_time": 142}
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

        JobContextMenu{
            id: job_context_menu
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
        width:jobs_view.width
        height: 400

        anchors.right: side_view.left
        anchors.rightMargin: 10
        anchors.top:main_menu.top
        anchors.topMargin: 50
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


    InfoDialog {
        id: popJobAboutDialog
        title: "About Fermer"
        width: 220
        height: 70
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
           id: usermode_menuitem
           text: "User Mode (not impl.)"
           checkable: true
           checked: supervisormode_menuitem.checked ? false : true
       }
       MenuItem {
           id: supervisormode_menuitem
           text: "Supervisor Mode (not impl.)"
           checkable: true
           checked: usermode_menuitem.checked ? false : true
       }
       MenuSeparator { }
       iconSource: "icons/remove.png"
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
               console.log(" rowCount():"+scrollbar_transient_menuitem.checked)
           }
       }
       MenuSeparator { }
       MenuItem {
           text: "About"
           onTriggered:{
               popJobAboutDialog.text=("     QT:                "+BladesModel.qt_version()
                                       +"\n     AFermer:       0.5.2")
                                       //+"\n\n     Developed In 'Platige Image'"
                                       //+"\n             www.platige.com")
               popJobAboutDialog.show()
           }
       }
       
    }
}
