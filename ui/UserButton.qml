import QtQuick 2.0

Rectangle {
    id:userRect
    width: 220
    height: 50
    property string username: ""
    color: "#21436A"
    MouseArea {

        anchors.fill: parent

    }

    Text {
        id: usernamestr
        x: 8
        y: 8
        width: 204
        height: 34
        font.pixelSize: 24
        color:"white"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
    }


    onUsernameChanged: usernamestr.text = username


}
