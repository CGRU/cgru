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
    anchors.topMargin: 130
	anchors.left: parent.left
	anchors.leftMargin: 15
	Layout.alignment:Qt.AlignRight
	opacity:0.5
    property string custom_font: robotoRegular.name
    //font.weight: Font.Normal
	property int custom_font_size: 13
    
    
	
	Text {
        text: "User Name"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
    Text {
        text: "User ID"
        anchors.right: parent.right
        font.pixelSize: parent.custom_font_size
        font.family:parent.custom_font
        font.letterSpacing:1
        color: "white"
        }
	Text {
        text: "Priority"
		anchors.right: parent.right
		font.pixelSize: parent.custom_font_size
		font.family:parent.custom_font
		font.letterSpacing:1
		color: "white"
	    }
	Text {
        text: "User Machine"
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
        anchors.topMargin: 130
        anchors.left: info_title.right
        anchors.leftMargin: 20
        opacity:0.8
        property string custom_font: robotoRegular.name
        property int custom_font_size: 13
        Text {
            text: users_ListView.currentItem.v_user_name
            anchors.left: parent.left
            font.pixelSize: parent.custom_font_size
            font.family:parent.custom_font
            font.letterSpacing:1
            color: "white"
            }
        Text {
            text: users_ListView.currentItem.v_user_id
            anchors.left: parent.left
            font.pixelSize: parent.custom_font_size
            font.family:parent.custom_font
            font.letterSpacing:1
            color: "white"
            }
        Text {
            text: users_ListView.currentItem.v_priority
            anchors.left: parent.left
            font.pixelSize: parent.custom_font_size
            font.family:parent.custom_font
            font.letterSpacing:1
            color: "white"
            }
        Text {
            text: users_ListView.currentItem.v_user_machine
            anchors.left: parent.left
            font.pixelSize: parent.custom_font_size
            font.family:parent.custom_font
            font.letterSpacing:1
            color: "white"

            }
    }
} 
