#include "MotorControlServiceCore.hpp"

int main( [[maybe_unused]] int argc, [[maybe_unused]] char *argv[] )
{
    // IpFromMainInput address_this_server_( 3, argv );
    auto module_ = std::make_shared<ModuleFT232RL>( 0, 490 );
    auto core_ = std::make_unique<MotorControlServiceCore>( module_ );
    UniversalServer server_( "127.0.0.1", 38000, std::move( core_ ) );
    return server_.run();
}
