package com.wardtn.ffmpegdemo

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.wardtn.ffmpegdemo.base.BaseActivity
import com.wardtn.ffmpegdemo.databinding.ActivityTestBinding
import com.wardtn.ffmpegdemo.media.FFMediaPlayer
import com.wardtn.ffmpegdemo.util.CommonUtils

class TestActivity : BaseActivity<ActivityTestBinding>() {
    private val REQUEST_PERMISSIONS = arrayOf(
        Manifest.permission.CAMERA,
        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.WRITE_EXTERNAL_STORAGE)


    override fun initView() {
        super.initView()

        databinding.btnCopyAssert.setOnClickListener {
            CommonUtils.copyAssetsDirToSDCard(this, "byteflow", "/sdcard")
        }
        databinding.btnNative?.setOnClickListener {
            startActivity(Intent(this, NativeMediaPlayerActivity::class.java))
        }
    }

    override fun onResume() {
        super.onResume()
        if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            ActivityCompat.requestPermissions(this,
                REQUEST_PERMISSIONS,
                1)
        }
    }


    protected fun hasPermissionsGranted(permissions: Array<String>): Boolean {
        for (permission in permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission!!)
                != PackageManager.PERMISSION_GRANTED
            ) {
                return false
            }
        }
        return true
    }

    override fun getViewId(): Int {
        return R.layout.activity_test
    }


}