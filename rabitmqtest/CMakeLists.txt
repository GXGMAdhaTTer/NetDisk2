# 指定最低版本
cmake_minimum_required(VERSION 3.15)

# 指定c++版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定编译器插件
set(CMAKE_CXX_EXTENSIONS ON)

# 设置项目名和语言
project(NetDisk2 LANGUAGES CXX)

# 指定构建类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 开启缓存
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "Found CCache: ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
endif()

# 主体
# 添加可执行文件
add_executable(consumer)
# 搜索源文件
file(GLOB sources *.cc *.h) # 适用于在当前文件夹
target_sources(consumer PUBLIC ${sources})
# 添加第三方库（此方法只适用于知名库）
target_link_libraries(consumer PUBLIC alibabacloud-oss-cpp-sdk ssl crypt crypto curl pthread SimpleAmqpClient)
# 指定编译器
target_compile_definitions(consumer PUBLIC MY_NAME="gcc")

# 设置可执行文件输出路径
set_property(TARGET consumer PROPERTY RUNTIME_OUTPUT_DIRECTORY /home/xiaoguogao/myProject/NetDisk2/bin)

# 输出参数
message("PROJECT_NAME: ${PROJECT_NAME}")
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# 伪目标run
# add_custom_target(run COMMAND $<TARGET_FILE:consumer>)
# configure 伪目标，用于可视化地修改缓存变量
if (CMAKE_EDIT_COMMAND)
    add_custom_target(configure COMMAND ${CMAKE_EDIT_COMMAND} -B ${CMAKE_BINARY_DIR})
endif()
