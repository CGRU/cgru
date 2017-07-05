import QtQuick 2.7

WorkerScript.onMessage=function timeChange(update_state,serverExist,parent)  {
        if (serverExist){
            if (update_state==0){
                if (jobs_ListView.currentIndex!=-1){
                    parent.JobsModel.updateInteraction(filtered_text)
                    parent.JobsModel.setShowAllJobs(show_all_jobs)
                    console.log("gg "+JobsModel.areJobsDone().length)
                    if (root.side_state=="Tasks"){
                        TasksModel.updateTasksByJobID(jobs_ListView.currentItem.v_job_id)
                    }
                    //--- Title State Info
                    window_root.title=jobs_ListView.currentItem.v_progress+"%"+" - "+jobs_ListView.currentItem.v_block_name+"  ::  AFermer  ::  Render Manager"

                    if (JobState.OFFLINE==jobs_ListView.currentItem.v_state){
                        window_root.title="Stop  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.ERROR==jobs_ListView.currentItem.v_state){
                        window_root.title="Error  ::  AFermer  ::  Render Manager"
                    }
                    if (JobState.READY==jobs_ListView.currentItem.v_state){
                        window_root.title="Waiting  ::  AFermer  ::  Render Manager"
                    }
                }
                else{
                    window_root.title="No Jobs"
                }
            }
            if (update_state==1){
                BladesModel.updateInteraction(filtered_text)
            }
            if (update_state==2){
                JobsModel.updateInteraction(filtered_text)
                BladesModel.updateInteraction(filtered_text)
                JobsModel.updateGroupNodeSize()
                node_view.linesUpdate.call()
                //JobsModel.arangeNodes()
            }
            if (update_state==3){
                UsersModel.updateInteraction(filtered_text)
            }

            if(notify_toggl){
                if (JobsModel.areJobsDone().length){
                    notifyer.custom_text=JobsModel.areJobsDone()
                    notifyer.show()
                }
            }

        }
        else{
            console.log("No Connection ")
        }

        totalJobs=parent.JobsModel.totalJobs();
        doneJobs=JobsModel.doneJobs();
        waitingJobs=JobsModel.readyJobs();
        runningJobs=JobsModel.runningJobs();
        errorJobs=JobsModel.errorJobs();
        WorkerScript.sendMessage('reply')
}
