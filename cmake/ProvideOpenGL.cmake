# ProvideOpenGL.cmake — iOS workaround
# Must be included BEFORE find_package(Qt6) to pre-create the OpenGL::GL target
# that Qt6's WrapOpenGL dependency requires.

if(IOS AND NOT TARGET OpenGL::GL)
    if(NOT CMAKE_OSX_SYSROOT)
        execute_process(COMMAND xcrun --sdk iphoneos --show-sdk-path
                        OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    set(_gles_dir "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/OpenGLES.framework")

    add_library(OpenGL::GL UNKNOWN IMPORTED)
    set_target_properties(OpenGL::GL PROPERTIES
        IMPORTED_LOCATION "${_gles_dir}/OpenGLES"
        INTERFACE_INCLUDE_DIRECTORIES "${_gles_dir}/Headers"
    )

    # Mark as found so Qt6's WrapOpenGL sees it
    set(WrapOpenGL_FOUND TRUE CACHE BOOL "OpenGL provided by iOS OpenGLES")
    set(OPENGL_FOUND TRUE CACHE BOOL "OpenGL provided by iOS OpenGLES")
    set(OPENGL_INCLUDE_DIR "${_gles_dir}/Headers" CACHE PATH "")
    set(OPENGL_gl_LIBRARY "${_gles_dir}/OpenGLES" CACHE FILEPATH "")
    set(OPENGL_GLES2_INCLUDE_DIR "${_gles_dir}/Headers" CACHE PATH "")

    # Prevent cmake from running its own FindOpenGL
    set(OpenGL_FOUND TRUE)
    set(OpenGL_INCLUDE_DIR "${_gles_dir}/Headers")
    set(OpenGL_gl_LIBRARY "${_gles_dir}/OpenGLES")

    message(STATUS "iOS OpenGL workaround: using ${_gles_dir}")
endif()
