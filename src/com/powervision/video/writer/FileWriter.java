package com.powervision.video.writer;

/**
 * Created by liwei on 15-7-10.
 */
public abstract class FileWriter extends Object {
    Notify mNotify;

    abstract void init();
    public abstract void open();
    public abstract int write(int chn, byte[] data, long size, int frametype, long ts);
    public abstract void close();

    public interface Notify {
        void onNotify(int status);
    }

    public void setNotify(Notify notify) {
        mNotify = notify;
    }
}
