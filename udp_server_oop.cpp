#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;

public:
    ServerUDP() {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Socket error");

        // Ігноруємо сигнали від дочірніх процесів, щоб не було зомбі
        signal(SIGCHLD, SIG_IGN);

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Bind error");
    }

    ~ServerUDP() { close(sockfd); }

    void start() {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        std::cout << "Сервер чекає на повідомлення..." << std::endl;

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
            
            if (n > 0) {
                if (fork() == 0) { // Створюємо окремий ПРОЦЕС
                    std::string client_ip = inet_ntoa(client_addr.sin_addr);
                    std::cout << "[PID: " << getpid() << "] Отримано від " << client_ip << ": " << buffer << std::endl;
                    exit(0); // Дочірній процес завершує роботу
                }
            }
        }
    }
};
