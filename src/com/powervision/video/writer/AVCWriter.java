package com.powervision.video.writer;

import android.os.Environment;

/**
 * Created by liwei on 15-7-10.
 */
public class AVCWriter extends FileWriter {
    public static final int STATUS_INIT_OK = 0;
    public static final int STATUS_OPEN_OK = 1;
    public static final int STATUS_WRITE_OK = 2;
    public static final int STATUS_CLOSE_OK = 3;
    public static final String MP4_FILE_PATH = "/sdcard/test.mp4"; //Environment.getExternalStorageDirectory() + "/";

    private int mWidth = 0;
    private int mHeight = 0;
    private int mFrameRate = 26;
    private int nativeWriterObject;

    public AVCWriter(int width, int height, int fps) {
        mWidth = width;
        mHeight = height;
        mFrameRate = fps;
        init();
    }

    @Override
    void init() {
        nativeWriterObject = native_writerInit(mWidth, mHeight);
        native_setMp4FileName(nativeWriterObject, MP4_FILE_PATH);
        native_setMp4Fps(nativeWriterObject, mFrameRate);
//        if(nativeWriterObject != 0) {
//            if(mNotify != null) {
//                mNotify.onNotify(STATUS_INIT_OK);
//            }
//        }
    }

    @Override
    public void open() {
        native_startRecord(nativeWriterObject);
//        if(mNotify != null) {
//            mNotify.onNotify(ret);
//        }
    }

    @Override
    public void writeFrame(byte[] data, long size, long ts) {
        native_writeFrame(nativeWriterObject, data, size, ts);
        //return ret;
    }

    @Override
    public void close() {
        native_stopRecord(nativeWriterObject);
    }

    public native int native_writerInit(int width, int height);
    public native void native_setMp4FileName(int obj, String fileName);
    public native void native_setMp4Fps(int obj, int fps);
    public native void native_startRecord(int obj);
    public native void native_stopRecord(int obj);
    public native void native_writeFrame(int obj, byte[] data, long size, long ts);
}
