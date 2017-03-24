import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4




Rectangle {
        width: parent.width
        height: 84
        color: "#191e22"
        Layout.fillWidth: true
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            samples: 17
            radius:5
        }
        states: [
            State {
              name: "JobView"
                    PropertyChanges {target: button_jobs; st_opacity:0.78}
                    PropertyChanges {target: button_blades; st_opacity:0.38}
                    PropertyChanges {target: button_nodes; st_opacity:0.38}
                    PropertyChanges {target: button_users; st_opacity:0.38}
                    PropertyChanges {target: button_metric; st_opacity:0.38}
            },
            State {
                  name: "BladeView"
                  PropertyChanges {target: button_jobs; st_opacity:0.38}
                  PropertyChanges {target: button_blades; st_opacity:0.78}
                  PropertyChanges {target: button_nodes; st_opacity:0.38}
                  PropertyChanges {target: button_users; st_opacity:0.38}
                  PropertyChanges {target: button_metric; st_opacity:0.38}
            },
            State {
                    name: "NodeView"
                    PropertyChanges {target: button_jobs; st_opacity:0.38}
                    PropertyChanges {target: button_blades; st_opacity:0.38}
                    PropertyChanges {target: button_nodes; st_opacity:0.78}
                    PropertyChanges {target: button_users; st_opacity:0.38}
                    PropertyChanges {target: button_metric; st_opacity:0.38}
            },
            State {
                    name: "UsersView"
                    PropertyChanges {target: button_jobs; st_opacity:0.38}
                    PropertyChanges {target: button_blades; st_opacity:0.38}
                    PropertyChanges {target: button_nodes; st_opacity:0.38}
                    PropertyChanges {target: button_users; st_opacity:0.78}
                    PropertyChanges {target: button_metric; st_opacity:0.38}
            },
            State {
                    name: "MetricView"
                    PropertyChanges {target: button_jobs; st_opacity:0.38}
                    PropertyChanges {target: button_blades; st_opacity:0.38}
                    PropertyChanges {target: button_nodes; st_opacity:0.38}
                    PropertyChanges {target: button_users; st_opacity:0.38}
                    PropertyChanges {target: button_metric; st_opacity:0.78}
            },
            State {
                    name: "TerminalView"
                    PropertyChanges {target: button_jobs; st_opacity:0.38}
                    PropertyChanges {target: button_blades; st_opacity:0.38}
                    PropertyChanges {target: button_nodes; st_opacity:0.38}
                    PropertyChanges {target: button_users; st_opacity:0.38}
                    PropertyChanges {target: button_metric; st_opacity:0.38}
                    PropertyChanges {target: button_terminal; st_opacity:0.78}
            }
        ]
        Rectangle {
            id: tooltip
            anchors.centerIn: parent
            width: 900
            height: 59
            color: "white"
            opacity: 0.2
            layer.enabled: true
        }

        Rectangle {
            id: vertical_divider
            anchors.right: button_jobs.left
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: tooltip.height/1.5
            color: "black"
            opacity: 0.2
            layer.enabled: true
        }
        Rectangle {
            id: search_bar
            anchors.right: vertical_divider.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 15
            width: 220
            height: 30
            color: "white"
            opacity: 0.1
            layer.enabled: false
        }
        Image {
            opacity: 0.3
            anchors.left: search_bar.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            source:"icons/search.svg"
            sourceSize.width: 18
            sourceSize.height: 18
        }
        TextInput {
            id:search_text_input
            opacity: 0.7
            anchors.left: search_bar.left
            anchors.leftMargin: 40
            anchors.verticalCenter: parent.verticalCenter
            width:search_bar.width-60
            clip: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 14
            font.family: robotoRegular.name
            font.weight: Font.Normal
            color: "white"
            Text {
                id: placeholderText
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                visible: !(parent.text.length || parent.inputMethodComposing)
                font: parent.font
                text: "Search..."
                opacity: 0.4
                color: "white"
            }
            onTextChanged: {
                root.filtered_text=search_text_input.text
            }

        }
           

        Image {
            opacity: 0.3
            anchors.right: search_bar.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            source:"icons/remove.svg"
            sourceSize.width: 14
            sourceSize.height: 14
            MouseArea {
                acceptedButtons: Qt.LeftButton
                anchors.fill: parent
                onClicked: {
                        search_text_input.text = ""
                    }
                }
        }

        ToolTipButton{
            id: button_jobs
            anchors.right: button_nodes.right
            anchors.rightMargin: 90
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/tasks.svg"
            onClicked:{
                    item_states.state="JobView"
                    JobsModel.updateInteraction(filtered_text)
                }
        }
        ToolTipButton{
            id:button_blades
            anchors.right: button_nodes.right
            anchors.rightMargin: 45
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/blades.svg"
            onClicked:{
                    item_states.state="BladeView"
                    BladesModel.updateInteraction(filtered_text)
                }

        }
        ToolTipButton{
            id: button_nodes
            anchors.centerIn: tooltip;
            image:'icons/nodes.svg'
            onClicked:{
                    item_states.state="NodeView"
                }
        }
        ToolTipButton{
            id: button_users
            anchors.left: button_nodes.left
            anchors.leftMargin: 45
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/users.svg"
            onClicked:{
                    item_states.state="UsersView"
                    UsersModel.updateInteraction(filtered_text)
                }
        }
        ToolTipButton{
            id: button_metric
            anchors.left: button_nodes.left
            anchors.leftMargin: 90
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/metric.svg"
            onClicked:{
                    item_states.state="MetricView"
                }
        }

        ToolTipButton{
            id: button_terminal
            anchors.left: button_nodes.left
            anchors.leftMargin: 135
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/terminal.svg"
            onClicked:{
                    item_states.state="TerminalView"
                }
            visible:supervisor_mode
        }

        Rectangle {
            id: vertical_right_divider
            anchors.left: supervisor_mode ? button_terminal.right : button_metric.right
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: tooltip.height/1.5
            color: "black"
            opacity: 0.2
            layer.enabled: true
        }


        property real left_opacity: 0.38
        ToolTipButton{
            id: align_left
            anchors.left: vertical_right_divider.left
            anchors.leftMargin: 10
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/align_left.svg"
            st_opacity: left_opacity
            size_x: 18
            size_y: 18
            onClicked:{
                    item_align.state="AlignLeft"
                }
        }
        property real center_opacity: 0.38
        ToolTipButton{
            id: align_center
            anchors.left: vertical_right_divider.left
            anchors.leftMargin: 50
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/align_center.svg"
            st_opacity: center_opacity
            size_x: 18
            size_y: 18
            onClicked:{
                    item_align.state="AlignCenter"
                }
        }
        property real right_opacity: 0.38
        ToolTipButton{
            id: align_right
            anchors.left: vertical_right_divider.left
            anchors.leftMargin: 90
            anchors.verticalCenter: tooltip.verticalCenter
            image:"icons/align_right.svg"
            st_opacity: right_opacity
            size_x: 18
            size_y: 18
            onClicked:{
                    item_align.state="AlignRight"
                }
        }

        Image {
            id: logo
            opacity: 90
            anchors.left: parent.left
            anchors.leftMargin: 23
            anchors.verticalCenter: parent.verticalCenter
            sourceSize.width: 151
            sourceSize.height: 39
            anchors.verticalCenterOffset: 5
	    
            source: "icons/fermer.svg"
        }
        Image {
            id: dots_menu
            y:  parent.height/2-10
            x: parent.width-60
            source: "icons/dots_menu.svg"
            sourceSize.width: 26
            sourceSize.height: 21
            MouseArea {
                            acceptedButtons: Qt.LeftButton
                            anchors.fill: parent
                            propagateComposedEvents: true
                            onReleased: {
                                    if (mouse.button === Qt.LeftButton) {
                                        main_contextMenu.popup()
                                    }
                                }
                            }
        }

    }
