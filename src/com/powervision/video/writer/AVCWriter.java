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
    public static final String MP4_FILE_PATH = "/sdcard/"; //Environment.getExternalStorageDirectory() + "/";

    private int mWidth = 0;
    private int mHeight = 0;
    private int mFrameRate = 26;
    private int nativeWriterObject;

    public AVCWriter(int width, int height) {
        mWidth = width;
        mHeight = height;
        init();
    }

    @Override
    void init() {
        nativeWriterObject = native_init(MP4_FILE_PATH, mWidth, mHeight, mFrameRate);
//        if(nativeWriterObject != 0) {
//            if(mNotify != null) {
//                mNotify.onNotify(STATUS_INIT_OK);
//            }
//        }
    }

    @Override
    public void open() {
        int ret = native_open(nativeWriterObject);
//        if(mNotify != null) {
//            mNotify.onNotify(ret);
//        }
    }

    @Override
    public int write(int chn, byte[] data, long size, int frametype, long ts) {
        int ret = native_write(nativeWriterObject, chn, data, size, frametype, ts);
        return ret;
    }

    @Override
    public void close() {
        native_close(nativeWriterObject);
    }

    private native int native_init(String path, int width, int height, int frameRate);
    private native int native_open(int CObject);
    private native int native_write(int CObject, int chn, byte[] data, long size, int frameType, long ts);
    private native int native_close(int Cobject);
    private native void native_release(int CObject);
}
