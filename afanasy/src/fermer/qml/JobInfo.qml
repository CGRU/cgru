import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import JobStateEnums 1.0

Item{
    ColumnLayout {
	id:info_title

    anchors.left: parent.left
    anchors.leftMargin: 15
    Layout.alignment:Qt.AlignRight
	opacity:0.5
    property string custom_font: robotoRegular.name
    property int custom_font_size: 13
    
    
	
	Text {
        text: "Job Name"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Description"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Progress"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "User Name"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Job Priority"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Started At"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Elapsed Time"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Software"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "Slots"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
    Text {
        text: "Job ID"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: "Blades Mask"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: "Blades Mask Exclude"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: "Appr. Time To Finish"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: "Dependency"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    /*
    Text {
        text: "Tasks Running"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }*/

    }


    ColumnLayout {
	id:info_value

	anchors.left: info_title.right
	anchors.leftMargin: 20
    opacity:0.8
    property string custom_font: robotoRegular.name
	property int custom_font_size: 13
	Text {
        text: jobs_ListView.currentItem.v_block_name
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: jobs_ListView.currentItem.v_annotation
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text:3==jobs_ListView.currentItem.v_state ? "Done"
                                                  : JobState.RUNNING==jobs_ListView.currentItem.v_state ? jobs_ListView.currentItem.v_progress+"%"
                                                  : JobState.OFFLINE==jobs_ListView.currentItem.v_state ? "Stopped on "+jobs_ListView.currentItem.v_progress+"%"
                                                  : JobState.READY==jobs_ListView.currentItem.v_state ? "Waiting on "+jobs_ListView.currentItem.v_progress+"%"
                                                  : JobState.ERROR==jobs_ListView.currentItem.v_state ? "Errored" : jobs_ListView.currentItem.v_progress+"%"
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
        }
	Text {
        text:  jobs_ListView.currentItem.v_user_name
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"

	    }
	Text {
        text: jobs_ListView.currentItem.v_priority
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: jobs_ListView.currentItem.v_started
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: jobs_ListView.currentItem.v_elapsed
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: jobs_ListView.currentItem.v_software
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: jobs_ListView.currentItem.v_slots
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
    Text {
        text: jobs_ListView.currentItem.v_job_id
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: jobs_ListView.currentItem.v_blades_mask
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: jobs_ListView.currentItem.v_blades_mask_exlude
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: jobs_ListView.currentItem.v_approx_time
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    Text {
        text: jobs_ListView.currentItem.v_depends
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    /*
    Text {
        text: jobs_ListView.currentItem.v_blades_length
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }*/
    }
} 
