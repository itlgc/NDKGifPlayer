# 学习目标

单独处理GIF图播放的意义

GIF处理的两种方式以及对应的开源库

GIF图片的格式的编码原理

利用系统源码手写GIF图播放库（主流89a的GIF图格式，兼容87a格式）

# 学习总结

Java处理的方式并不适用于处理大文件和数量较多情况下的GIF文件处理。对应这种情况还是要采取native层的处理方式。

一般大公司都会针对GIF播放提供专门的库，并不会用Java方式去处理。考虑的GIF文件大小和数量。对应中小型项目的看需求去选择。

## gif处理方式

### Java方式：

​    gihub: Glide 、GifView （Glide在加载大文件的时候容易卡死，GifView在recycleView使用会卡成翔）

​    其中解析GIF文件方式又分为两种：通过Movie类去解析gif文件和 自己编码（github上一些开源项目中使用的是某大神写的一些gif解析器）去解析gif文件。

Java的处理方式对内存消耗比较大的，如果加载比较大的GIF文件，以及如果运行在低配置的手机上时，就会明显卡顿，影响体验。 

（所有Java解析gif的库都会有两个类GifDecoder.java和GifFrame.java 正是这两个类的方式导致内存消耗比较大）

### NDK方式：

​    github: android-gif-drawable    采用C的方式写的，基于native方式来开发的。这个开源库性能比较高，对内存开销极小。

也有缺点：首先内部C文件比较多，编译出来也有一两百kb大小，基于编译期处理，对于打断点调试就很蛋疼了，它会在编译时对运行的方法包裹一层额外的方法。

选择GIF包的时候一定要考虑兼容和包的大小的问题

## GIF图片格式编码原理 

参考网络文章（这里是针对89a格式的GIF图） https://blog.csdn.net/wzy198852/article/details/17266507 

每个帧都有数据区域和 扩展块区域（扩展块又分为图形控制扩展块、图形文本扩展块、应用程序扩展块、注释扩展块），每一帧的延迟时间在其中的图形控制扩展块中

![img](file:///private/var/folders/8t/lx2d5qn94dd8yrxqk7mg0q3r0000gn/T/WizNote/eb02f357-7e84-4241-b7c2-1f8d5747c8e1/index_files/57097690.png)

## 利用系统提供的库来二次开发GIF图播放 

参考源码路径：external/giflib/

 \#播放GIF暂时只需要这两个c库

​    dgif_lib.c

​    gifalloc.c