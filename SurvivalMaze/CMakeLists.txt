cmake_minimum_required(VERSION 3.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(SurvivalMaze)

FILE(GLOB SOURCES "src/*.cpp" "src/*.h")
FILE(GLOB GAME_SOURCES "src/Game/*.cpp" "src/Game/*.h")

add_executable(SurvivalMaze ${SOURCES} ${GAME_SOURCES})

make_filters("${SOURCES}")
make_filters("${GAME_SOURCES}")


target_link_libraries(SurvivalMaze PUBLIC D3D12Renderer)

set_property(TARGET SurvivalMaze PROPERTY CXX_STANDARD 17)
set_property(TARGET SurvivalMaze PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CURRENT_WORKING_DIRECTORY}")

set(CMAKE_INSTALL_PREFIX ../bin)

