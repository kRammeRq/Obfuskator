QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#INCLUDEPATH += /usr/include/clang/ /usr/include/llvm/
INCLUDEPATH += $$system(llvm-config --includedir)
#INCLUDEPATH += /usr/lib/llvm-18/include/clang   # Указал правильный путь
INCLUDEPATH += /usr/lib/llvm-18/include
INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5/QtCore
#INCLUDEPATH += /usr/lib/llvm-18/include/clang
#LIBS += -L$$system(llvm-config --libdir) -lclang -lLLVM -lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangAST -lclangLex -lclangBasic
LIBS += -L$$system(llvm-config --libdir) \
    -lclang -lLLVM -lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver \
    -lclangSerialization -lclangParse -lclangSema -lclangAnalysis \
    -lclangAST -lclangASTMatchers -lclangLex -lclangBasic
LIBS += -L/usr/lib/llvm-18/lib -lclang -lLLVM -lclang-cpp
LIBS += -lQt5Core
#LIBS += -lclang -lllvm -lclangRISCV
#LIBS += -L/path/to/llvm/lib
#LIBS += -L/usr/lib/llvm-12/lib
SOURCES += \
    FileDialogChoose.cpp \
    MyASTclass.cpp \
    main.cpp \
    mainwindow.cpp \
    settingswindow.cpp

HEADERS += \
    FileDialogChoose.h \
    MyASTclass.h \
    mainwindow.h \
    settingswindow.h

FORMS += \
    mainwindow.ui \
    settingswindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
