ТЕОРЕТИЧНІ ВІДОМОСТІ:

UDP (User Datagram Protocol) — це протокол транспортного рівня в мережі TCP/IP, який використовується для відправки даних між комп’ютерами.

Основні характеристики:

1. Тип          | Безз’єднувальний (connectionless)                  
2. Надійність   | Не гарантує доставку, порядок або контроль помилок 
3. Швидкість    | Дуже швидкий, бо немає перевірок                   
4. Структура    | Дані надсилаються “пакетами” (датаграми)           
5. Використання | Стримінг, онлайн-ігри, VoIP, DNS                   

в TCP клієнт і сервер підключаються один до одного, тобто є підтвердження отримання пакету,
а в UDP сервер просто отримує пакети від будь-якого клієнта і немає гарантії, що пакет дійде

Структура UDP-пакету

a. Source Port (порт джерела) - 16 біт (0–15) -- Порт, з якого відправлено пакет

b. Destination Port (порт призначення) - 16 біт (16–31) -- Порт, куди надсилається пакет

c. Length (довжина) - 16 біт (32–47) -- Довжина заголовка + даних у байтах (мінімум 8)

d. Checksum (контрольна сума) - 16 біт (48–63) -- Перевірка цілісності заголовка і даних; 0 = не обчислено

e. Data / Payload - 0–65535 байт (64–… ) -- Дані пакета, фактична інформація


Детально по бітам:

1. Source Port (16 біт) - [15 14 13 ... 2 1 0]

2. Destination Port (16 біт) - [31 30 29 ... 18 17 16]

3. Length (16 біт) - [47 46 45 ... 34 33 32]

4. Checksum (16 біт) - [63 62 61 ... 50 49 48]

5. Data / Payload - [64 ... N]


Використання в програмуванні

- sendto() / recvfrom() у C/C++
- Клієнт просто відправляє датаграму серверу
- Сервер приймає будь-які пакети та визначає IP + порт відправника


Переваги UDP

- Швидкий
- Мало накладних витрат
- Підходить для реального часу

Недоліки UDP

- Пакети можуть загубитись
- Порядок пакетів не гарантується
- Сервер повинен сам обробляти багатьох клієнтів


-----------------------------------
Код сокет-клієнта та сокет-сервера:
-----------------------------------

UDP клієнт та сервер на C++, який відповідає наступним умовам:

- багато клієнтів можуть одночасно надсилати повідомлення
- сервер показує IP + порт клієнта
- якщо клієнт пише quit, то клієнтська програма завершується
- якщо сервер вводить quit, то серверна програма завершується

------------
Сервер (UDP)
------------

// udp_server.cpp

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Створення сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Прив’язка
    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    std::cout << "UDP сервер запущено на порту " << PORT << std::endl;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        // Прийом повідомлення
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&client_addr, &addr_len);

        // Інформація про клієнта
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);

        std::cout << "["
                  << client_ip << ":" << client_port
                  << "] -> " << buffer << std::endl;

        // Перевірка введення з сервера (неблокуюче)
        std::string server_input;
        if (std::cin.rdbuf()->in_avail() > 0) {
            std::getline(std::cin, server_input);
            if (server_input == "quit") {
                std::cout << "Сервер завершує роботу..." << std::endl;
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}


------------
Клієнт (UDP)
------------

// udp_client.cpp

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Створення сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    std::cout << "Введіть повідомлення (quit для виходу):\n";

    while (true) {
        std::string message;
        std::getline(std::cin, message);

        if (message == "quit") {
            std::cout << "Клієнт завершує роботу..." << std::endl;
            break;
        }

        sendto(sockfd, message.c_str(), message.size(), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));
    }

    close(sockfd);
    return 0;
}


-----------------------------------------------------

1. Скомпілювати (NetBeans, VirtualBox, Eclipse тощо):

g++ udp_server.cpp -o server
g++ udp_client.cpp -o client

2. Запустити сервер: ./server
3. Запустити кілька клієнтів (у різних терміналах): ./client

Ці приклади реалізовані в однопоточному виконанні, - тобто сервер приймає повідомлення послідовно використовуючи цикл while. Якщо доведеться робити якісь довгі операції (наприклад, запис у базу, обробка даних), сервер може зависнути і не зможе приймати інші пакети.


Багатопоточність, наприклад, дозволить серверу обробляти кілька клієнтів одночасно - кожне повідомлення від окремого клієнта можна обробляти у своєму потоці, не блокуючи головний цикл.


Багатопоточний UDP-сервер:

// udp_server_multithread.cpp

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex cout_mutex;      // щоб потоки не плутали вивід
std::atomic<bool> running(true);  // для завершення роботи сервера

void handle_client(char* buffer, sockaddr_in client_addr) {
    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);

    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "[" << client_ip << ":" << client_port
              << "] -> " << buffer << std::endl;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // створення сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Помилка створення сокета\n";
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Помилка прив'язки\n";
        return 1;
    }

    std::cout << "Багатопотоковий UDP сервер запущено на порту " << PORT << std::endl;

    // потік для введення сервера (quit)
    std::thread input_thread([&]() {
        std::string server_input;
        while (running) {
            std::getline(std::cin, server_input);
            if (server_input == "quit") {
                running = false;
                std::cout << "Сервер завершує роботу..." << std::endl;
            }
        }
    });

    // основний цикл отримання повідомлень
    std::vector<std::thread> threads;
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT,
                             (struct sockaddr*)&client_addr, &addr_len);
        if (n > 0) {
            // обробка пакета в окремому потоці
            threads.emplace_back(handle_client, buffer, client_addr);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // очікування всіх потоків
    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    if (input_thread.joinable()) input_thread.join();
    close(sockfd);

    return 0;
}


---------------------------------------
Розібравшись з суттю предметної області та реалізувавши правильну схему і працюючу логіку клієнт-сервера, тепер напишемо цю логіку в стилі ООП:
---------------------------------------

ООП UDP-сервер: udp_server_oop.cpp

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


ООП UDP-клієнт: udp_client_oop.cpp

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



Як запускати?
--------------------------
1. Компіляція:

g++ udp_server_oop.cpp -o server -pthread
g++ udp_client_oop.cpp -o client

2. Запуск сервера: ./server
3. Запуск клієнта: ./client

Під час тестування і демонстрації слід запустити не один а мінімум два і більше клієнтів.


Контрольні запитання:
---------------------------------------------
1. Який порт використовується ?
2. Що таке private і public в даній програмі?
3. Що таке AF_INET і які бувають інші типи?
4. Навіщо використовується htons(PORT)?
5. Що таке SOCK_DGRAM і які бувають інші типи?
6. Що означає наступний рядок -  struct sockaddr_in server_addr;?
7. Навіщо в коді використовуються наступні заголовочні файли?

#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

8. Що означає наступний блок коду:

~ClientUDP() {
    close(sockfd);
}


