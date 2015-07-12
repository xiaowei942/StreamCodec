package com.powervision.video.writer;

/**
 * Created by liwei on 15-7-10.
 */
public interface Notify {
    public static int WRITE_SUCCESS = 0;
    public void onNotify(FileWriter writer, int status);
}
