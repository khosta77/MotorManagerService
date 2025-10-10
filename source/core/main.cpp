#include "user_core.hpp"
#include "ft232rl.hpp"
#include "server.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    // IpFromMainInput address_this_server_( 3, argv );
    auto module_ = std::make_unique<FT232RL>();
    auto core_ = std::make_unique<UserCore>(std::move(module_));
    Server server_("127.0.0.1", 38000, std::move(core_));
    return server_.run();
}
