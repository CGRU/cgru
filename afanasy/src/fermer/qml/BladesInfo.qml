import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import BladeStateEnums 1.0

Item{
    id:blades_info
    ColumnLayout {
	id:info_title
	anchors.top: parent.top
    anchors.topMargin: 50
	anchors.left: parent.left
	anchors.leftMargin: 15
	Layout.alignment:Qt.AlignRight
	opacity:0.5
    property string custom_font: robotoRegular.name
    //font.weight: Font.Normal
	property int custom_font_size: 13
    
    
	
	Text {
		text: "Machine Name"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "State"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "System"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Blades Group"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "CPU Usage"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Memory Usage"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Network Usage"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Performance Slots"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Avalible Performance Slots"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Working Time"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "IP:Port Address"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Custom Properties"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Blade Resources"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Custom Data"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "CPU Numbers"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "CPU Mhz"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Memory Total Mb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Memory Free Mb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Memory Cached Mb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Memory Buffer Mb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Swap Total Mb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "HDD Total Gb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "HDD Free Gb"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "HDD Read kb/sec"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "HDD Write kb/sec"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "HDD Busy"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Net Receive kb/sec"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: "Net Send kb/sec"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
    Text {
        text: "Jobs In"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }

    }


    ColumnLayout {
	id:info_value
    anchors.top: parent.top
    anchors.topMargin: 50
	anchors.left: info_title.right
	anchors.leftMargin: 20
	opacity:0.8
    property string custom_font: robotoRegular.name
	property int custom_font_size: 13
	Text {
        text: blades_ListView.currentItem.v_name
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_state==BladeState.BUSY ? "Working"
                                              : blades_ListView.currentItem.v_state==BladeState.OFFLINE ? "Offline"
                                              : blades_ListView.currentItem.v_state==BladeState.READY ? "Idle"
                                              : blades_ListView.currentItem.v_state==BladeState.NIMBY ? "Free For Owner"
                                              : blades_ListView.currentItem.v_state==BladeState.BIG_NIMBY ? "Skip"
                                              : blades_ListView.currentItem.v_state==BladeState.DIRTY ? "DIRTY"
                                              : "else"
        anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_base_os
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
        }
	Text {
        text:  blades_ListView.currentItem.v_blades_group
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"

	    }
	Text {
        text: blades_ListView.currentItem.v_loaded_cpu +" %"
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_loaded_mem +" %"
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_loaded_net+" Kb/s"
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "performance_slots")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "avalible_performance_slots")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_working_time
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_ip_address
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "properties")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "resources")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "data")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "cpu_num")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "cpu_mhz")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "mem_total_mb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "mem_free_mb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "mem_cached_mb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "mem_buffers_mb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "swap_used_mb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "hdd_total_gb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "hdd_free_gb")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "hdd_rd_kbsec")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "hdd_wr_kbsec")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: blades_ListView.currentItem.v_hdd_busy
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "net_recv_kbsec")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
		text: BladesModel.getBladeInfoByIndex(blades_ListView.currentIndex, "net_send_kbsec")
		anchors.left: parent.left
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
    Text {
        text: blades_ListView.currentItem.v_jobsIn
        anchors.left: parent.left
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
    }
} 
