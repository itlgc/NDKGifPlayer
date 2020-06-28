package com.it.ndkgifplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

import androidx.appcompat.app.AppCompatActivity;

/**
 * 加载GIF大文件
 */
public class MainActivity extends AppCompatActivity {

    Bitmap bitmap;
    ImageView imageView;
    GifHandler gifHandler;

    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            // 需要刷新性能下一帧
            int mNextFrame=gifHandler.updateFrame(bitmap);
            handler.sendEmptyMessageDelayed(1,mNextFrame);
            imageView.setImageBitmap(bitmap);
            imageView.setScaleType(ImageView.ScaleType.FIT_XY);
        }
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.image);
        // 运行时权限申请
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            String perms[] = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms, 1000);
            }
        }
        CopyUtils.copyAssetsAndWrite(getApplicationContext(), "demo.gif", false);
    }


    public void ndkLoadGif(View view) {
        //gif图片大约有14M
        File file=new File(Environment.getExternalStorageDirectory(),"demo.gif");
        gifHandler = new GifHandler(file.getAbsolutePath());

        //得到gif   width  height  生成Bitmap
        int width=gifHandler.getWidth();
        int height=gifHandler.getHeight();
        bitmap= Bitmap.createBitmap(width,height, Bitmap.Config.ARGB_8888);

        //下一帧的刷新事件
        int nextFrame=gifHandler.updateFrame(bitmap);
        handler.sendEmptyMessageDelayed(1,nextFrame);
    }
}
