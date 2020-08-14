
import QtQuick 2.0



Rectangle {

    id:buttonrec
    property bool clickOccurs : false

    MouseArea {
        id: buttonrecArea
        anchors.fill: parent
        onPressed: {
            buttonrec.color = "#87cefa"

        }
        onReleased: {
            buttonrec.color ="#21436A"

        }

        onClicked: {

            clickOccurs = true
        }

    }
    color: "#21436A"
    width: 75
    height:40
    border.width: 2
    border.color: "#34A8FB"


}

