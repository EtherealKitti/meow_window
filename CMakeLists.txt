get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

file(GLOB SOURCE_FILES
    source/*.c
)

add_library(${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        dependencies
)

add_subdirectory(dependencies/Vulkan-Headers)
add_subdirectory(dependencies/Vulkan-Loader)

find_library(vulkan
    NAMES vulkan libvulkan
    PATHS dependencies/Vulkan-Loader/loader
    NO_DEFAULT_PATH
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        vulkan
)

file(GLOB PLATFORM_SPECIFIC_SOURCE_FILES
    platformSpecificSourceFiles/${CMAKE_SYSTEM_NAME}/*.c
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            wayland-client
            xkbcommon
            ${PLATFORM_SPECIFIC_SOURCE_FILES}
    )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            winmm
            imm32
            setupapi
            version
            cfgmgr32
            ${PLATFORM_SPECIFIC_SOURCE_FILES}
    )
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)
