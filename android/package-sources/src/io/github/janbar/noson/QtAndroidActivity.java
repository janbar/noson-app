package io.github.janbar.noson;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.window.OnBackInvokedDispatcher;
import android.content.ComponentCallbacks2;

import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import org.qtproject.qt.android.QtActivityBase;

public class QtAndroidActivity extends QtActivityBase
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // handle back pressed from android-33
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            getOnBackInvokedDispatcher()
                    .registerOnBackInvokedCallback(
                            OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                            () -> {
                                Log.d("noson", "Handle BACK PRESSED");
                                super.onKeyDown(
                                        KeyEvent.KEYCODE_PAGE_UP,
                                        new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_PAGE_UP));
                            });
        }

        // handle immersive mode from android-35
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.VANILLA_ICE_CREAM) {
            WindowInsetsControllerCompat windowInsetsController =
                    WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
            // configure the behavior of the hidden system bars.
            windowInsetsController.setSystemBarsBehavior(
                    WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            // hide system bars
            windowInsetsController.hide(WindowInsetsCompat.Type.systemBars());
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        // handle back pressed up to android-32
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
            Log.d("noson", "Handle BACK PRESSED LEGACY");
            return super.onKeyDown(
                    KeyEvent.KEYCODE_PAGE_UP,
                    new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_PAGE_UP));
        }
        return super.onKeyDown(keyCode, event);
    }
}

