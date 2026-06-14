# Tests CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(catch2)

# Create the test executable
add_executable(NimbusTests
    Main.cpp
    AudioEngine/AudioGraphTests.cpp
)

# Link Catch2
target_link_libraries(NimbusTests PRIVATE Catch2::Catch2WithMain)
target_include_directories(NimbusTests PRIVATE ../Source)
