import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:jobs_view
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
        width:parent.width
        height:35
        anchors.top: parent.top

        layer.enabled: true

        RowLayout {
            id: layout
            anchors.fill: parent
            anchors.topMargin: -(parent.height-40)
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.progress

                horizontalAlignment: Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Progress"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.elapsed

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Elapsed"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.user

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "User"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.job_name

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Job Name"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        JobsModel.sortingChangeState()
                        JobsModel.multiSorting(4)
                    }
                }
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.approximate_time

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "ETA"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.slots

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Slots"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.priority

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Priority"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        JobsModel.sortingChangeState()
                        JobsModel.multiSorting(6)
                    }
                }
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.started

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Created At"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        JobsModel.sortingChangeState()
                        JobsModel.multiSorting(7)
                    }
                }
            }
            Text {
                Layout.preferredWidth: jobs_view.job_minimum_Width.software

                horizontalAlignment : Text.AlignHCenter
                layer.enabled: true
                color: "white"
                opacity: 0.45
                text: "Software"
                font.letterSpacing:1.2
                font { family: robotoRegular.name; pixelSize: 14}
            }
        }
    }
}
