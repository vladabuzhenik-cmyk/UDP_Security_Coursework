#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#define PORT 8080
#define BUFFER_SIZE 1024

class ClientUDP {

private:
    int sockfd;
    struct sockaddr_in server_addr;

public:
    ClientUDP(const std::string &ip = "127.0.0.1", int port = PORT) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Помилка створення сокета");

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
    }

    ~ClientUDP() {
        close(sockfd);
    }

    void start() {
        std::cout << "Введіть повідомлення (quit для виходу):" << std::endl;
        std::string message;

        while (true) {
            std::getline(std::cin, message);
            if (message == "quit") {
                std::cout << "Клієнт завершує роботу..." << std::endl;
                break;
            }
            send_message(message);
        }
    }

private:
    void send_message(const std::string &msg) {
        sendto(sockfd, msg.c_str(), msg.size(), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));
    }
};

int main() {
    try {
        ClientUDP client;
        client.start();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}