# SARibbon — amalgamated single-file build
# 仅依赖 Qt6::Widgets

add_library(SARibbon STATIC
    SARibbon.h
    SARibbon.cpp
)

target_include_directories(SARibbon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SARibbon PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_compile_definitions(SARibbon PRIVATE
    SA_RIBBON_BAR_NO_EXPORT
    SA_COLOR_WIDGETS_NO_DLL
)

set_target_properties(SARibbon PROPERTIES
    AUTOMOC ON
)

# MSVC: UTF-8
if(MSVC)
    target_compile_options(SARibbon PRIVATE /utf-8 /bigobj)
endif()
