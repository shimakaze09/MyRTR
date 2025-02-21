GLOBAL_GROUP_SOURCES(RST SOURCES PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${PROJECT_SOURCE_DIR}/include/MyRTR"
)

IF (${MY_MYGL_GLAD})
    ADD_DEFINITIONS(-DMY_MYGL_GLAD)
ENDIF ()

ADD_TARGET_GDR(
        MODE "LIB"
        SOURCES ${SOURCES}
        INTERFACE_INC ON
        PUBLIC_GLIBS
        "My::MyScene_core"
        "My::MyScene_tool"
        "My::MyGL_core"
        "My::MyDP_core"
)
