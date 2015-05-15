
file(GLOB_RECURSE STP_TEST_SRC_FILES ./*.cpp)

add_executable (STPTest ${STP_TEST_SRC_FILES})
target_link_libraries (STPTest  ${EXTRA_LIBS})

install (TARGETS STPTest DESTINATION CMakeBuild)

