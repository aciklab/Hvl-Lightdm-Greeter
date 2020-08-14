import QtQuick 2.9

import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Window {


    id: win2
    visible: true
    property bool multiMonitor: (Screen.desktopAvailableWidth > Screen.width) ? true : false

}
