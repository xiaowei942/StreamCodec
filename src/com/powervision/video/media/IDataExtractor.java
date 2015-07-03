package com.powervision.video.media;

/**
 * Created by liwei on 15-7-3.
 */
public interface IDataExtractor {
    //Get one frame video data
    public byte[] getFrame();
    public boolean openDataExtractor();
    public void closeDataExtractor();
    public void start();
    public int getStatus();
}
