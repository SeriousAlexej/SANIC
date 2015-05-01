TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += debug

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    entities/box.cpp \
    entities/decoration.cpp \
    entities/player.cpp \
    entities/pointlight.cpp \
    basic.cpp \
    camera.cpp \
    dirtools.cpp \
    entity.cpp \
    GLDebugDrawer.cpp \
    input_handler.cpp \
    light.cpp \
    mesh.cpp \
    modelinstance.cpp \
    quaternion_utils.cpp \
    shader.cpp \
    solidbody.cpp \
    texture.cpp \
    world.cpp \
    world_graphics.cpp \
    world_physics.cpp \
    dialogs/tinyfiledialogs.c

HEADERS += \
    dialogs/tinyfiledialogs.h \
    entities/box.h \
    entities/decoration.h \
    entities/player.h \
    entities/pointlight.h \
    basic.h \
    camera.h \
    default_model.h \
    dirtools.h \
    entity.h \
    entityevent.h \
    GLDebugDrawer.h \
    input_handler.h \
    light.h \
    mesh.h \
    modelinstance.h \
    quaternion_utils.h \
    shader.h \
    solidbody.h \
    texture.h \
    world.h \
    world_graphics.h \
    world_physics.h \
    properties.h \
    props_draw.h

LIBS += -L$$PWD/bullet/src/BulletDynamics/ -lBulletDynamics
LIBS += -L$$PWD/bullet/src/BulletCollision/ -lBulletCollision
LIBS += -L$$PWD/bullet/src/LinearMath/ -lLinearMath

LIBS += -lGL -lGLEW -lboost_system -lboost_filesystem -lsfml-system -lsfml-window -lsfml-graphics -lGLU -lAntTweakBar

INCLUDEPATH += $$PWD/bullet/src/
