cmake_minimum_required(VERSION 3.16)
project(caliber VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network DataVisualization)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/core/expression_parser.cpp
    src/core/math_engine.cpp
    src/core/history_manager.cpp
    src/modes/basic/basic_widget.cpp
    src/modes/scientific/scientific_widget.cpp
    src/modes/programming/programming_widget.cpp
    src/modes/date/date_widget.cpp
    src/modes/conversion/unit_converter.cpp
    src/modes/conversion/currency_api.cpp
    src/modes/conversion/conversion_widget.cpp
    src/modes/equations/equations_widget.cpp
    src/modes/graphing/function_parser.cpp
    src/modes/graphing/graphing_widget.cpp
    src/widgets/display_widget.cpp
    src/widgets/mode_sidebar.cpp
    src/widgets/history_panel.cpp
    resources/resources.qrc
)

set(HEADERS
    src/mainwindow.h
    src/core/expression_parser.h
    src/core/math_engine.h
    src/core/history_manager.h
    src/modes/basic/basic_widget.h
    src/modes/scientific/scientific_widget.h
    src/modes/programming/programming_widget.h
    src/modes/date/date_widget.h
    src/modes/conversion/unit_converter.h
    src/modes/conversion/currency_api.h
    src/modes/conversion/conversion_widget.h
    src/modes/equations/equations_widget.h
    src/modes/graphing/function_parser.h
    src/modes/graphing/graphing_widget.h
    src/widgets/display_widget.h
    src/widgets/mode_sidebar.h
    src/widgets/history_panel.h
)

if(WIN32)
    add_executable(caliber WIN32 ${SOURCES} ${HEADERS} windows/caliber.rc)
else()
    add_executable(caliber ${SOURCES} ${HEADERS})
endif()

target_include_directories(caliber PRIVATE src)

target_link_libraries(caliber
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::DataVisualization
)

# ── Windows deployment ────────────────────────────────────────────────────────
if(WIN32)
    # After build, run windeployqt to copy all required Qt DLLs next to the exe
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET caliber POST_BUILD
            COMMAND "${WINDEPLOYQT}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:caliber>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)
if(UNIX AND NOT APPLE)
    install(TARGETS caliber RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES resources/icons/caliber.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install(FILES package/caliber.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
elseif(APPLE)
    # Bundle install handled by macOS packaging script
    install(TARGETS caliber BUNDLE DESTINATION .)
else()
    install(TARGETS caliber RUNTIME DESTINATION .)
endif()

# ── CPack — common metadata ───────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME            "caliber")
set(CPACK_PACKAGE_VENDOR          "Caliber")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical calculator — Basic, Scientific, Programming, Graphing and more")
set(CPACK_PACKAGE_VERSION         "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR   "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR   "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH   "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT         "caliber@example.com")
set(CPACK_PACKAGE_HOMEPAGE_URL    "https://github.com/imloafy/caliber")
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()
set(CPACK_STRIP_FILES             TRUE)

# ── CPack — DEB (.deb for Debian/Ubuntu) ─────────────────────────────────────
set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "Caliber <caliber@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION     "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY    "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6widgets6 (>= 6.2), libqt6charts6 (>= 6.2), libqt6network6 (>= 6.2), libqt6datavisualization6 (>= 6.2)")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS   ON)

# ── CPack — RPM (.rpm for Fedora/RHEL) ───────────────────────────────────────
set(CPACK_RPM_PACKAGE_LICENSE        "MIT")
set(CPACK_RPM_PACKAGE_GROUP          "Applications/Productivity")
set(CPACK_RPM_PACKAGE_REQUIRES
    "qt6-qtbase >= 6.2, qt6-qtcharts >= 6.2, qt6-qtdatavis3d >= 6.2")
set(CPACK_RPM_PACKAGE_AUTOREQPROV    ON)

# ── CPack — NSIS (.exe for Windows) ──────────────────────────────────────────
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME       "Caliber")
    set(CPACK_NSIS_PACKAGE_NAME       "Caliber")
    set(CPACK_NSIS_INSTALL_ROOT       "$PROGRAMFILES64")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "caliber.exe")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\Caliber.lnk' '$INSTDIR\\\\caliber.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\Caliber.lnk'")
endif()

# ── CPack — TGZ (.tar.gz portable tarball) ───────────────────────────────────
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)

include(CPack)
