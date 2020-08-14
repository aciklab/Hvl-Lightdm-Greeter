import QtQuick 2.9


Rectangle{
    id: textrect
    color: "black"
    TextInput{
        id: textInput
        cursorVisible: true

        //topPadding: 7
        font.weight: Font.Medium
        horizontalAlignment: Text.AlignLeft
        clip: true
        opacity: 1
        font.capitalization: Font.MixedCase
        font.pixelSize: 16
        topPadding :8
        leftPadding :4
        selectByMouse: true
        onTextEdited: textrect.stext = textInput.text
        Keys.onTabPressed: {
            tabclicked = true
        }
        color: "white"
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        font.bold: true
        Rectangle {
            id: showPassword
            visible: false
            width:30
            height: 27
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 4
            border.width: 2
            border.color: "white"
            color: "#21436A"
            MouseArea {
                anchors.fill: parent

                onPressed: {
                    showPassword.color = "#87cefa"
                    textInput.echoMode = TextInput.Normal;
                    textInput.enabled = false

                }
                onReleased: {
                    showPassword.color ="#21436A"
                    textInput.echoMode = TextInput.Password;
                    textInput.enabled = true
                }



            }
        }


    }
    width: 300
    height: 35
    border.width: 2
    border.color: "#34A8FB"
    property string stext: ""

    onStextChanged: {
        textInput.text = stext
    }

    property bool noecho :false

    onNoechoChanged: {
        textInput.echoMode = TextInput.Password;
        showPassword.visible = true
    }

    property bool setfocus : false
    onSetfocusChanged: {

        if(setfocus == true)
            textInput.focus = true
        else
            textInput.focus = false

        tabclicked = false
    }

    property bool tabclicked : false
}
