include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

function(add_es_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE evensteven_core Catch2::Catch2WithMain)
    catch_discover_tests(${name})
endfunction()

add_es_test(test_audio_io)
add_es_test(test_preprocessing)
add_es_test(test_loudness)
add_es_test(test_balance)
add_es_test(test_segmentation)
add_es_test(test_export)
