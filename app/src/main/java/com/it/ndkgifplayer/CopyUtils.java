package com.it.ndkgifplayer;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

/**
 * Created by lgc on 2020-02-10.
 *
 * @description
 */
public class CopyUtils {

    /**
     * 将Assets目录下的fileName文件拷贝至手机内部存储中
     *
     * @param context
     * @param fileName
     */
    private static String copyAssetsAndWrite(Context context, String fileName) {

        try {
            File outFile = new File(Environment.getExternalStorageDirectory(), fileName);

            InputStream is = context.getAssets().open(fileName);
            FileOutputStream fos = new FileOutputStream(outFile);
            byte[] buffer = new byte[is.available()];
            int byteCount;
            while ((byteCount = is.read(buffer)) != -1) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            is.close();
            fos.close();
            Log.d("TAG:", "文件拷贝成功" + outFile.getAbsolutePath());
            return outFile.getAbsolutePath();

        }catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static String copyAssetsAndWrite(Context context, String fileName,boolean isUpdate) {

        try {
            File outFile = new File(Environment.getExternalStorageDirectory(), fileName);

            if (isUpdate || !outFile.exists()) {
                InputStream is = context.getAssets().open(fileName);
                FileOutputStream fos = new FileOutputStream(outFile);
                byte[] buffer = new byte[is.available()];
                int byteCount;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
                Log.d("TAG:", "文件拷贝成功" + outFile.getAbsolutePath());
                return outFile.getAbsolutePath();
            }else {
                Log.d("TAG:", "文件已存在" + outFile.getAbsolutePath());
                return outFile.getAbsolutePath();
            }


        }catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

}
