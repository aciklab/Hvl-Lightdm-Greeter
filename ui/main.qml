import QtQuick 2.9

import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import org.hvl.greeter 1.0

Window {


    id: win1
    visible: true
    property bool multiMonitor: (Screen.desktopAvailableWidth > Screen.width) ? true : false
    property string username: ""
    property string password: ""


    Greeter {
        id:greeter
        onLogincompletedChanged: {
            if(logincompleted == true){

                logincompleted = false
                loginFrame.receivedMessage = message
                loginFrame.endofAuth = true



            }


        }
    }

    LoginFrame {
        id: loginFrame
        visible: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        suserlist: greeter.userlist
        Component.onCompleted: listButtons(greeter.userlist)

        onIshiddenChanged: {
            if(ishidden){
                waitFrame.stext = qsTr("Authenticating")
                waitFrame.visible = true
            }else{
                waitFrame.visible = false
            }
        }


    }

    onUsernameChanged: {
        greeter.username = username;
    }

    onPasswordChanged: {
        greeter.password = password;
    }

    WaitFrame {
        id: waitFrame
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        stext: qsTr("Waiting for user and services")

    }


    UserButton {
        id: userbutton0
        visible: false
        x:10
        y:10
    }

    UserButton {
        id: userbutton1
        visible: false
        x:10
        y: 1 * height + 11

    }

    UserButton {
        id: userbutton2
        visible: false
        x:10
        y: 2 * height + 12

    }

    UserButton {
        id: userbutton3
        visible: false
        x:10
        y: 3 * height + 13

    }


    UserButton {
        id: userbutton4
        visible: false
        x:10
        y: 4 * height + 14

    }

    UserButton {
        id: userbutton5
        visible: false
        x:10
        y: 5 * height + 15

    }


    function listButtons(userListStr){
        var usercount;
        var occur = {};
        var i
        var res = userListStr.split(",", 3);

        for(i = 0; i< res.length; i++){

            switch (i){
            case 0:
                userbutton0.visible = true
                userbutton0.username = res[0]
                userbutton0.color = "#87cefa"

                break;

            case 1:
                userbutton1.visible = true
                userbutton1.username = res[1]


                break;


            case 2:
                userbutton2.visible = true
                userbutton2.username = res[2]


                break;

            }

        }

    }

}



/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1;anchors_height:100;anchors_width:500}
}
 ##^##*/
