import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:metrics_view
    color: "#2b373d"
    width: 1024



    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        samples: 17
        radius:2
    }

    ColumnLayout {
        id: layout
        anchors.fill: metrics_view
        anchors.topMargin: 50
        spacing:10
        MetricJobAtWeek {
            Layout.alignment: Qt.AlignTop
            width: 1200
            height: 250
        }
    }
}
