package com.powervision.video.media.extractor;

/**
 * Created by liwei on 15-7-3.
 */
public class ExtractorFactory {
    public static IDataExtractor createFileDataExtractor(String path) {
        return new FileDataExtractor(path);
    };
    public static IDataExtractor createStreamDataExtractor() {
        return new StreamDataExtractor();
    };
}
