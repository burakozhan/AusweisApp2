import QtQuick 2.5

import Governikus.Global 1.0

Item {
	property alias source: tabImage.source
	property alias text: tabText.text
	signal clicked

	Item {
		id: tabImageItem
		height: parent.height
		width: Constants.menubar_width
		anchors.left: parent.left

		Image {
			id: tabImage
			height: Utils.dp(35)
			fillMode: Image.PreserveAspectFit
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
		}
	}

	Text {
		id: tabText
		anchors.left: tabImageItem.right
		anchors.leftMargin: Utils.dp(10)
		anchors.verticalCenter: parent.verticalCenter
		font.pixelSize: Constants.small_font_size
		renderType: Text.NativeRendering
		color: Constants.secondary_text
	}

	MouseArea {
		anchors.fill: parent
		onClicked: parent.clicked()
	}
}
