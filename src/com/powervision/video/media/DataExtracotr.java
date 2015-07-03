package com.powervision.video.media;

/**
 * Created by liwei on 15-7-3.
 */
abstract class DataExtractor {
    public static final int DATA_EXTRACTOR_STATUS_OK = 1;
    public static final int DATA_EXTRACTOR_STATUS_FAILED = 0;

    //Get one frame video data
    abstract byte[] getFrame();

    abstract boolean openDataExtractor();
    abstract void closeDataExtractor();
    abstract void start();
    abstract int getStatus();
}
