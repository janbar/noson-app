import QtQuick 2.2

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
            return 24.0 * scaleFactor * fontScaleFactor;
        if (s === "small")
            return 28.0 * scaleFactor * fontScaleFactor;
        if (s === "medium")
            return 32.0 * scaleFactor * fontScaleFactor;
        if (s === "large")
            return 36.0 * scaleFactor * fontScaleFactor;
        if (s === "x-large")
            return 40.0 * scaleFactor * fontScaleFactor;

        return 0.0;
    }

}
