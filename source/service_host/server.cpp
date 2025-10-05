#include "server.hpp"

void Server::launchServer()
{
    if (int listenCode = listen(server_fd_, SOMAXCONN); listenCode)
    {
        close(server_fd_);
        throw ListenException(listenCode);
    }
    // Пусть будет выводится созданный сервер
    std::cout << std::format("Server \'{}\' launch: {}:{}", core_->serverName_, ip_, port_) << std::endl;
}

void Server::settingsFileDescriptor()
{
    // Настраиваем массив pollfd
    fds_[0].fd = server_fd_;
    fds_[0].events = POLLIN;

    for (int i = 1; i <= MAX_CLIENTS; ++i)
    {
        clients_fds_[i - 1] = -1;
        fds_[i].fd = -1;
    }
}

void Server::checkingSocketsOnNewConnect()
{
    if ((fds_[0].revents & POLLIN))
    {
        clients_fds_[nfds_ - 1] = accept(server_fd_, (struct sockaddr*)&client_addr_, &client_len_);

        if (clients_fds_[nfds_ - 1] < 0)
            std::cerr << "---> \"accept\" in checkingSocketsOnNewContentOrConnect" << std::endl;

        fds_[nfds_].fd = clients_fds_[(nfds_ - 1)];
        fds_[nfds_].events = POLLIN;
        ++nfds_;
    }
}

bool Server::ifMessageEmptyCloseSocket(const int i)
{
    for (auto client = clients_name_.begin(); client != clients_name_.end(); ++client)
    {
        if ((*client).second == clients_name_[fds_[i].fd])
        {
            std::cout << std::format("\t[USER-ERASE]({})\n", clients_name_[fds_[i].fd]);
            clients_name_.erase(client);
            break;
        }
    }
    close(fds_[i].fd);
    fds_[i].fd = -1;
    return true;
}

bool Server::get_WhoAmI_Info(const int i, std::string& message)
{
    if (clients_name_.count(fds_[i].fd))
        return true;

    std::cout << "->" << message << std::endl;
    try
    {
        auto aboutNewUser = deserialize<pkg::WhoWantsToTalkToMe>(message);
        clients_name_[fds_[i].fd] = std::move(aboutNewUser.name);
        std::cout << std::format("\t[USER-ADD]({})\n", clients_name_[fds_[i].fd]);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    { /* nothing */
    }

    return false;
}

void Server::processTheRequest([[maybe_unused]] const int i, [[maybe_unused]] std::string& message)
{
    try
    {
        core_->Process(fds_[i].fd, clients_name_[fds_[i].fd], message);
    }
    catch (const std::exception& emsg)
    {
        std::cerr << emsg.what() << std::endl;
    }
}

void Server::checkingSocketsOnNewContent()
{
    for (int i = 1; i < nfds_; ++i)
    {
        if ((fds_[i].revents & POLLIN))
        {
            std::string totalMessage = readFromSock(fds_[i].fd);

            // В сообщении приходит бесконечный поток, разибраем его
            std::vector<std::string> messages = split(std::move(totalMessage));

            if (messages.empty())
            {
                ifMessageEmptyCloseSocket(i);
                continue;
            }

            for (std::string message : messages)
            {
                if (get_WhoAmI_Info(i, message))
                {
                    std::cout << "=>" << message << std::endl;
                    processTheRequest(i, message);
                }

                message.clear();
            }

            totalMessage.clear();
            messages.clear();
        }
    }
}

Server::Server(const std::string& IP, const int& PORT, std::unique_ptr<ICore> core)
    : NetworkSerializer()
    , ip_(IP)
    , port_(PORT)
    , core_(std::move(core))
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1)
        throw SocketNotCreate();

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    server_addr_.sin_port = htons(port_);

    // Привязка сокета к адресу
    if (int bindCode = bind(server_fd_, (struct sockaddr*)&server_addr_, sizeof(server_addr_)); bindCode)
    {
        close(server_fd_);
        throw BindFailure();
    }
}

Server::~Server()
{
    close(server_fd_);
    for (int i = 1; i < nfds_; i++)
    {
        if (fds_[i].fd != -1)
            close(fds_[i].fd);
    }
}

int Server::run()
{
    serverWorkStatus_ = false;
    core_->Init();
    launchServer();
    settingsFileDescriptor();

    while (true)
    {
        if (int ret = poll(fds_.get(), nfds_, 0); ret == -1)
            throw POLLDestroyed();

        checkingSocketsOnNewConnect();
        checkingSocketsOnNewContent();
        core_->Launch();

        if (serverWorkStatus_)
            break;
    }
    core_->Stop();
    return 0;
}

void Server::stop()
{
    serverWorkStatus_ = true;
}
