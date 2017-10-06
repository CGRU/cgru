import QtQuick 2.7
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.1

Menu {
    MenuItem {
        text: "Show Log"
        onTriggered:{
            popUserLogDialog.text=UsersModel.userLog()
            popUserLogDialog.show()
        }
    }
   MenuSeparator { }
   MenuItem {
       text: "Set User Color"
       onTriggered:{
           color_dialog.open();
       }
   }
   MenuItem {
       text: "Set Priority"
       onTriggered:{
           set_user_priority_text_input_dialog.text=users_ListView.currentItem.v_priority;
           popUserSetPriority.show();
       }
   }
   MenuItem {
       text: "Set Max Running Tasks"
       onTriggered:{
           set_user_max_running_tasks_text_input_dialog.text=UsersModel.getUserInfo("max_running_tasks");
           popUserMaxRunningTasks.show();
       }
   }
   MenuItem {
       text: "Set Blades Mask"
       onTriggered:{
           set_user_blades_mask_text_input_dialog.text=UsersModel.getUserInfo("blade_mask");
           popUserBladesMask.show();
       }
   }
   MenuItem {
       text: "Set Exlude Blades Mask"
       onTriggered:{
           set_user_exlude_blades_mask_text_input_dialog.text=UsersModel.getUserInfo("blade_mask_exclude");
           popUserBladesMaskExlude.show();
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


