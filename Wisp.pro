#-------------------------------------------------
#
# Project created by QtCreator 2013-01-03T16:48:01
#
#-------------------------------------------------

QT       += core gui opengl xml xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Wisp
TEMPLATE = app


SOURCES += \
    src/main/wisp.cpp \
    src/gui/mainwindow.cpp \
    src/gui/glWidget.cpp \
    src/core/scene.cpp \
    src/core/random.cpp \
    src/core/parser.cpp \
    src/core/paramset.cpp \
    src/core/object.cpp \
    src/core/geometry.cpp \
    src/core/common.cpp \
    src/core/block.cpp \
    src/filters/box.cpp \
    src/shapes/sphere.cpp \
    src/integrators/ao.cpp \
    src/cameras/perspective.cpp \
    src/samplers/independent.cpp \
    src/filters/tent.cpp \
    src/filters/gaussian.cpp \
    src/filters/mitchell.cpp \
    src/shapes/mesh.cpp \
    src/accelerators/kdtree.cpp \
    src/accelerators/grid.cpp \
    src/bsdf/microfacet.cpp \
    src/bsdf/diffuse.cpp \
    src/bsdf/dielectric.cpp \
    src/light/arealight.cpp \
    src/integrators/path.cpp \
    src/core/timer.cpp \
    src/samplers/stratified.cpp \
    src/phase/isotropic.cpp \
    src/integrators/mipath.cpp \
    src/core/shape.cpp \
    src/core/light.cpp \
    src/core/medium.cpp \
    src/medium/homogeneous.cpp \
    src/medium/heterogeneous.cpp \
    src/integrators/volpath.cpp \
    src/core/reflection.cpp \
    src/integrators/volpath_simple.cpp


HEADERS  += \
    src/gui/mainwindow.h \
    src/gui/glWidget.h \
    src/core/transform.h \
    src/core/shape.h \
    src/core/scene.h \
    src/core/sampler.h \
    src/core/renderer.h \
    src/core/random.h \
    src/core/parser.h \
    src/core/paramset.h \
    src/core/object.h \
    src/core/integrator.h \
    src/core/geometry.h \
    src/core/frame.h \
    src/core/filter.h \
    src/core/common.h \
    src/core/camera.h \
    src/core/block.h \
    src/core/aggregate.h \
    src/core/bsdf.h \
    src/core/light.h \
    src/core/distribution1D.h \
    src/core/timer.h \
    src/core/medium.h \
    src/core/phase.h \
    src/core/reflection.h

FORMS    += \
    src/gui/mainwindow.ui

RESOURCES += \
    resource/resources.qrc

INCLUDEPATH +=  \
    src/core \
    external/glm-0.9.3.4 \
    $(BOOST_ROOT)

release: DESTDIR = build/release
debug:   DESTDIR = build/debug
OBJECTS_DIR = $$DESTDIR
RCC_DIR = $$DESTDIR
MOC_DIR = $$DESTDIR
UI_DIR = $$DESTDIR

CONFIG += thread
CONFIG += console
CONFIG -= app_bundle

win32 {
        # You will have to update the following two lines based on where you have installed OpenEXR
        #QMAKE_LIBDIR += openexr/lib/i386
        #INCLUDEPATH += openexr/include

        QMAKE_CXXFLAGS += /O2 /fp:fast /GS- /GL /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS
        QMAKE_LDFLAGS += /LTCG
        #SOURCES += src/support_win32.cpp
        #LIBS += IlmImf.lib Iex.lib IlmThread.lib Imath.lib Half.lib
}

