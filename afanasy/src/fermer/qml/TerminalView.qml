import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4


Rectangle{
    id:terminal_view
    color: "#2b373d"
    width: 1024
    border.color: "#fe9400"
    border.width: 1
    property bool init: false

    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        samples: 17
        radius:2
    }
    focus: true
    ScrollView {
        width: parent.width-6
        height: parent.height-6
        anchors.left: parent.left
        anchors.leftMargin: 4
        highlightOnFocus:true
        style: ScrollViewStyle {
            transientScrollBars: root.transient_toggl
            scrollBarBackground: Item {
                implicitWidth: 12
                implicitHeight: 26
            }
            handle: Rectangle {
                implicitWidth: 12
                implicitHeight: 26
                color: "white"
                opacity: 0.4
            }
            decrementControl: Rectangle {
                implicitWidth: 12
                implicitHeight: 2
                color: "white"
                opacity: 0.3
            }
            incrementControl: Rectangle {
                implicitWidth: 12
                implicitHeight: 1
                color: "white"
                opacity: 0.3
            }
        }
        TextEdit {
            id: text_input
            //width: parent.width-6
            //height: parent.height-6
            anchors.centerIn: parent
            activeFocusOnPress:true
            selectByMouse: true
            color: "white"
            selectionColor: text_input.focus ? Qt.rgba(0.88,0.51,0,0.67) : Qt.rgba(0.88,0.51,0,0.27)//"#b26800"
            font.letterSpacing: 1.7
            font.weight: Font.Normal
            font { family: robotoRegular.name; pixelSize: 14}
            wrapMode: TextEdit.WordWrap
            text:settings.python_edit

            Keys.onPressed: {
                //console.log("selHand"+event.modifiers)
                //console.log("tab "+Qt.Key_Tab)
                if ((event.key == Qt.Key_Tab)&&(event.modifiers & Qt.ShiftModifier)){
                    console.log("selHand")
                }
            }
            Shortcut {
                sequence: "Ctrl+E"
                onActivated: {
                    var start_line=text_input.getText(0,text_input.cursorPosition).lastIndexOf("\n")
                    var start_space=text_input.getText(start_line+2,text_input.cursorPosition).lastIndexOf(" ")
                    var start_pos=start_line
                    if (-1!=start_space) start_pos=start_line+start_space+2
                    python_output=General.executeSelected("print '\\n'.join(dir("+text_input.getText(start_pos+1,text_input.cursorPosition)+"))")
                }
            }

            onTextChanged: {
                /*
                var dot=text_input.getText(text_input.cursorPosition-1,text_input.cursorPosition)
                if (dot=="."){
                    console.log("text . "+text_input.cursorRectangle)
                    //autocomplition.popup()
                    //popInfoDialog.show()
                }*/
                settings.python_edit=text
                if (!terminal_view.init){
                    terminal_view.init=true
                    General.terminalText(text_input.textDocument)
                }
            }

            Keys.onReturnPressed: {
                if (event.modifiers & Qt.ControlModifier){
                    python_output=General.executeSelected(text_input.selectedText)
                }
                else{
                    if (text_input.text[text_input.cursorPosition-1]==":"){
                        text_input.insert(text_input.cursorPosition,"\n    ");
                    }
                    else{
                        var start_line=text_input.getText(0,text_input.cursorPosition).lastIndexOf("\n")
                        var any_char=text_input.getText(start_line,text_input.cursorPosition).match(/\S\w*/)
                        var spaces=""
                        if (any_char){
                            var start_char=text_input.getText(start_line,text_input.cursorPosition).indexOf(any_char)
                            var spaces=Array(start_char).join(" ")
                        }
                        text_input.insert(text_input.cursorPosition,"\n"+spaces);
                    }
                }
            }

            Keys.onTabPressed:{
                //console.log("event.modifiers . "+event.modifiers)
                if (event.modifiers & Qt.ShiftModifier){
                    console.log("event.modifiers . "+event.modifiers)
                }
                else{
                    text_input.insert(text_input.cursorPosition,"    ");
                    var selected_text=text_input.selectedText
                    if (selected_text!=""){
                        var found=selected_text.indexOf("\n")+1
                        var found_text=selected_text.substr(found)
                        var glob_pos=text_input.cursorPosition+(selected_text.length-found_text.length)
                        text_input.insert(glob_pos,"    ");
                        var iter=1
                        while (found!=0) {
                            found=found_text.indexOf("\n")+1
                            if (found>0){
                                found_text=found_text.substr(found)
                                glob_pos=text_input.cursorPosition+(selected_text.length-found_text.length)+(iter*4)
                                text_input.insert(glob_pos,"    ");
                                iter+=1
                            }
                        }
                    }
                }
            }
        }
    }
    Menu {
       id:autocomplition
       onAboutToShow: {
            text_input.focus=true;
       }

       MenuItem {
           text: "getAll"
           onTriggered:{
                text_input.insert(text_input.cursorPosition,"getAll()")
           }
       }
       MenuItem {
           text: "godMode"
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

    Window {
        id: popInfoDialog
        title: "Task Info"
        width: 100
        height: 200
        flags: Qt.ToolTip
        //modality: Qt.ApplicationModal
        color: "#394c54"
        x:text_input.cursorRectangle.x
        y:text_input.cursorRectangle.y+root.y
        property string text: "None"
        property string returnValue: ""
        //onActiveChanged: {console.log("popInfoDialog");text_input.linkActivated()}
        //activeFocusItem :text_input

        TextEdit{
            anchors.centerIn: parent
        }
    }
}
