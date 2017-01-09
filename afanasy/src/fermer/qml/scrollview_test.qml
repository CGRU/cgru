import QtQuick 2.0
import QtQuick.Controls 1.0

Item{
    width: 180; height: 80
    ScrollView {
            anchors.fill: parent
            ListView {
                

                model: ListModel {
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }

            }
            delegate: Text {
                text: name + ": " + number
            }
        }
    }
}
