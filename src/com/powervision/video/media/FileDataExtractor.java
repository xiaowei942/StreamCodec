package com.powervision.video.media;

import android.os.Environment;
import android.util.Log;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by liwei on 15-7-3.
 */
public class FileDataExtractor extends DataExtractor implements Runnable{
    //For debug
    private static final boolean DEBUG = false;
    private static final String TAG = "FileDataExtractor";

    //Video source file location.
    private static final String DEFAULT_DATA_FILE_PATH = Environment.getExternalStorageDirectory() + "/test.h264";

    //Customized file path
    private String filePath;

    //Video input dependent stream
    private FileInputStream fis;

    //Length of the file
    private int fileLength = -1;

    //Maximium stream length
    private static final int MAX_SISE = 20*1024*1024;

    //Save the file stream data
    private byte[] bytes = new byte[MAX_SISE];
    private byte[] Sps = new byte[128];
    private byte[] Pps = new byte[128];

    private int status;

    private static List <Integer> naluList = new ArrayList<Integer>();

    //Constructor
    public FileDataExtractor() {

    }

    //Constructor
    public FileDataExtractor(String path) {
        filePath = path;
    }

    @Override
    byte[] getFrame() {
        return new byte[0];
    }

    @Override
    boolean openDataExtractor() {
        if(filePath != null) {
            return openFile(filePath);
        } else {
            return openFile(DEFAULT_DATA_FILE_PATH);
        }
    }

    @Override
    void closeDataExtractor() {
        closeFile(fis);
    }

    @Override
    void start() {
        this.run();
    }

    @Override
    int getStatus() {
        return status;
    }

    //Open raw video file
    private boolean openFile(String file) {
        try {
            fis = new FileInputStream(file);
            return true;
        } catch (FileNotFoundException e) {
            Log.e(TAG, "Cannot open input file stream.");
            e.printStackTrace();
            return false;
        }
    }

    //Close
    private boolean closeFile(FileInputStream fis) {
        if(fis != null) {
            try {
                fis.close();
            } catch (IOException e) {
                Log.e(TAG, "Cannot close input file stream.");
                e.printStackTrace();
                return false;
            }
        }
        return true;
    }

    //Get data from video file stream in bytes.
    private int getData(FileInputStream fis) {
        int count = 0;
        try {
            count = fis.read(bytes);
            return count;
        } catch (IOException e) {
            Log.e(TAG, "Cannot get data from file.");
            e.printStackTrace();
            return -1;
        }
    }

    private int getNaluList(byte[] src) {
        for(int i=0; i<src.length; i++) {
            if( (src[i] == 0x0) && (src[i+1] == 0x0)) {
                if(src[i+2] == 0x01) {

                }else if((src[i+2] == 0x0) && (src[i+3] == 0x01)) {
                    naluList.add(i);
                    i+=3;
                }
            }
        }
        naluList.add(fileLength);

        if(DEBUG) {
            for(int count=0; count<naluList.size(); count++) {
                Log.i(TAG, "Count: " + count + "  Value: " + naluList.get(count).toString());
            }
        }

        return naluList.size();
    }

    private boolean hasSpsPps() {
        boolean hasSps = false;
        boolean hasPps = false;

        int index = naluList.get(0);
        for(int i=0; i<5; i++) {
            int temp = bytes[index+i];
            if((temp & 0x0f) == 0x07) {
                hasSps = true;
                break;
            }
        }

        if(hasSps) {
            index = naluList.get(0);
            for(int j=0; j<5; j++) {
                int temp = bytes[index+j];
                if((temp & 0x0f) == 0x08) {
                    hasPps = true;
                    break;
                }
            }
        }

        return false;
    }

    @Override
    public void run() {
        fileLength = getData(fis);
        if( fileLength == -1) {
            status = DATA_EXTRACTOR_STATUS_FAILED;
            return;
        };

        if(!hasSpsPps()) {
            status = DATA_EXTRACTOR_STATUS_FAILED;
            return;
        }

        
        status = DATA_EXTRACTOR_STATUS_OK;
    }
}
