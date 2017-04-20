import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:blades_view
    color: "#2b373d"

    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        samples: 17
        radius:2
    }
    Rectangle {
        id: header_layout
        color: "#2b373d"
        width:blades_view.width
        height:35
        anchors.top: blades_view.top

        layer.enabled: true
        
        RowLayout {
            id: layout
            anchors.fill: parent
            anchors.topMargin: -(parent.height-40)
            Text {
                id:header_base
                Layout.preferredWidth: blades_view.blade_minimum_Width.header_base*1

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Base"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        BladesModel.sortingChangeState()
                        BladesModel.multiSorting(1)
                    }
                }
            }
            Text {
                id:header_state
                Layout.preferredWidth: blades_view.blade_minimum_Width.state*1

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"State"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        BladesModel.sortingChangeState()
                        BladesModel.multiSorting(2)
                    }
                }
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.name*1

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Name"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            /*
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.blades_group*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Blades Gr."
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }*/
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.cpu*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"CPU"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.memory*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Memory"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.network*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Network"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.slots*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Slots"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.av_slots*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Av.Slots"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.eplased*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Eplased"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: blades_view.blade_minimum_Width.address*1
                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text:"Address"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
        }
    }
}
