package com.it.ndkgifplayer;

import android.graphics.Bitmap;

/**
 * Created by lgc on 2020-03-25.
 */
public class GifHandler {
    private long gifAddr; //信使的作用   本质就是GifFileType
    static {
        System.loadLibrary("native-lib");
    }
    public GifHandler(String path) {
        this.gifAddr = loadPath(path);
    }

    public int getWidth() {
        return getWidth(gifAddr);
    }
    public int getHeight() {
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap) {
        return updateFrame(gifAddr,bitmap);
    }


    private native long loadPath(String path);
    public native int getWidth(long ndkGif);
    public native int getHeight(long ndkGif);
    //隔一段事件 调用一次 返回下一帧的延迟时间
    public native int updateFrame(long ndkGif, Bitmap bitmap);

}
