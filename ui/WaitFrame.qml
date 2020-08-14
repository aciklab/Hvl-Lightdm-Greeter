import QtQuick 2.0

Rectangle {
    id: waitRectangle
    visible: true
    width: 500
    height: 400
    property string stext: qsTr("Authenticating")
    transformOrigin: Item.Center
    border.width: 4
    Text {
        id: waittext
        width: 250
        height: 50
        text: qsTr("Authenticating")
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    onStextChanged: waittext.text = stext
}
