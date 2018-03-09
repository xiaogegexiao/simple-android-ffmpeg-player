package com.nirovision.nostalgiasample

import android.graphics.Bitmap
import android.graphics.Paint
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.nirovision.nostalgia.JniBridge
import java.nio.ByteBuffer


class MainActivity : AppCompatActivity() {
    companion object {
        val TAG = MainActivity::class.simpleName
    }

    lateinit var mSurfaceView : SurfaceView
    lateinit var mSurfaceHolder: SurfaceHolder

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mSurfaceView = findViewById(R.id.surface_view)
        mSurfaceHolder = mSurfaceView.holder
        Thread(
                Runnable { JniBridge.getInstance().decodeStream("http://www.sample-videos.com/video/mp4/720/big_buck_bunny_720p_1mb.mp4", 10, object : JniBridge.VideoCallback {
                    override fun onVideo(height: Int, width: Int, bitmap: ByteArray?) {
                        Log.d(TAG, "byte array received")
                        bitmap?.let {
                            var canvas = mSurfaceHolder.lockCanvas()
                            canvas.drawColor(resources.getColor(android.R.color.black))
                            val bmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
                            bmp.copyPixelsFromBuffer(ByteBuffer.wrap(it))
                            canvas.drawBitmap(bmp, 0f, 0f, Paint())
                            mSurfaceHolder.unlockCanvasAndPost(canvas)
                        }
                    }
                }) }
        ).start()
    }
}
