#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

#define PORT 8080
#define BUFFER_SIZE 1024

class ServerUDP {

private:
    int sockfd;
    struct sockaddr_in server_addr;
    std::atomic<bool> running;
    std::mutex cout_mutex;
    std::vector<std::thread> threads;

public:
    ServerUDP(int port = PORT) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Помилка створення сокета");

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Помилка прив'язки");

        running = true;
        std::cout << "UDP сервер запущено на порту " << port << std::endl;
    }

    ~ServerUDP() {
        close(sockfd);
    }

    void start() {
        std::thread input_thread(&ServerUDP::handle_input, this);

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[BUFFER_SIZE];

        while (running) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT,
                                 (struct sockaddr*)&client_addr, &addr_len);
            if (n > 0) {
                threads.emplace_back(&ServerUDP::handle_client, this, std::string(buffer), client_addr);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Очікування всіх потоків
        for (auto &t : threads) if (t.joinable()) t.join();
        if (input_thread.joinable()) input_thread.join();
    }

private:
    void handle_client(const std::string &message, sockaddr_in client_addr) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        std::cout << "[" << client_ip << ":" << client_port << "] -> " << message << std::endl;
    }

    void handle_input() {
        std::string input;
        while (running) {
            std::getline(std::cin, input);
            if (input == "quit") {
                running = false;
                std::cout << "Сервер завершує роботу..." << std::endl;
            }
        }
    }
};

int main() {
    try {
        ServerUDP server;
        server.start();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}