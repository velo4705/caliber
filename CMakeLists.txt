cmake_minimum_required(VERSION 3.16)
project(caliber VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network)
# DataVisualization only for desktop — mobile doesn't have desktop OpenGL headers
if(NOT IOS AND NOT ANDROID)
    find_package(Qt6 QUIET COMPONENTS DataVisualization)
    if(Qt6DataVisualization_FOUND)
        add_compile_definitions(HAVE_DATAVISUALIZATION)
    endif()
endif()

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
    src/modes/statistics/statistics_widget.cpp
    src/modes/calculus/calculus_widget.cpp
    src/modes/financial/financial_widget.cpp
    src/modes/numbertheory/numbertheory_widget.cpp
    src/modes/electrical/electrical_widget.cpp
    src/modes/digitallogic/digitallogic_widget.cpp
    src/modes/vectors/vectors_widget.cpp
    src/modes/physics/physics_widget.cpp
    src/modes/chemistry/chemistry_widget.cpp
    src/modes/civilmech/civilmech_widget.cpp
    src/modes/advancedmath/advancedmath_widget.cpp
    src/modes/discretemath/discretemath_widget.cpp
    src/modes/mcs/mcs_widget.cpp
    src/modes/signalprocessing/signalprocessing_widget.cpp
    src/modes/controlsystems/controlsystems_widget.cpp    src/widgets/display_widget.cpp
    src/widgets/mode_sidebar.cpp
    src/widgets/history_panel.cpp
    src/widgets/formula_panel.cpp
    src/widgets/animated_stacked_widget.cpp
    src/widgets/gradient_theme_dialog.cpp
    src/widgets/swipeable_tab_widget.cpp
    src/widgets/mode_pager.cpp
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
    src/modes/statistics/statistics_widget.h
    src/modes/calculus/calculus_widget.h
    src/modes/financial/financial_widget.h
    src/modes/numbertheory/numbertheory_widget.h
    src/modes/electrical/electrical_widget.h
    src/modes/digitallogic/digitallogic_widget.h
    src/modes/vectors/vectors_widget.h
    src/modes/physics/physics_widget.h
    src/modes/chemistry/chemistry_widget.h
    src/modes/civilmech/civilmech_widget.h
    src/modes/advancedmath/advancedmath_widget.h
    src/modes/discretemath/discretemath_widget.h
    src/modes/mcs/mcs_widget.h
    src/modes/signalprocessing/signalprocessing_widget.h
    src/modes/controlsystems/controlsystems_widget.h
    src/widgets/display_widget.h
    src/widgets/mode_sidebar.h
    src/widgets/history_panel.h
    src/widgets/formula_panel.h
    src/widgets/animated_stacked_widget.h
    src/widgets/gradient_theme_dialog.h
    src/widgets/swipeable_tab_widget.h
    src/widgets/mode_pager.h
)

if(WIN32)
    add_executable(caliber WIN32 ${SOURCES} ${HEADERS} windows/caliber.rc)
elseif(ANDROID)
    qt_add_executable(caliber ${SOURCES} ${HEADERS})
    set_target_properties(caliber PROPERTIES
        QT_ANDROID_APP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
    )
else()
    add_executable(caliber ${SOURCES} ${HEADERS})
endif()

target_include_directories(caliber PRIVATE src)

target_link_libraries(caliber PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    $<$<BOOL:${Qt6DataVisualization_FOUND}>:Qt6::DataVisualization>
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

# ── Android ──────────────────────────────────────────────────────────────────
if(ANDROID)
    set(QT_ANDROID_APP_ICON "@mipmap/ic_launcher")
    set(QT_ANDROID_APP_NAME "Caliber")
    set(QT_ANDROID_PACKAGE_NAME "com.caliber.app")
    set(QT_ANDROID_APP_VERSION_NAME "2.0.0")
    set(QT_ANDROID_APP_VERSION_CODE "20000")
    set(QT_ANDROID_MIN_SDK_VERSION "28")
    set(QT_ANDROID_TARGET_SDK_VERSION "34")
    set(QT_ANDROID_PLATFORM "android-34")
    set(QT_ANDROID_MANIFEST "${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml")
    set(QT_ANDROID_EXTRA_LIBS "${QT_ANDROID_EXTRA_LIBS}")

    # Copy Android resources
    qt_add_resources(caliber "android_res"
        PREFIX "/"
        FILES
            android/res/values/strings.xml
    )
endif()

# ── iOS ──────────────────────────────────────────────────────────────────────
if(IOS)
    set_target_properties(caliber PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Caliber"
        MACOSX_BUNDLE_BUNDLE_VERSION "2.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "2.0.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.caliber.app"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/ios/Info.plist"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "16.0"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
endif()

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)
if(ANDROID)
    # Install rules handled by androiddeployqt
elseif(UNIX AND NOT APPLE)
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
set(CPACK_PACKAGE_HOMEPAGE_URL    "https://github.com/velo4705/caliber")
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
