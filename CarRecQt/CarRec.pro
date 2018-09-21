TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    carrec.cpp \
    svm.cpp \
    cJSON.c \
    Util.c \
    Encode.c

INCLUDEPATH += /usr/local/include \
/usr/local/include/opencv \
/usr/local/include/opencv2

LIBS += /usr/local/lib/libopencv_highgui.so \
/usr/local/lib/libopencv_core.so \
/usr/local/lib/libopencv_imgproc.so \
/usr/local/lib/libopencv_video.so \
/usr/local/lib/libopencv_calib3d.so \
/usr/local/lib/libopencv_ml.so \
/usr/local/lib/libopencv_objdetect.so

HEADERS += \
    carrec.h \
    datastruct.h \
    svm.h \
    cJSON.h \
    Util.h \
    Encode.h


LIBS +=-lpthread \
    -luuid
