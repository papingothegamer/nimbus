#include <catch2/catch_session.hpp>

int main( int argc, char* argv[] ) {
    // This is the Catch2 main entry point.
    // It can be customized later if we need global setup/teardown.
    int result = Catch::Session().run( argc, argv );
    return result;
}
