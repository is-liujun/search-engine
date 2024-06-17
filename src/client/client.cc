#include <myheader.h>

#include <iostream>
#include <string>

#include "../../include/TransProtocol.hpp"
#include "../../include/json.hpp"

void test()
{
    const char *ip = "127.0.0.1";
    unsigned short port = 8080;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    RET_CHECK(socket_fd, -1, "socket");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ret = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        std::cerr << strerror(errno) << '\n';
        close(socket_fd);
        return;
    }
    std::string recv;
    while (1)
    {
        int transId = 1;
        std::cout << "请输入要查询的选项: 1为关键词, 2为网页查询"<< '\n';
        std::cin >>transId;
        std::cout << "请输入要查询的内容: "<< '\n';
        std::cin >> recv;
        std::cout << ">> " << recv << '\n';
        TransProtocol message(transId, recv);
        std::string toSend = message.toString();
        ::send(socket_fd, toSend.c_str(), toSend.size(), MSG_WAITALL);
        std::cout << "send: " << toSend.substr(8) << '\n';
        recv.clear();
        int fileSize = 0;
        ::recv(socket_fd, &fileSize, sizeof(int), MSG_NOSIGNAL);
        std::cout << "recvSize = " << fileSize << '\n';
        char str[fileSize + 1];
        str[fileSize] = '\0';
        ::recv(socket_fd, str, fileSize, MSG_NOSIGNAL);
        message = str;
        recv = message.getMessage();
        nlohmann::json jsonObject = nlohmann::json::parse(recv);
        if(message.getTransId()==100)
        {
            std::cout << jsonObject["res"] << '\n';
        }
        else if(message.getTransId()==200)
        {for (auto &item : jsonObject)
        {
            std::cout << "recv: title :" << item["title"] << '\n'
            << "content: "<< item["content"] << '\n';
        }}
    }
}

int main()
{
    test();
}