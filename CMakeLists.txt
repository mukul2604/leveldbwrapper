project(lwrapper_api CXX)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

SET(lwrapper_api-test_SRCS
  lwrapper.c
  lwrapper_api-test.cpp
)

add_executable(lwrapper_api-test
  ${lwrapper_api-test_SRCS}
)

target_link_libraries(lwrapper_api-test leveldb ${GTEST_BOTH_LIBRARIES} pthread)
