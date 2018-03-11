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
    var bmp: Bitmap? = null
    var byteBuffer: ByteBuffer? = null
    var isStreaming = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mSurfaceView = findViewById(R.id.surface_view)
        mSurfaceHolder = mSurfaceView.holder
    }

    fun stopStreaming() {
        synchronized(isStreaming) {
            if (!isStreaming) {
                Log.d(TAG, "already stopped")
            } else {
                isStreaming = false
            }
        }
    }

    fun startStreaming() {
        synchronized(isStreaming) {
            isStreaming = true
            Thread(
                    Runnable { JniBridge.getInstance().decodeStream("rtsp://admin:@10.20.80.101:80/videoMain", object : JniBridge.VideoCallback {
                        override fun keepStreaming(): Boolean {
                            var tempIsStreaming = false
                            synchronized(isStreaming){
                                tempIsStreaming = isStreaming
                            }
                            return tempIsStreaming
                        }

                        override fun onVideo(height: Int, width: Int, bitmap: ByteArray?) {
                            bitmap?.let {byteArray ->
                                var canvas = mSurfaceHolder.lockCanvas()
                                canvas.drawColor(resources.getColor(android.R.color.black))
                                if (bmp == null) {
                                    bmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
                                }
                                byteBuffer = ByteBuffer.wrap(byteArray)
                                bmp?.copyPixelsFromBuffer(byteBuffer)
                                canvas.drawBitmap(bmp, 0f, 0f, Paint())
                                byteBuffer = null
                                mSurfaceHolder.unlockCanvasAndPost(canvas)
                            }
                        }
                    }) }
            ).start()
        }
    }

    override fun onResume() {
        super.onResume()
        startStreaming()
    }

    override fun onPause() {
        super.onPause()
        stopStreaming()
    }
}
