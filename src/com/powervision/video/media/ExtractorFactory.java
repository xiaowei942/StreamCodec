package com.powervision.video.media;

/**
 * Created by liwei on 15-7-3.
 */
public class ExtractorFactory {
    public static FileDataExtractor createFileDataExtractor(String path) {
        return new FileDataExtractor(path);
    };
}
