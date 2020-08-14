import QtQuick 2.9


Rectangle {
    id: loginRectangle
    visible: true
    width: 500
    height: 400
    transformOrigin: Item.Center
    border.width: 4
    property bool ishidden: false
    property bool endofAuth: false
    property string receivedMessage: ""
    property string suserlist: ""


    StringInput {
        id: userInput
        x: 119
        y: 166

        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        setfocus: true

        onTabclickedChanged: {
            if(tabclicked == true){
                setfocus = false
                passwordInput.setfocus = true
                tabclicked = false
            }
        }

    }
    StringInput {
        id: passwordInput
        x: 119
        y: 216
        anchors.horizontalCenter: parent.horizontalCenter
        noecho: true
        setfocus: false
        onTabclickedChanged: {
            if(tabclicked == true){
                setfocus = false
                userInput.setfocus = true
                tabclicked = false
            }else{

            }
        }

    }

    CustomButton {
        id:loginButton
        x: 325
        y: 257
        Text {
            id: loginText
            text:  qsTr("Login")
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
        }

        onClickOccursChanged: {

            if(clickOccurs == true){
                authenticate()
                clickOccurs = false
            }
        }

    }


    CustomButton {
        id:cancelButton
        x: 100
        y: 257
        Text {
            id: cancelText
            text:  qsTr("Cancel")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 9
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
        }

        onClickOccursChanged: {
            if(clickOccurs == true){
                userInput.stext = ""
                passwordInput.stext = ""
                clickOccurs = false
            }
        }

    }

    CustomButton {
        id:passwordChangeButton
        x: 244
        y: 257
        Text {
            id: changePassswordText
            width: 60
            height: 30
            text:  qsTr("Change\nPassword")
            horizontalAlignment: Text.AlignHCenter
            leftPadding: 2
            lineHeight: 1
            textFormat: Text.AutoText
            wrapMode: Text.WordWrap
            font.pointSize: 9
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
        }

    }

    Text {
        id: messagetext
        x: 100
        y: 308
        width: 300
        height: 77
        wrapMode: Text.WordWrap
        font.bold: true
        font.pixelSize: 13
    }

    Keys.onEnterPressed: {
        authenticate()
    }
    Keys.onReturnPressed: {
        authenticate()
    }


    Keys.onEscapePressed: {
        userInput.stext = ""
        passwordInput.stext = ""
    }

    onEndofAuthChanged: {

        resetLogin()
        endofAuth = false
    }

    onReceivedMessageChanged:  {
        console.log("---"+ receivedMessage)

        if(receivedMessage.length > 1)
            messagetext.text = receivedMessage

        receivedMessage = ""
    }


    onSuserlistChanged: {
        messagetext.text = suserlist
    }

    function authenticate(){
        if(passwordInput.stext.length > 0 && userInput.stext.length > 0){
            loginRectangle.visible = false

            ishidden = true
            win1.username = userInput.stext.trim()
            win1.password = passwordInput.stext.trim()
            //start login
        }

    }

    function resetLogin(){
        passwordInput.stext = ""
        loginRectangle.visible = true
        ishidden = false
    }

}
