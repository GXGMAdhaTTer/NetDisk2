# 指定最低版本
cmake_minimum_required(VERSION 3.15)
# 指定c++版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定编译器插件
set(CMAKE_CXX_EXTENSIONS ON)
project(NetDisk2 LANGUAGES CXX)

# 增加子项目，编译的时候可以一起编译
add_subdirectory(rabitmqtest)
add_subdirectory(server2nd)

# 指定构建类型 Debug or Release
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
add_executable(myserver) # 添加可执行文件
aux_source_directory(src sources) # 添加源文件文件夹，适用于不在同一个目录下
# file(GLOB sources *.cc *.h) # 适用于在当前文件夹
target_sources(myserver PUBLIC ${sources}) # 关联源文件
# 添加第三方库（此方法只适用于知名库）
target_link_libraries(myserver PUBLIC wfrest workflow ssl crypt crypto alibabacloud-oss-cpp-sdk curl pthread SimpleAmqpClient)
# 添加第三方库（此方法只适用于知名库）
target_compile_definitions(myserver PUBLIC MY_NAME="gcc")

# 设置可执行文件输出路径
set_property(TARGET myserver PROPERTY RUNTIME_OUTPUT_DIRECTORY /home/xiaoguogao/myProject/NetDisk2/bin)

# 输出项目参数
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