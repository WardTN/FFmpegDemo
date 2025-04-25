package com.wardtn.ffmpegdemo.base

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding


abstract class BaseActivity<DB : ViewDataBinding> : AppCompatActivity() {

    lateinit var databinding: DB

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val rootView = View.inflate(this, getViewId(), null)
        setContentView(rootView)
        databinding = DataBindingUtil.bind(rootView)!!
        databinding.lifecycleOwner = this

        initView()
        initClick()
    }

    abstract fun getViewId(): Int

    open fun initView() {}

    open fun initClick() {}




}