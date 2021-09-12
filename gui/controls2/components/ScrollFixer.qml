import QtQuick 2.14

WheelHandler {
    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
    onWheel: {
        if (!target) {
            return;
        }
        var min = -target.topMargin;
        var max = target.contentHeight - target.height + target.topMargin + target.bottomMargin;
        if (target.headerItem) {
            min -= target.headerItem.height;
            max -= target.headerItem.height;
        }
        const newY = target.contentY + (event.hasPixelDelta ? -event.pixelDelta.y : -event.angleDelta.y);
        // Show scrollbar
        target.flick(0, 0.0001);
        target.contentY = Math.max(min, Math.min(newY, max));
        target.returnToBounds();
        event.accepted = true
    }
}
