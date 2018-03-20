package com.shuai.dataprivacy;

import android.content.Context;
import android.content.res.AssetManager;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.UnsupportedEncodingException;

/**
 * 加密数据工具类
 */

public class PrivacyUtils {
    static {
        try {
            System.loadLibrary("privacy-lib");
        }catch (Throwable e) {
            e.printStackTrace();
        }
    }

    private final String TAG = this.getClass().getSimpleName();
    private volatile static PrivacyUtils instance;
    private String fileName = null;

    private static final int MAX_DATA_SIZE = 256;
    public static final int offset_1 = 0;
    public static final int offset_2 = offset_1 + MAX_DATA_SIZE;
    public static final int offset_3 = offset_2 + MAX_DATA_SIZE;
    public static final int offset_4 = offset_3 + MAX_DATA_SIZE;
    public static final int offset_5 = offset_4 + MAX_DATA_SIZE;
    public static final int offset_6 = offset_5 + MAX_DATA_SIZE;
    public static final int offset_7 = offset_6 + MAX_DATA_SIZE;
    public static final int offset_8 = offset_7 + MAX_DATA_SIZE;
    public static final int offset_9 = offset_8 + MAX_DATA_SIZE;
    public static final int offset_10 = offset_9 + MAX_DATA_SIZE;
    public static final int offset_11 = offset_10 + MAX_DATA_SIZE;
    public static final int offset_12 = offset_11 + MAX_DATA_SIZE;
    public static final int offset_13 = offset_12 + MAX_DATA_SIZE;
    public static final int offset_14 = offset_13 + MAX_DATA_SIZE;
    public static final int offset_15 = offset_14 + MAX_DATA_SIZE;



    private PrivacyUtils() {
    }

    public static PrivacyUtils getInstance() {
        if (instance == null) {
            synchronized(PrivacyUtils.class) {
                if (instance == null) {
                    instance = new PrivacyUtils();
                }
            }
        }

        return instance;
    }
    /**
     *  初始化函数
     *  @param context  上下文环境
     */
    public synchronized void init(Context context) {
        if (context == null) {
            return;
        }

        fileName = context.getFilesDir().getAbsolutePath()
                + File.separator + "hengchang.bmp";

        if(TextUtils.isEmpty(fileName)) {
            Log.e(TAG,"PrivacyUtils init fileName is null");
            return;
        }

        Log.v(TAG,"=fileName is = " + fileName);

        if (!isFileExist(fileName)) {
            initPrivacy(context.getResources().getAssets(), fileName);
        }
    }

    /**
     *  存储数据
     *  @param  data 存储的数据，大小不要超过250个byte
     *  @param offset 存储偏移量
     */
    public synchronized boolean setData(String data, int offset){
        try {
            if (TextUtils.isEmpty(fileName)) {
                return false;
            }

            byte[] bytes = data.getBytes("UTF-8");
            if (bytes == null) {
                return false;
            }

            if (bytes.length > MAX_DATA_SIZE - 6) {
                Log.e(TAG,"data size limit!");
                return false;
            }


            return savePrivacy(fileName, bytes, offset);
        }catch (UnsupportedEncodingException e) {
            return false;
        }
    }

    /**
     *  读取数据
     *  @param offset  偏移量
     */
    public synchronized String getData(int offset) {
        try {
            if (TextUtils.isEmpty(fileName)) {
                return null;
            }

            byte[] data = getPrivacy(fileName, offset);
            return new String(data,"UTF-8");
        } catch (UnsupportedEncodingException e) {
            return null;
        }
    }


    public native static boolean savePrivacy(String filename, byte[] data, int offset);
    public native static byte[] getPrivacy(String filename,int offset);
    public native static void initPrivacy(AssetManager am, String filename);
    public native static boolean isFileExist(String filename);
}
