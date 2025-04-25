package com.wardtn.ffmpegdemo.media

import android.view.Surface

class FFMediaPlayer {
    companion object {
        init {
            System.loadLibrary("learn-ffmpeg")
        }
    }

    fun init(url: String,  surface: Surface) {
            ANativeWindowInit(url, surface)
    }

    external fun stringFromJNI(): String

    external fun ANativeWindowInit(url:String, surface: Surface)
    external fun ANativeWindowPlay()

}