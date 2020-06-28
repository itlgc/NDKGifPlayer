#include <jni.h>
#include <string>
#include <malloc.h>
#include "giflib/gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>

#define  LOG_TAG    "TAG"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

//两种GIF格式都支持的方法需要定义的宏
#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  trans_index(ext) ((ext)->Bytes[3])
#define  transparency(ext) ((ext)->Bytes[0] & 1)
#define  delay(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))


typedef struct GifBean{
    int  current_frame; //当前帧  第几帧
    int  total_frame;   //总帧数
    int *dealys;    //延迟时间数组 长度不确定（有多少帧就有多少个延迟数据）
    // 注意：每次的延迟时间不一定是一样的
} GifBean;


void drawFrame(GifFileType *pType, GifBean *pBean, AndroidBitmapInfo info, void *pVoid);
int drawFrame2(GifFileType* gif,GifBean * gifBean, AndroidBitmapInfo  info, void* pixels,  bool
force_dispose_1);

//这里只针对89a（主流）的GIF图片格式的处理，   后续要兼容87a（过时）格式的
extern "C"
JNIEXPORT jlong JNICALL
Java_com_it_ndkgifplayer_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {
    // TODO: implement loadPath()
    //获取一个指向UTF-8格式字符串内容的指针。
    const char *path =env->GetStringUTFChars(path_,0);

    //===============系统库提供的方法============
    //打开gif文件
    int error;
    GifFileType *gifFileType = DGifOpenFileName(path, &error);
    //初始化默认内存  这是打开文件要做的操作之一
    DGifSlurp(gifFileType);

    //初始化GifBean   相当于new GifBean
    GifBean *gifBean = (GifBean *)malloc(sizeof(GifBean));
    memset(gifBean, 0, sizeof(GifBean)); //清空内存地址操作;
    gifFileType->UserData = gifBean;

    //初始化延时数组
    gifBean->dealys = (int *) malloc(sizeof(int) * gifFileType->ImageCount);
    memset(gifBean->dealys, 0, sizeof(int) * gifFileType->ImageCount);


//    gifFileType->UserData = gifBean;
    //参数赋值
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;


    //延迟时间数组的填充
    ExtensionBlock* ext;
    //遍历每一帧
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        SavedImage frame =  gifFileType->SavedImages[i];

        //遍历扩展块 默认是4个
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function==GRAPHICS_EXT_FUNC_CODE){
                //拿到图形扩展块
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }

        if (ext){
            //Delay Time - 单位1/100秒，如果值不为1，表示暂停规定的时间后再继续往下处理数据流
            //这里延迟时间2个字节来表示一个int（即需要拼成一个int类型）    Bytes[1]是高8位  Bytes[2]是低8位
            int frame_delay = 10 * (ext->Bytes[1] | (ext->Bytes[2] << 8)); //ms
            LOGE("每帧的延时时间： %d   ",frame_delay);
            gifBean->dealys[i] = frame_delay;
        }
    }
    LOGE("gif 长度大小(即帧数)：  %d  ",gifFileType->ImageCount);

    //释放一个指向UTF-8格式字符串内容的指针
    env->ReleaseStringUTFChars(path_, path);
    return (jlong)gifFileType;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_it_ndkgifplayer_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    // TODO: implement getWidth()
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    return gifFileType->SWidth;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_it_ndkgifplayer_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    // TODO: implement getHeight()
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    return gifFileType->SHeight;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_it_ndkgifplayer_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                                    jobject bitmap) {
    // TODO: implement updateFrame()
    //代表GIF图片信息
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    GifBean *gifBean =  (GifBean *)gifFileType->UserData;

    //代表Bitmap信息
    AndroidBitmapInfo info;
    //&info  取址 传递进去是作为入参 出参对象，最终将获取的数据赋值到info中  区别java传递对象的方式
    AndroidBitmap_getInfo(env, bitmap, &info);

    //锁住这个bitmap，防止其他线程对它操作
    void *pixels; //pixels 二维数组  这里代表一个空的gif--bitmap
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

#if 1  //1:仅仅支持89a  0:两者都支持
    //渲染 仅仅考虑89a格式的GIF
    drawFrame(gifFileType, gifBean, info, pixels);
#else
    //89a 87a两者都支持
    drawFrame2(gifFileType, gifBean, info, pixels, false);
#endif

    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
        LOGE("重新过来  %d  ",gifBean->current_frame);
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->dealys[gifBean->current_frame];//返回下一帧延迟时间

}

//只支持89a
//渲染每一帧图片  （GIF文件的大小往往会比真正内容区域更大，所以要考虑到坐标偏移）
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    //当前帧
    SavedImage frame = gifFileType->SavedImages[gifBean->current_frame];
    //整幅图片的首地址
    int* px = (int *)pixels;

    GifImageDesc frameInfo = frame.ImageDesc;
    GifByteType  gifByteType;//压缩数据
    //存放rgb数据的字典    也可以理解为一个解压缩工具，将想要的rgb数据解压出来
    ColorMapObject *colorMapObject = frameInfo.ColorMap;

    //Bitmap 往下偏移
    px = (int *) ((char*)px + info.stride * frameInfo.Top);

    int *line; //每一行的首地址
    int  pointPixel;
    //先遍历行，再遍历列  我们是要遍历内容区域的部分，为了只去绘制有内容的区域
    for (int y = frameInfo.Top; y <frameInfo.Top+frameInfo.Height ; ++y) {
        line=px;//待绘制的行 首地址
        for (int x = frameInfo.Left; x <frameInfo.Left + frameInfo.Width ; ++x) {
            //拿到每一个坐标的位置，就是索引，拿到索引才能拿到数据
            pointPixel=  (y-frameInfo.Top)*frameInfo.Width+(x-frameInfo.Left);

            //从帧中获取出每个坐标（即像素点）的压缩数据(底层是通过lzw算法压缩的)     RasterBits：表示当前帧的像素数组
            gifByteType = frame.RasterBits[pointPixel];
            //解压获取rgb
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];

            //逐行进行渲染
            line[x]=argb(255,gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        //渲染完上一行后 指到下一行
        px = (int *) ((char*)px + info.stride);//info.stride 一行的像素量
    }


}

//支持89a同时兼容87a
int drawFrame2(GifFileType* gif,GifBean * gifBean, AndroidBitmapInfo  info, void* pixels,  bool
force_dispose_1) {
    GifColorType *bg;

    GifColorType *color;

    SavedImage * frame;

    ExtensionBlock * ext = 0;

    GifImageDesc * frameInfo;

    ColorMapObject * colorMap;

    int *line;

    int width, height,x,y,j,loc,n,inc,p;

    void* px;



    width = gif->SWidth;

    height = gif->SHeight;



    frame = &(gif->SavedImages[gifBean->current_frame]);

    frameInfo = &(frame->ImageDesc);

    if (frameInfo->ColorMap) {

        colorMap = frameInfo->ColorMap;

    } else {

        colorMap = gif->SColorMap;

    }



    bg = &colorMap->Colors[gif->SBackGroundColor];



    for (j=0; j<frame->ExtensionBlockCount; j++) {

        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {

            ext = &(frame->ExtensionBlocks[j]);

            break;

        }

    }
    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && gifBean->current_frame > 0) {
        gifBean->current_frame=gifBean->current_frame-1,
                drawFrame2(gif,gifBean, info, pixels,  true);
    }

    else if (ext && dispose(ext) == 2 && bg) {

        for (y=0; y<height; y++) {

            line = (int*) px;

            for (x=0; x<width; x++) {

                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);

            }

            px = (int *) ((char*)px + info.stride);

        }

    } else if (ext && dispose(ext) == 3 && gifBean->current_frame > 1) {
        gifBean->current_frame=gifBean->current_frame-2,
                drawFrame2(gif,gifBean, info, pixels,  true);

    }
    px = pixels;
    if (frameInfo->Interlace) {

        n = 0;

        inc = 8;

        p = 0;

        px = (int *) ((char*)px + info.stride * frameInfo->Top);

        for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {

            for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {

                loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }



                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char*)px + info.stride * inc);

            n += inc;

            if (n >= frameInfo->Height) {

                n = 0;

                switch(p) {

                    case 0:

                        px = (int *) ((char *)pixels + info.stride * (4 + frameInfo->Top));

                        inc = 8;

                        p++;

                        break;

                    case 1:

                        px = (int *) ((char *)pixels + info.stride * (2 + frameInfo->Top));

                        inc = 4;

                        p++;

                        break;

                    case 2:

                        px = (int *) ((char *)pixels + info.stride * (1 + frameInfo->Top));

                        inc = 2;

                        p++;

                }

            }

        }

    }

    else {

        px = (int *) ((char*)px + info.stride * frameInfo->Top);

        for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {

            line = (int*) px;

            for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {

                loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }

                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char*)px + info.stride);

        }
    }

    return delay(ext);
}