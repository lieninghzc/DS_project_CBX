cmake_minimum_required(VERSION 3.22)

# ============================================================
# MSPM0G3507 项目 — 根 CMakeLists.txt
# 参照 ti/MSPM0G3507/GNU-Test 结构
# ============================================================

# ── C 标准 ──
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# ── 构建类型 ──
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_C_FLAGS_MINSIZEREL "-Os")
endif()

# ── 项目名 ──
set(CMAKE_PROJECT_NAME mspm0g3507_app)
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# ── SDK 路径 ──
set(SDK_DIR ${CMAKE_SOURCE_DIR}/mspm0-sdk/mspm0_sdk_2_11_00_07)

# ── 启用语言 ──
enable_language(C ASM)

# ── 生成 compile_commands.json（VSCode 智能提示用） ──
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ── 可执行文件 ──
add_executable(${CMAKE_PROJECT_NAME})

# ── 子模块（源文件 + 头文件 + 驱动库） ──
add_subdirectory(cmake/ti32cubemx)

# ── 编译后生成 .hex 和 .bin ──
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generate .hex and .bin..."
)

# ── 清理列表 ──
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    ADDITIONAL_CLEAN_FILES "${CMAKE_PROJECT_NAME}.map;${CMAKE_PROJECT_NAME}.hex;${CMAKE_PROJECT_NAME}.bin"
)
