add_library(kvs INTERFACE)

target_include_directories(kvs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/kvs)

target_sources(kvs INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kvs/kvs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kvs/kvs.inl
)

if (BUILD_TESTS)
    add_executable(kvs-tests tests/kvs_test.cpp)

    if (KVS_ENABLE_LOGS)
        target_compile_definitions(kvs-tests PRIVATE KVS_ENABLE_LOGS)
    endif()
    
    target_link_libraries(kvs-tests PRIVATE kvs GTest::gtest GTest::gtest_main)
    gtest_discover_tests(kvs-tests)
endif()