import QtQuick 2.7
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4

Menu {
    property bool multiselected: false
    property int size_selected
    /*
    MenuItem {
        text: "   Items Selected: "+JobsModel.sizeSelected()
        visible: multiselected
    }
    MenuSeparator {visible: multiselected }*/
   MenuItem {
       text: "Open Folder"
       onTriggered:{
           JobsModel.jobOutputFolder()
       }
       visible: !multiselected
   }
   MenuItem {
       text: "Open In RV (not impl.)"
       visible: !multiselected
   }
   MenuItem {
       text: "Show Log"
       onTriggered:{
           popJobLogDialog.text=JobsModel.jobLog()
           popJobLogDialog.show()
       }
       visible: !multiselected
   }
   MenuItem {
       text: "Show Error Blades"
       onTriggered:{
           popJobErrorDialog.text=JobsModel.jobShowErrorBlades()
           popJobErrorDialog.show()
       }
       visible: !multiselected
   }
   MenuSeparator {visible: !multiselected }
   MenuItem {
       text: "Set Priority"
       onTriggered:{
           set_priority_text_input_dialog.text=jobs_ListView.currentItem.v_priority;
           popJobSetPriority.show();
       }
   }
   MenuItem {
       text: "Set Max Running Tasks (not impl.)"
   }
   MenuItem {
       text: "Set Blades Mask"
       onTriggered:{
           set_host_mask_text_input_dialog.text=jobs_ListView.currentItem.v_blades_mask
           popJobSetHostsMask.show();
       }
   }
   MenuSeparator { }
   MenuItem {
       text: "Start"
       onTriggered:{
            JobsModel.startJob()
       }
   }
   MenuItem {
       text: "Pause"
       onTriggered:{
            JobsModel.pauseJob()
       }
   }
   MenuItem {
       text: "Stop"
       onTriggered:{
            JobsModel.stopJob()
       }
   }
   MenuItem {
       text: "Reset Error Blades"
       onTriggered:{
           JobsModel.jobResetErrorHosts()
       }
   }
   MenuItem {
       text: "Restart"
       onTriggered:{
            JobsModel.restartJob()
       }
   }
   MenuItem {
       text: "Restart Error Tasks"
       onTriggered:{
            JobsModel.restartJobErrors()
       }
   }
   MenuItem {
       text: "Skip"
       onTriggered:{
            JobsModel.skipJobs()
       }
       visible: jobs_ListView.currentItem.v_group_size>1 ? true : false
   }
   MenuItem {
       text: jobs_ListView.currentItem.v_group_size>1 ? "Delete Group" : "Delete"
       onTriggered:{
            JobsModel.deleteJobGroup()
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
