add_library(picomaplib STATIC
    map.cpp
)

target_include_directories(picomaplib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(picomaplib PUBLIC
    pico_stdlib
)
