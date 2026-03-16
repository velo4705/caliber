cmake_minimum_required(VERSION 3.16)
project(caliber VERSION 1.0.0 LANGUAGES CXX)

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

add_executable(caliber ${SOURCES} ${HEADERS})

target_include_directories(caliber PRIVATE src)

target_link_libraries(caliber
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::DataVisualization
)
