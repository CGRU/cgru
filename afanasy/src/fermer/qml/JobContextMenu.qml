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
       text: "Open Ouput Folder"
       onTriggered:{
           console.log("Trying to open: "+JobsModel.jobGetOutputFolder())
           if  (!JobsModel.jobGetOutputFolder().length){
               popJobFolderIsNotExistDialog.text="Can`t To Find Folder"
               popJobFolderIsNotExistDialog.show()
           }
           else{
               JobsModel.jobOpenOutputFolder()
           }
       }
       visible: !multiselected
   }
   /*
   MenuItem {
       text: "Open In RV (not impl.)"
       visible: !multiselected
   }*/
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
   MenuItem {
       text: "Reset Error Blades"
       onTriggered:{
           JobsModel.jobResetErrorHosts()
       }
   }
   MenuSeparator {}
   /*
   MenuItem {
       text: "Set Max Running Tasks (not impl.)"
   }*/
   Menu {
       title: "Start,Stop,Restart ..."
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
           text: "Restart"
           onTriggered:{
                JobsModel.restartJob()
           }
       }
       MenuItem {
           text: "Restart Error Tasks"
           onTriggered:{
                JobsModel.jobRestarErrorsTasks()
           }
       }
       MenuItem {
           text: "Restart Running Tasks"
           onTriggered:{
                JobsModel.jobRestartRunningTasks()
           }
       }
       MenuItem {
           text: "Restart Skipped Task"
           onTriggered:{
                JobsModel.jobRestartSkippedTasks()
           }
       }
       MenuItem {
           text: "Restart Done Task"
           onTriggered:{
                JobsModel.jobRestartDoneTasks()
           }
       }
   }
   Menu {
       title: "Set Parameter"

       MenuItem {
           text: "Set Blades Mask"
           onTriggered:{
               set_host_mask_text_input_dialog.text=jobs_ListView.currentItem.v_blades_mask
               popJobSetHostsMask.show();
           }
       }
       MenuItem {
           text: "Set Blades Mask Exclude"
           onTriggered:{
               set_host_mask_exclude_text_input_dialog.text=jobs_ListView.currentItem.v_blades_mask_exlude
               popJobSetHostsMaskExclude.show();
           }
       }
       MenuSeparator {}
       MenuItem {
           text: "Set Error Avoid Blade"
           onTriggered:{
               set_error_avoid_blade_text_input_dialog.text=JobsModel.getJobInfo("errors_avoid_blades");
               popJobSetErrorAvoidBlade.show();
           }
       }
       MenuItem {
           text: "Set Task Error Retries"
           onTriggered:{
               set_error_retries_text_input_dialog.text=JobsModel.getJobInfo("errors_retries");
               popJobSetErrorRetries.show();
           }
       }
       MenuItem {
           text: "Set Error Forgive Time"
           onTriggered:{
               set_error_forgive_time_text_input_dialog.text=JobsModel.getJobInfo("errors_forgive_time");
               popJobSetErrorForgiveTime.show();
           }
       }
       MenuItem {
           text: "Set Tasks Max Run Time"
           onTriggered:{
               set_max_run_time_text_input_dialog.text=JobsModel.getJobInfo("tasks_max_runtime");
               popJobSetMaxRunTime.show();
           }
       }
       MenuSeparator {}
       MenuItem {
           text: "Set Priority"
           onTriggered:{
               set_priority_text_input_dialog.text=jobs_ListView.currentItem.v_priority;
               popJobSetPriority.show();
           }
       }
       MenuItem {
           text: "Set Max Running Tasks"
           onTriggered:{
               set_max_running_task_text_input_dialog.text=JobsModel.getJobInfo("max_run_tasks");
               popJobSetMaxRunningTask.show();
           }
       }
       MenuItem {
           text: "Set Max Running Task Per Blade"
           onTriggered:{
               set_max_running_task_per_blade_text_input_dialog.text=JobsModel.getJobInfo("max_run_tasks_perblade");
               popJobSetMaxRunningTaskPerBlade.show();
           }
       }
       MenuItem {
           text: "Set Depend Mask"
           onTriggered:{
               set_depend_mask_text_input_dialog.text=JobsModel.getJobInfo("depend_mask");
               popJobSetDependMask.show();
           }
       }
       MenuItem {
           text: "Set Slots"
           onTriggered:{
               set_slots_text_input_dialog.text=jobs_ListView.currentItem.v_slots;
               popJobSetSlots.show();
           }
       }
       MenuItem {
           text: "Set Life Time"
           onTriggered:{
               set_life_time_input_dialog.text=JobsModel.getJobInfo("time_life");
               popJobSetLifeTime.show();
           }
       }
       MenuItem {
           text: "Set Wait Time"
           onTriggered:{
               set_wait_time_input_dialog.text=JobsModel.getJobInfo("time_wait");
               popJobSetWaitTime.show();
           }
       }
       MenuItem {
           text: "Set Annotation"
           onTriggered:{
               set_annotation_input_dialog.text=jobs_ListView.currentItem.v_annotation;
               popJobSetAnnotation.show();
           }
       }
       MenuItem {
           text: "Set OS Needed"
           onTriggered:{
               set_os_input_dialog.text=JobsModel.getJobInfo("need_os");
               popJobSetOS.show();
           }
       }
       MenuItem {
           text: "Set Need Memory"
           onTriggered:{
               set_memory_input_dialog.text=JobsModel.getJobInfo("need_memory");
               popJobSetMemory.show();
           }
       }
       MenuItem {
           text: "Set Need HDD"
           onTriggered:{
               set_hdd_input_dialog.text=JobsModel.getJobInfo("meed_hdd");
               popJobSetHDD.show();
           }
       }
       MenuItem {
           text: "Set Post Command"
           onTriggered:{
               set_post_command_input_dialog.text=JobsModel.getJobInfo("command");
               popJobSetPostCommand.show();
           }
       }
   }
   MenuSeparator { }
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
            JobsModel.passUpdate()
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
