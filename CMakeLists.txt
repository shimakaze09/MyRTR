# Set the minimum required version of CMake
CMAKE_MINIMUM_REQUIRED(VERSION 3.14 FATAL_ERROR)

# Set the project
PROJECT(MyRTR VERSION 0.0.1 LANGUAGES CXX)
MESSAGE(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")

# Add 'd' postfix for debug builds
SET(CMAKE_DEBUG_POSTFIX "d")

# Set C++ version
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

IF (MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
ENDIF ()

# Set the output directories
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")

# Turn on the Visual Studio folders
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Add CMake modules
INCLUDE(FETCHCONTENT)

MESSAGE(STATUS "Looking for: MyCMake v0.4.0")
FIND_PACKAGE(MyCMake 0.4.0 QUIET)
IF (${MyCMake_FOUND})
    MESSAGE(STATUS "MyCMake v${MyCMake_VERSION} found")
ELSE ()
    SET(ADDRESS "https://github.com/shimakaze09/MyCMake.git")
    MESSAGE(STATUS "MyCMake v0.4.0 not found\n"
            "fetch from ${ADDRESS} with tag v0.4.0")
    FETCHCONTENT_DECLARE(
            MyCMake
            GIT_REPOSITORY ${ADDRESS}
            GIT_TAG "v0.4.0"
    )
    message(STATUS "Building MyCMake v0.4.0")
    FETCHCONTENT_MAKEAVAILABLE(MyCMake)
    MESSAGE(STATUS "MyCMake v0.4.0 built")
ENDIF ()

# Set default build path prefix
INIT_INSTALL_PREFIX()

MESSAGE(STATUS "----------")
IF (EXISTS "${PROJECT_SOURCE_DIR}/include/_deps/glad/glad.h"
        AND EXISTS "${PROJECT_SOURCE_DIR}/include/_deps/KHR/khrplatform.h"
        AND EXISTS "${PROJECT_SOURCE_DIR}/src/_deps/glad/glad.c")
    MESSAGE(STATUS "found glad")
ELSE ()
    MESSAGE(FATAL_ERROR "not found glad\n"
            "read setup.md for more detail")
ENDIF ()

MESSAGE(STATUS "----------")
FIND_PACKAGE(glfw3 REQUIRED)
IF (TARGET glfw)
    MESSAGE(STATUS "glfw3_FOUND: ${glfw3_FOUND}")
    MESSAGE(STATUS "glfw3_DIR: ${glfw3_DIR}")
ENDIF ()

MESSAGE(STATUS "----------")
IF (EXISTS "${PROJECT_SOURCE_DIR}/include/_deps/stb_image.h")
    MESSAGE(STATUS "found stb_image.h")
ELSE ()
    MESSAGE(FATAL_ERROR "not found stb_image.h in include/_deps/\n"
            "read README.md for more detail")
ENDIF ()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/_deps/)

SET(MY_MYGL_GLAD TRUE)

ADD_DEP(MyGL 0.1.1)
ADD_DEP(MyScene 0.2.0)
ADD_DEP(MyDP 0.3.0)

ADD_SUB_DIRS_REC(src)

EXPORT_TARGETS(DIRECTORIES "include")