
set(SHADER_DIR "resources/shaders")

file(GLOB SHADER_FILES "${SHADER_DIR}/*.frag.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    string(REPLACE ".glsl" "" OUTPUT_FILE ${SHADER_FILE})
	string(REPLACE "/" "_" TARGET_NAME ${SHADER_FILE})
	string(REPLACE ":" "_" TARGET_NAME ${TARGET_NAME})
	add_custom_target(
		${TARGET_NAME}
		COMMAND glslc  -fshader-stage=frag ${SHADER_FILE} -o ${OUTPUT_FILE}
		COMMENT "compiling ${SHADER_FILE} to ${OUTPUT_FILE}"
        VERBATIM
    )

	add_dependencies(${PROJECT_NAME} ${TARGET_NAME})
endforeach()


file(GLOB SHADER_FILES "${SHADER_DIR}/*.vert.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    string(REPLACE ".glsl" "" OUTPUT_FILE ${SHADER_FILE})
	string(REPLACE "/" "_" TARGET_NAME ${SHADER_FILE})
	string(REPLACE ":" "_" TARGET_NAME ${TARGET_NAME})
	add_custom_target(
		${TARGET_NAME}
		COMMAND glslc  -fshader-stage=vert ${SHADER_FILE} -o ${OUTPUT_FILE}
		COMMENT "compiling ${SHADER_FILE} to ${OUTPUT_FILE}"
        VERBATIM
    )

	add_dependencies(${PROJECT_NAME} ${TARGET_NAME})
endforeach()

file(GLOB SHADER_FILES "${SHADER_DIR}/*.comp.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    string(REPLACE ".glsl" "" OUTPUT_FILE ${SHADER_FILE})
	string(REPLACE "/" "_" TARGET_NAME ${SHADER_FILE})
	string(REPLACE ":" "_" TARGET_NAME ${TARGET_NAME})
	add_custom_target(
		${TARGET_NAME}
		COMMAND glslc  -fshader-stage=comp ${SHADER_FILE} -o ${OUTPUT_FILE}
		COMMENT "compiling ${SHADER_FILE} to ${OUTPUT_FILE}"
        VERBATIM
    )

	add_dependencies(${PROJECT_NAME} ${TARGET_NAME})
endforeach()


