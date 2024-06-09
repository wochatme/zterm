# the build for Windows X64
project(zterm-win64 CXX)

set(ZTERMAPP_DIR "${CMAKE_CURRENT_LIST_DIR}"
    CACHE PATH "ZTerm application directory")

set(PRECOMPILE_HEADER_FILES stdafx.h)

add_executable(${PROJECT_NAME} WIN32 
	ZTerm.cpp
	XView.cpp
	stdafx.cpp
	config.cpp
	network.cpp
	ZTerm.rc
	res/pterm.rc
	)

# Specify the precompiled header(s)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PRECOMPILE_HEADER_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Specify the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/wtl ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} libwt scintilla libsqlite secp256k1)

target_include_directories(${PROJECT_NAME} PUBLIC c:/gpt/vcpkg/packages/curl_x64-windows-static/include)
target_include_directories(${PROJECT_NAME} PUBLIC c:/gpt/vcpkg/packages/zlib_x64-windows-static/include)

target_link_libraries(${PROJECT_NAME} debug c:/gpt/vcpkg/packages/curl_x64-windows-static/debug/lib/libcurl-d.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/vcpkg/packages/zlib_x64-windows-static/debug/lib/zlibd.lib)

target_link_libraries(${PROJECT_NAME} optimized c:/gpt/vcpkg/packages/curl_x64-windows-static/lib/libcurl.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/vcpkg/packages/zlib_x64-windows-static/lib/zlib.lib)

target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/windows/Debug/libterm.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/windows/Debug/guimisc.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/Debug/guiterminal.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/Debug/eventloop.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/Debug/network.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/Debug/settings.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/Debug/utils.lib)
target_link_libraries(${PROJECT_NAME} debug c:/gpt/pterm/windows/pterm-be-list.dir/Debug/be_list.obj)

target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/windows/MinSizeRel/libterm.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/windows/MinSizeRel/guimisc.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/MinSizeRel/guiterminal.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/MinSizeRel/eventloop.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/MinSizeRel/network.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/MinSizeRel/settings.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/MinSizeRel/utils.lib)
target_link_libraries(${PROJECT_NAME} optimized c:/gpt/pterm/windows/pterm-be-list.dir/MinSizeRel/be_list.obj)

