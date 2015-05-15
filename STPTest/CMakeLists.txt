
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE STP_TEST_SRC_FILES ./*.cpp)
add_executable (STPTest ${STP_TEST_SRC_FILES})
target_link_libraries (STPTest  ${EXTRA_LIBS})
target_link_libraries(STPTest ${GTEST_BOTH_LIBRARIES})

install (TARGETS STPTest DESTINATION STPBIN)

add_test(STPTesting STPTest)

