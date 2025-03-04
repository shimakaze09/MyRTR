GET_TARGET_NAME(CORE "${PROJECT_SOURCE_DIR}/src/core")
ADD_TARGET(
        TEST
        MODE EXE
        LIB ${CORE} glfw
)