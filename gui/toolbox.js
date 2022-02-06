.pragma library

// signal helpers

function connectOnce(sig, slot) {
    var f = function() { slot.apply(this, arguments); sig.disconnect(f); };
    sig.connect(f);
}

function connectWhileFalse(sig, slot) {
    var f = function() { slot.apply(this, arguments); if (arguments[0] === true) sig.disconnect(f); };
    sig.connect(f);
}

function connectWhileTrue(sig, slot) {
    var f = function() { slot.apply(this, arguments); if (arguments[0] !== true) sig.disconnect(f); };
    sig.connect(f);
}
