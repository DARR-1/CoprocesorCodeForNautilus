cmake_minimum_required(VERSION 3.10.0)
project(NautilusCoproccesor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${OpenCV_INCLUDE_DIRS})

include(CTest)
enable_testing()

add_executable(NautilusCoproccesor astar.cpp main.cpp Server.cpp)

target_link_libraries(NautilusCoproccesor
    Ws2_32
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
