FIND_PROGRAM(LCOV_BIN lcov)
FIND_PROGRAM(GENHTML_BIN genhtml)

#coverage set up
if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
if (LCOV_BIN)
    SET(CMAKE_FILES_DIR ${CMAKE_BINARY_DIR}/CMakeFiles)
    SET(COV_TEMP_DIR ${CMAKE_FILES_DIR}/cov_temp)
    SET(COV_OUT_DIR ${CMAKE_BINARY_DIR}/coverage)
    #coverage copmilation parameters
    SET(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS_COVERAGE "-g -O0 --coverage -fprofile-arcs -ftest-coverage")

    SET(LCOV_PARAMS "--no-external")
    if(GCOV_TOOL)
        SET(LCOV_PARAMS "${LCOV_PARAMS} --gcov-tool {GCOV_TOOL}")
    endif(GCOV_TOOL)

    # Create the gcov target. Run coverage tests with 'make gcov'
    ADD_CUSTOM_TARGET(coverage
            COMMAND mkdir -p ${COV_TEMP_DIR}
            COMMAND ${LCOV_BIN} ${LCOV_PARAMS} -q -c -i -d ${PROJECT_SOURCE_DIR} -o ${COV_TEMP_DIR}/app_base.info
            COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
    ADD_CUSTOM_COMMAND(TARGET coverage
            COMMAND echo "=================== LCOV ===================="
            COMMAND ${LCOV_BIN} ${LCOV_PARAMS} -q -c -d ${PROJECT_SOURCE_DIR} -o app_test.info
            COMMAND ${LCOV_BIN} ${LCOV_PARAMS} -a app_base.info -a app_test.info -o app_total.info
            COMMAND ${GENHTML_BIN} -q app_total.info --output-directory ${COV_OUT_DIR}
            WORKING_DIRECTORY ${COV_TEMP_DIR}
            )
    ADD_DEPENDENCIES(coverage ${TEST_TARGET})
    # Make sure to clean up the coverage folder
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${COV_OUT_DIR} ${COV_TEMP_DIR})

    # Create the gcov-clean target. This cleans the build as well as generated
    # .gcda and .gcno files.
    ADD_CUSTOM_TARGET(scrub
            COMMAND ${CMAKE_MAKE_PROGRAM} clean
            COMMAND rm -f ${OBJECT_DIR}/*.gcno
            COMMAND rm -f ${OBJECT_DIR}/*.gcda
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
else (LCOV_BIN)
    MESSAGE("Lcov tool was not found. Install lcov.")
endif (LCOV_BIN)
endif (CMAKE_BUILD_TYPE STREQUAL "Coverage")
