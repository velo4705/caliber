# FindWrapOpenGL.cmake — iOS workaround
# Provides OpenGL::GL using the iOS OpenGLES framework
# because Homebrew's macOS Qt6 headers need this target

if(IOS AND NOT TARGET OpenGL::GL)
    set(_ios_sdk "${CMAKE_OSX_SYSROOT}")
    if(NOT _ios_sdk)
        execute_process(COMMAND xcrun --sdk iphoneos --show-sdk-path
                        OUTPUT_VARIABLE _ios_sdk OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    add_library(OpenGL::GL UNKNOWN IMPORTED)
    set_target_properties(OpenGL::GL PROPERTIES
        IMPORTED_LOCATION "${_ios_sdk}/System/Library/Frameworks/OpenGLES.framework"
        INTERFACE_INCLUDE_DIRECTORIES "${_ios_sdk}/System/Library/Frameworks/OpenGLES.framework/Headers"
    )

    set(WrapOpenGL_FOUND TRUE)
    set(OPENGL_FOUND TRUE)
    set(OPENGL_GLES2_INCLUDE_DIR "${_ios_sdk}/System/Library/Frameworks/OpenGLES.framework/Headers")
endif()
