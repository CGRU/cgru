import QtQuick 2.7
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4

Menu {
    MenuItem {
        text: "Show Log"
        onTriggered:{
            popBladeLogDialog.text="Hi"
            popBladeLogDialog.show()
        }
    }
    /*
    MenuItem {
        text: "Show Detail Info"
    }*/
   MenuSeparator { }
   MenuItem {
       text: "Free"
       onTriggered:{
            BladesModel.actFree(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuItem {
       text: "Free Only For Me"
       onTriggered:{
            BladesModel.actNimby(blades_ListView.currentItem.v_blade_id)
       }
       visible:BladesModel.isMyBlade(blades_ListView.currentItem.v_blade_id)
   }
   MenuItem {
       text: "Skip"
       onTriggered:{
           BladesModel.actNIMBY(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuSeparator { }
   MenuItem {
       text: "Eject Tasks"
       onTriggered:{
           BladesModel.actEjectTasks(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuItem {
       text: "Ejest Not My Tasks"
       onTriggered:{
           BladesModel.actEjectNotMyTasks(blades_ListView.currentItem.v_blade_id)
       }
   }

   MenuItem {
       text: "Set Max Slots"
       onTriggered:{
           set_max_slots_input_dialog.text=blades_ListView.currentItem.v_avalible_performance_slots;
           popBladeSetMaxSlots.show();
           //BladesModel.actCapacity(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuItem {
       text: "Set Max Tasks"
       onTriggered:{
           set_max_tasks_input_dialog.text=BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "max_running_task");
           popBladeSetMaxTasks.show();
       }
   }
   MenuSeparator { }
   MenuItem {
       text: "Reboot Blade"
       onTriggered:{
           BladesModel.actReboot(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuItem {
       text: "Wake-On-Lan Wake"
       onTriggered:{
           BladesModel.actWOLWake(blades_ListView.currentItem.v_blade_id)
       }
   }
   MenuItem {
       text: "Wake-On-Lan Sleep"
       onTriggered:{
           BladesModel.actWOLSleep(blades_ListView.currentItem.v_blade_id)
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
