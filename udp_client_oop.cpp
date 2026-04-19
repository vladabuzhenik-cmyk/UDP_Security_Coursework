#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;

public:
    ServerUDP(int port = PORT) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Помилка сокета");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Помилка bind");

        // Очищення процесів-зомбі
        signal(SIGCHLD, SIG_IGN);
        std::cout << "Сервер (Multiprocessing) запущено на порті " << port << std::endl;
    }

    ~ServerUDP() { close(sockfd); }

    void start() {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[BUFFER_SIZE];

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                 (struct sockaddr*)&client_addr, &addr_len);
            if (n > 0) {
                pid_t pid = fork(); // СТВОРЕННЯ ПРОЦЕСУ

                if (pid == 0) { // Дочірній процес
                    handle_client(std::string(buffer), client_addr);
                    exit(0); 
                }
            }
        }
    }

private:
    void handle_client(const std::string &message, sockaddr_in client_addr) {
        char* ip = inet_ntoa(client_addr.sin_addr);
        std::cout << "[PID " << getpid() << "] Від " << ip << ": " << message << std::endl;
        sleep(2); // Імітація обробки
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
