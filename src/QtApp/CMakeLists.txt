# QtApp Module - Main application
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME}
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    IO
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
