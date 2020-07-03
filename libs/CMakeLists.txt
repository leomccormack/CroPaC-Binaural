
message(STATUS "Configuring hcropaclib...")

# Set-up project 
project(hcropaclib LANGUAGES C)
add_library(${PROJECT_NAME} STATIC)

# Link with SAF
target_link_libraries(${PROJECT_NAME} PRIVATE saf)

# Source files
target_sources(${PROJECT_NAME} 
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/hcropaclib/src/hcropac_internal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hcropaclib/src/hcropac_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hcropaclib/src/hcropaclib.c 
)

# Include directory
target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hcropaclib/include/>   
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
) 


