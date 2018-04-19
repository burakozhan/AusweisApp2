import QtQuick 2.7
import QtQuick.Controls 2.0

import Governikus.Global 1.0


Row {
	id: root

	property int availableWidth: 0
	readonly property int contentWidth: root.implicitWidth
	readonly property alias searchText: searchField.displayText

	anchors.top: parent ? parent.top : undefined
	anchors.topMargin: Constants.titlebar_padding
	anchors.right: parent ? parent.right : undefined
	anchors.bottom: parent ? parent.bottom : undefined
	anchors.bottomMargin: Constants.titlebar_padding
	spacing: Constants.titlebar_padding

	GTextField {
		id: searchField
		height: parent.height
		width: root.availableWidth - parent.spacing - iconItem.width - 2 * Constants.titlebar_padding

		visible: false

		onAccepted: {
			iconItem.forceActiveFocus(Qt.MouseFocusReason)
		}

		Behavior on visible {
			PropertyAnimation {
				duration: 150
			}
		}
	}

	Image {
		id: iconItem

		height: parent.height
		width: height
		fillMode: Image.PreserveAspectFit
		source: "qrc:///images/android/search_icon.svg"

		MouseArea {
			anchors.fill: parent
			onClicked: {
				// Storage of the new value is needed because the query
				// of searchField.visible still delivers the old value.
				var searchFieldVisible = !searchField.visible

				if (searchFieldVisible) {
					iconItem.source = "qrc:///images/android/search_cancel.svg"
					searchField.forceActiveFocus(Qt.MouseFocusReason)
					Qt.inputMethod.show()
				} else {
					iconItem.source = "qrc:///images/android/search_icon.svg"
					iconItem.forceActiveFocus(Qt.MouseFocusReason)
					searchField.text = ""
					Qt.inputMethod.hide()
				}

				searchField.visible = searchFieldVisible
			}
		}
	}
}
