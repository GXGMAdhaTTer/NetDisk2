cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
project(NetDisk2 LANGUAGES CXX)

add_subdirectory(rabitmqtest)

# 指定构建类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 缓存
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "Found CCache: ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
endif()

# 主体
add_executable(myserver)
aux_source_directory(src sources)
target_sources(myserver PUBLIC ${sources})
target_link_libraries(myserver PUBLIC wfrest workflow ssl crypt crypto alibabacloud-oss-cpp-sdk curl pthread SimpleAmqpClient)
target_compile_definitions(myserver PUBLIC MY_NAME="gcc")

# 设置可执行文件输出路径
set_property(TARGET myserver PROPERTY RUNTIME_OUTPUT_DIRECTORY /home/xiaoguogao/myProject/NetDisk2/bin)

message("PROJECT_NAME: ${PROJECT_NAME}")
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# 伪目标run
# add_custom_target(run COMMAND $<TARGET_FILE:myserver>)
# configure 伪目标，用于可视化地修改缓存变量
if (CMAKE_EDIT_COMMAND)
    add_custom_target(configure COMMAND ${CMAKE_EDIT_COMMAND} -B ${CMAKE_BINARY_DIR})
endif()