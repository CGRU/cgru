import QtQuick 2.7
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.1

Menu {
   MenuItem {
       text: "Set User Color"
       onTriggered:{
           color_dialog.open();
       }
   }
   MenuItem {
       text: "Set Priority (not impl.)"
       onTriggered:{
           //BladesModel.actReboot(blades_ListView.currentItem.v_blade_id)
       }
   }
   style: MenuStyle {
       frame: Rectangle {
           color: "#394c54"
           border.color: "#394c54"
       }
       itemDelegate {
           background: Rectangle {
               color: "#394c54"
           }
           label: Text {
               color: "white"
               opacity: styleData.selected ? 0.9 :0.7
               text: styleData.text
               font.family:robotoRegular.name
               font.pointSize: 10
           }
       }
   }
}


