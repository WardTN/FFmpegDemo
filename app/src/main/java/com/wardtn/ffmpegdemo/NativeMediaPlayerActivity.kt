package com.wardtn.ffmpegdemo

import android.os.Environment
import android.view.SurfaceHolder
import com.wardtn.ffmpegdemo.base.BaseActivity
import com.wardtn.ffmpegdemo.databinding.ActivityNativeMediaPlayerBinding
import com.wardtn.ffmpegdemo.media.FFMediaPlayer
import java.io.File

class NativeMediaPlayerActivity : BaseActivity<ActivityNativeMediaPlayerBinding>(),
    SurfaceHolder.Callback {

    private var mMediaPlayer: FFMediaPlayer? = null

    private val mVideoPath =
        Environment.getExternalStorageDirectory().absolutePath + "/byteflow/one_piece.mp4"


    override fun initView() {
        super.initView()
        databinding.surfaceView.holder.addCallback(this)
        databinding.btnCallback.setOnClickListener {
            mMediaPlayer?.stringFromJNI()
        }
    }


    override fun surfaceCreated(p0: SurfaceHolder?) {
        mMediaPlayer = FFMediaPlayer()

        if (File(mVideoPath).exists()) {
            mMediaPlayer?.init(mVideoPath, p0!!.surface)
        }
    }

    override fun surfaceChanged(p0: SurfaceHolder?, p1: Int, p2: Int, p3: Int) {
        mMediaPlayer?.ANativeWindowPlay()
    }

    override fun surfaceDestroyed(p0: SurfaceHolder?) {

    }

    override fun getViewId(): Int {
        return R.layout.activity_native_media_player
    }


}