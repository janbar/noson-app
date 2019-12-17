import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    property real scaleFactor: 1.0
    property real fontScaleFactor: 1.0
    property real gridUnit: 16.0 * scaleFactor

    function dp(p) {
        return scaleFactor * p;
    }

    function gu(u) {
        return gridUnit * u;
    }

    function fs(s) {
        if (s === "x-small")
            return 10.0 * scaleFactor * fontScaleFactor;
        if (s === "small")
            return 12.0 * scaleFactor * fontScaleFactor;
        if (s === "medium")
            return 14.0 * scaleFactor * fontScaleFactor;
        if (s === "large")
            return 18.0 * scaleFactor * fontScaleFactor;
        if (s === "x-large")
            return 22.0 * scaleFactor * fontScaleFactor;

        return 0.0;
    }

    function fx(s) {
        if (s === "x-small")
            return Theme.fontSizeExtraSmall * scaleFactor * fontScaleFactor;
        if (s === "small")
            return Theme.fontSizeSmall * scaleFactor * fontScaleFactor;
        if (s === "medium")
            return Theme.fontSizeMedium * scaleFactor * fontScaleFactor;
        if (s === "large")
            return Theme.fontSizeLarge * scaleFactor * fontScaleFactor;
        if (s === "x-large")
            return Theme.fontSizeHuge * scaleFactor * fontScaleFactor;

        return 0.0;
    }

}
