import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:users_view
    color: "#2b373d"

    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        samples: 17
        radius:2
    }

    Rectangle {
        color: "#2b373d"
        width:parent.width
        height:35
        anchors.top: parent.top

        layer.enabled: true

        RowLayout {
            id: layout
            anchors.fill: parent
            anchors.topMargin: -(parent.height-40)

            Text {
                Layout.preferredWidth: users_view.user_minimum_Width.user_machine_ip

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "User Machine"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: users_view.user_minimum_Width.user

                //horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "User"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: users_view.user_minimum_Width.tasks_running

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Task Running"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: users_view.user_minimum_Width.jobs_size

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Active Jobs"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: users_view.user_minimum_Width.priority

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Priority"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
        }
    }
}
