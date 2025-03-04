DOWNLOAD_FILE(
        https://cdn.jsdelivr.net/gh/shimakaze09/MyData@main/MyBL/LTCTex.h
        ${CMAKE_CURRENT_SOURCE_DIR}/_deps/LTCTex.h
        SHA256 3b6a19b9687e47c7a9ddf6f5f6dc2becb006bcb3739da30891c8631a39452ccb)

ADD_TARGET(
        MODE STATIC
        SOURCE
        "${PROJECT_SOURCE_DIR}/include/MyRTR"
        LIB
        My::MyScene_tool
        My::MyGL_core
        My::MyDP_core
        INC
        include
)
