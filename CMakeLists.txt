cmake_minimum_required(VERSION 3.26)
project(IBLRenderer)

set(CMAKE_CXX_STANDARD 11)

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 设置链接的库文件目录
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 查找指定目录下的所有源文件并收集到一个变量中
aux_source_directory(core HEADER)
aux_source_directory(src SOURCE)
aux_source_directory(include/imgui ImGui)

# 构建可执行文件
add_executable(IBLRenderer ${HEADER} ${SOURCE} ${ImGui} main.cpp)

# 链接库, glfw3使用静态库 assimp使用动态库(需要将.dll 放置程序.exe目录下或者系统变量中)
target_link_libraries(IBLRenderer libglfw3.a libassimp.dll.a)
