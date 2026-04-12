set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "$<TARGET_FILE_DIR:Level-Editor>/shaders")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

# Fragment Shaders
file(GLOB SHADER_FILES "${SHADER_DIR}/*.frag.glsl")
message(STATUS "Compiling shaders in ${SHADER_DIR} to SPIR-V format")
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.frag")
    
    string(REPLACE "/" "_" TARGET_NAME "${SHADER_FILE}")
    string(REPLACE ":" "_" TARGET_NAME "${TARGET_NAME}")
    string(REPLACE "." "_" TARGET_NAME "${TARGET_NAME}")
    
    add_custom_target(
        ${TARGET_NAME}
        COMMAND glslc -fshader-stage=frag "${SHADER_FILE}" -o "${OUTPUT_FILE}"
        COMMENT "Compiling ${SHADER_NAME}.frag.glsl to SPIR-V, output: ${OUTPUT_FILE}"
        VERBATIM
    )
    
    add_dependencies(Level-Editor ${TARGET_NAME})
endforeach()

# Vertex Shaders
file(GLOB SHADER_FILES "${SHADER_DIR}/*.vert.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.vert")
    
    string(REPLACE "/" "_" TARGET_NAME "${SHADER_FILE}")
    string(REPLACE ":" "_" TARGET_NAME "${TARGET_NAME}")
    string(REPLACE "." "_" TARGET_NAME "${TARGET_NAME}")
    
    add_custom_target(
        ${TARGET_NAME}
        COMMAND glslc -fshader-stage=vert "${SHADER_FILE}" -o "${OUTPUT_FILE}"
        COMMENT "Compiling ${SHADER_NAME}.vert.glsl to SPIR-V, output: ${OUTPUT_FILE}"
        VERBATIM
    )
    
    add_dependencies(Level-Editor ${TARGET_NAME})
endforeach()

# Compute Shaders
file(GLOB SHADER_FILES "${SHADER_DIR}/*.comp.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.comp")
    
    string(REPLACE "/" "_" TARGET_NAME "${SHADER_FILE}")
    string(REPLACE ":" "_" TARGET_NAME "${TARGET_NAME}")
    string(REPLACE "." "_" TARGET_NAME "${TARGET_NAME}")
    
    add_custom_target(
        ${TARGET_NAME}
        COMMAND glslc -fshader-stage=comp "${SHADER_FILE}" -o "${OUTPUT_FILE}"
        COMMENT "Compiling ${SHADER_NAME}.comp.glsl to SPIR-V, output: ${OUTPUT_FILE}"
        VERBATIM
    )
    
    add_dependencies(Level-Editor ${TARGET_NAME})
endforeach()

add_custom_command(TARGET Level-Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Level-Editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_BINARY_DIR}/shaders" 
        "$<TARGET_FILE_DIR:Level-Editor>/shaders"
    COMMENT "Copying compiled shaders to $<TARGET_FILE_DIR:Level-Editor>/shaders"
    VERBATIM
)
