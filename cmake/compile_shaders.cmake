
set(SHADER_DIR "resources/shaders")

file(GLOB SHADER_FILES "${SHADER_DIR}/*.frag" "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.comp")

foreach(SHADER_FILE ${SHADER_FILES})
    string(REPLACE "." "_" FILENAME_WE ${SHADER_FILE})
    set(OUTPUT_FILE "${FILENAME_WE}.spv")
	string(REPLACE "/" "_" TARGET_NAME ${SHADER_FILE})
	string(REPLACE ":" "_" TARGET_NAME ${TARGET_NAME})
	add_custom_target(
		${TARGET_NAME}
		COMMAND glslc ${SHADER_FILE} -o ${OUTPUT_FILE}
		COMMENT "compiling ${SHADER_FILE} to ${OUTPUT_FILE}"
        VERBATIM
    )

	add_dependencies(${PROJECT_NAME} ${TARGET_NAME})
endforeach()