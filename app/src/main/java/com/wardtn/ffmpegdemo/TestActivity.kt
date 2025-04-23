package com.wardtn.ffmpegdemo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.TextView

class TestActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_test)

        (findViewById<View>(R.id.tv_ffmpeg) as TextView).text = stringFromJNI()

    }

    external fun stringFromJNI(): String

    companion object {
        init {
            System.loadLibrary("learn-ffmpeg")
        }
    }

}