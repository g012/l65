cmake_minimum_required(VERSION 3.1)

project(l65)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(L65_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(L65_BINARY_DIR ${PROJECT_BINARY_DIR})

set(LINKLIBS)

if (MSVC)

    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

    add_definitions(-D_HAS_EXCEPTIONS=0 -DWIN32_LEAN_AND_MEAN -DUNICODE -D_UNICODE)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
    foreach (flag CMAKE_C_FLAGS
                  CMAKE_C_FLAGS_DEBUG
                  CMAKE_C_FLAGS_RELEASE
                  CMAKE_C_FLAGS_MINSIZEREL
                  CMAKE_C_FLAGS_RELWITHDEBINFO
                  CMAKE_CXX_FLAGS
                  CMAKE_CXX_FLAGS_DEBUG
                  CMAKE_CXX_FLAGS_RELEASE
                  CMAKE_CXX_FLAGS_MINSIZEREL
                  CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if (${flag} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
        endif()
        if(${flag} MATCHES "/EHsc")
            string(REGEX REPLACE "/EHsc" "/EHs-c-" ${flag} "${${flag}}")
        endif()
        if(${flag} MATCHES "/GR")
            string(REGEX REPLACE "/GR" "/GR-" ${flag} "${${flag}}")
        endif()
        if(${flag} MATCHES "/Gm")
            string(REGEX REPLACE "/Gm" "/Gm-" ${flag} "${${flag}}")
        endif()
    endforeach()

    add_compile_options(/WX /MP /fp:fast /volatile:iso)
    add_compile_options(/wd4103 /wd4244 /wd4456 /wd4554 /wd4557 /wd4702 /wd6255 /wd6262 /wd28278)

else ()

    if (APPLE)
        add_definitions(-DLUA_USE_MACOSX)
    else ()
        add_definitions(-DLUA_USE_LINUX)
    endif ()

	add_definitions(-D_FILE_OFFSET_BITS=64)

    add_compile_options(-fno-strict-aliasing -fomit-frame-pointer -ffast-math -fvisibility=hidden)
    add_compile_options(-Wsign-compare -Wno-missing-braces -Wno-unused-result -Wno-unused-function -Wno-unused-variable -Wno-switch -Wno-parentheses -Wno-string-plus-int)

endif()

set(L65_SOURCES 
        ${L65_SOURCE_DIR}/lfs.c
        ${L65_SOURCE_DIR}/lpeg.c
        ${L65_SOURCE_DIR}/main.c
    )
set(L65_HEADERS
        ${L65_SOURCE_DIR}/lua.h
        ${L65_BINARY_DIR}/scripts.h
        ${L65_SOURCE_DIR}/stb_image.h
    )

file(GLOB L65_FILES ${L65_SOURCE_DIR}/*.l65)
set(L65_SCRIPTS
        ${L65_SOURCE_DIR}/asm.lua
        ${L65_SOURCE_DIR}/6502.lua
        ${L65_SOURCE_DIR}/dkjson.lua
        ${L65_SOURCE_DIR}/l65.lua
        ${L65_SOURCE_DIR}/l65cfg.lua
        ${L65_SOURCE_DIR}/re.lua
        ${L65_FILES}
    )

add_executable(embed ${L65_SOURCE_DIR}/embed.c)
set_property(TARGET embed PROPERTY C_STANDARD 99)
add_custom_command(
    OUTPUT ${L65_BINARY_DIR}/scripts.h
    COMMAND embed -o ${L65_BINARY_DIR}/scripts.h ${L65_SCRIPTS}
    DEPENDS embed ${L65_SCRIPTS}
)
add_custom_target(prereq DEPENDS ${L65_BINARY_DIR}/scripts.h)
add_executable(${PROJECT_NAME} ${L65_SOURCES} ${L65_HEADERS} ${L65_FILES})
add_dependencies(${PROJECT_NAME} prereq)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
target_include_directories(${PROJECT_NAME} PRIVATE "${L65_SOURCE_DIR}" "${L65_BINARY_DIR}")

if (MSVC)

    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO /MANIFEST:NO /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")

elseif (UNIX)

    list(APPEND LINKLIBS m)
    if (NOT APPLE)
        list(APPEND LINKLIBS dl)
    endif()

endif()

target_link_libraries(embed ${LINKLIBS})
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})

set(L7801_SOURCES 
        ${L65_SOURCE_DIR}/lfs.c
        ${L65_SOURCE_DIR}/lpeg.c
        ${L65_SOURCE_DIR}/l7801.c
    )
set(L7801_HEADERS
        ${L65_HEADERS}
    )

set(L7801_FILES ${L65_SOURCE_DIR}/scv.l7801)

set(L7801_SCRIPTS
        ${L65_SOURCE_DIR}/asm.lua
        ${L65_SOURCE_DIR}/uPD7801.lua
        ${L65_SOURCE_DIR}/dkjson.lua
        ${L65_SOURCE_DIR}/l7801.lua
        ${L65_SOURCE_DIR}/l65cfg.lua
        ${L65_SOURCE_DIR}/re.lua
        ${L7801_FILES}
    )

add_custom_command(
    OUTPUT ${L65_BINARY_DIR}/scripts_7801.h
    COMMAND embed -o ${L65_BINARY_DIR}/scripts_7801.h ${L7801_SCRIPTS}
    DEPENDS embed ${L7801_SCRIPTS}
)
add_custom_target(prereq_7801 DEPENDS ${L65_BINARY_DIR}/scripts_7801.h)
add_executable(l7801 ${L7801_SOURCES} ${L7801_HEADERS} ${L7801_FILES})
add_dependencies(l7801 prereq_7801)
set_property(TARGET l7801 PROPERTY C_STANDARD 99)
target_include_directories(l7801 PRIVATE "${L7801_SOURCE_DIR}" "${L65_BINARY_DIR}")
target_link_libraries(l7801 ${LINKLIBS})

set(LZ80_SOURCES
        ${L65_SOURCE_DIR}/lfs.c
        ${L65_SOURCE_DIR}/lpeg.c
        ${L65_SOURCE_DIR}/lz80.c
    )
set(LZ80_HEADERS
        ${L65_SOURCE_DIR}/lua.h
        ${L65_BINARY_DIR}/scripts_z80.h
        ${L65_SOURCE_DIR}/stb_image.h
    )

set(LZ80_SCRIPTS
        ${L65_SOURCE_DIR}/asm.lua
        ${L65_SOURCE_DIR}/z80.lua
        ${L65_SOURCE_DIR}/dkjson.lua
        ${L65_SOURCE_DIR}/lz80.lua
        ${L65_SOURCE_DIR}/l65cfg.lua
        ${L65_SOURCE_DIR}/re.lua
        ${L65_SOURCE_DIR}/gb.lz80
        ${L65_SOURCE_DIR}/hUGEDriver.lz80
    )

add_custom_command(
    OUTPUT ${L65_BINARY_DIR}/scripts_z80.h
    COMMAND embed -o ${L65_BINARY_DIR}/scripts_z80.h ${LZ80_SCRIPTS}
    DEPENDS embed ${LZ80_SCRIPTS}
)
add_custom_target(prereq_z80 DEPENDS ${L65_BINARY_DIR}/scripts_z80.h)
add_executable(lz80 ${LZ80_SOURCES} ${LZ80_HEADERS})
add_dependencies(lz80 prereq_z80)
set_property(TARGET lz80 PROPERTY C_STANDARD 99)
target_include_directories(lz80 PRIVATE "${L65_SOURCE_DIR}" "${L65_BINARY_DIR}")
target_link_libraries(lz80 ${LINKLIBS})

install(TARGETS l65 l7801 lz80
        RUNTIME DESTINATION bin)
install(DIRECTORY samples DESTINATION doc)
install(FILES README.md DESTINATION doc)
