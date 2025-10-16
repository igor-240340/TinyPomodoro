cmake_minimum_required(VERSION 3.22)

project(TinyPomodoro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/bass)

if (WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/TinyPomodoro.rc")
endif()

add_executable(TinyPomodoro main.cpp ${APP_ICON_RESOURCE})

if(WIN32)
    target_link_libraries(TinyPomodoro ${CMAKE_SOURCE_DIR}/bass/bass.lib) 
    add_custom_command(TARGET TinyPomodoro POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/bass/bass.dll
            $<TARGET_FILE_DIR:TinyPomodoro>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/bell-ding.mp3
            $<TARGET_FILE_DIR:TinyPomodoro>
    )
elseif(UNIX)
    target_link_libraries(TinyPomodoro ${CMAKE_SOURCE_DIR}/bass/libbass.so)
    add_custom_command(TARGET TinyPomodoro POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/bell-ding.mp3
            $<TARGET_FILE_DIR:TinyPomodoro>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/bass/libbass.so
            $<TARGET_FILE_DIR:TinyPomodoro>
    )
    set_target_properties(TinyPomodoro PROPERTIES BUILD_RPATH "$ORIGIN")
endif()
