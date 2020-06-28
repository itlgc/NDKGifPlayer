# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#添加一个库，根据native-lib.cpp源文件编译一个native-lib的动态库
add_library( # Sets the name of the library.
        native-lib
        # Sets the library as a shared library.
        SHARED
        #播放GIF暂时只需要这两个c库
        giflib/dgif_lib.c
        giflib/gifalloc.c
        # Provides a relative path to your source file(s).
        native-lib.cpp)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        log
        jnigraphics
        )