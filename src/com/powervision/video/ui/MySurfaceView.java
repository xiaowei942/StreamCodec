package com.powervision.video.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import com.powervision.video.media.codec.StreamCodec;

import java.nio.Buffer;
import java.nio.ByteBuffer;

/**
 * Created by liwei on 15-7-8.
 */
public class MySurfaceView extends SurfaceView implements
        SurfaceHolder.Callback, Runnable {

    private SurfaceHolder holder;
    private Canvas canvas;
    private Bitmap bitmap;

    public MySurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public MySurfaceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public MySurfaceView(Context context) {
        super(context);
        this.setKeepScreenOn(true);
        this.setFocusable(true);
        holder = this.getHolder();
        holder.addCallback(this);
    }

    public void setHolder(SurfaceHolder hd) {
        this.holder = hd;
    }

    public void setFrameBitmap(Bitmap bmp) {
        bitmap = bmp;
    }

    @Override
    public void run() {
        while (true) {
            if(StreamCodec.framePrepared) {
                draw();
                StreamCodec.framePrepared = false;
            }
            try {
                Thread.sleep(20);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void draw() {
        canvas = holder.lockCanvas();
        Log.i("MySurfaceView", "draw !!!");
        Rect rect = new Rect();
        getDrawingRect(rect);
        canvas.drawBitmap(bitmap, null, rect, null);
        holder.unlockCanvasAndPost(canvas);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
                               int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

    private native int writeJpegFileFromYUV420(String fileName, byte[] yuv420, int quality, int width, int height);
}


