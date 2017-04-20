import QtQuick 2.7
import QtQuick.Layouts 1.1

Item {
    height:20

    Rectangle {
        id: farm_usage_bar
        color: "white"
        anchors.fill: parent
        opacity: 0.3

        layer.enabled: true
    }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 2
        spacing: 20
        opacity: 0.7
        Item{
            height: parent.height
            width: 120
            Row{
                spacing: 5
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    opacity: 1
                    text: "Total Jobs:"
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name;pixelSize: 14}
                    font.weight: Font.Medium

                }
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    opacity: 1
                    text: totalJobs
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Light
                }
            }

        }
        Item{
            height: parent.height
            width: 120
            Row{
                spacing: 5
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: "Jobs Running:"
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Medium
                }
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: runningJobs
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Light
                }
            }
        }
        Item{
            height: parent.height
            width: 120
            Row{
                spacing: 5
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: "Jobs Waiting:"
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Medium
                }
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: waitingJobs
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Light
                }
            }
        }
        Item{
            height: parent.height
            width: 120
            Row{
                spacing: 5
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: "Jobs Done:"
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Medium
                }
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: doneJobs
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Light
                }
            }
        }
        Item{
            height: parent.height
            width: 120
            Row{
                spacing: 5
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: "Jobs Error:"
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Medium
                }
                Text {
                    horizontalAlignment : Text.AlignHCenter
                    layer.enabled: true
                    color: "white"
                    text: errorJobs
                    font.letterSpacing:1.2
                    font { family: robotoRegular.name; pixelSize: 14}
                    font.weight: Font.Light
                }
            }
        }
    }
}
