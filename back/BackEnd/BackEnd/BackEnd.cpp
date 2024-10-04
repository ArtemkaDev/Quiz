#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <json/json.h>
#include <stack>
#include <memory>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFLEN 512
#define PUBLIC_FOLDER "./public"

void initializeWinsock() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "Ошибка инициализации Winsock: " << iResult << std::endl;
        exit(1);
    }
}

SOCKET createListenSocket() {
    struct addrinfo* result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "Ошибка функции getaddrinfo: " << iResult << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки сокета: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Ошибка прослушивания сокета: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    return ListenSocket;
}

class Car {
protected:
    int wheels;
    std::string color;

public:
    Car(int w, const std::string& c) : wheels(w), color(c) {}
    virtual void displayInfo() const = 0;
    virtual ~Car() = default;
    virtual Json::Value toJson() const = 0;
};

class Sedan : public Car {
private:
    int maxSpeed;

public:
    Sedan(int w, const std::string& c, int ms) : Car(w, c), maxSpeed(ms) {
        if (wheels != 4) throw std::invalid_argument("Sedan must have 4 wheels");
    }

    void displayInfo() const override {
        std::cout << "Sedan: " << color << ", Max Speed: " << maxSpeed << std::endl;
    }

    Json::Value toJson() const override {
        Json::Value json;
        json["type"] = "Sedan";
        json["wheels"] = wheels;
        json["color"] = color;
        json["maxSpeed"] = maxSpeed;
        return json;
    }
};

// Класы для Pickup, Truck, Bus

std::unique_ptr<Car> createCar(const std::string& type, int wheels, const std::string& color, int uniqueParam) {
    if (type == "Sedan") return std::make_unique<Sedan>(wheels, color, uniqueParam);
    // Другие авто
    else throw std::invalid_argument("Unknown car type");
}

std::stack<std::unique_ptr<Car>> carStack;

void handleCreateCar(SOCKET ClientSocket, const Json::Value& requestData) {
    try {
        std::string type = requestData["type"].asString();
        int wheels = requestData["wheels"].asInt();
        std::string color = requestData["color"].asString();
        int uniqueParam = requestData["uniqueParam"].asInt();

        auto car = createCar(type, wheels, color, uniqueParam);
        carStack.push(std::move(car));

        Json::Value response;
        response["status"] = "success";

        std::string responseStr = Json::writeString(Json::StreamWriterBuilder(), response);
        send(ClientSocket, responseStr.c_str(), (int)responseStr.length(), 0);
    }
    catch (const std::exception& e) {
        Json::Value response;
        response["status"] = "error";
        response["message"] = e.what();

        std::string responseStr = Json::writeString(Json::StreamWriterBuilder(), response);
        send(ClientSocket, responseStr.c_str(), (int)responseStr.length(), 0);
    }
}

void handleGetCars(SOCKET ClientSocket) {
    Json::Value response;
    response["status"] = "success";

    Json::Value cars(Json::arrayValue);
    std::stack<std::unique_ptr<Car>> tempStack(std::move(carStack)); 

    while (!tempStack.empty()) {
        cars.append(tempStack.top()->toJson());
        tempStack.pop();
    }

    response["cars"] = cars;

    std::string responseStr = Json::writeString(Json::StreamWriterBuilder(), response);
    send(ClientSocket, responseStr.c_str(), (int)responseStr.length(), 0);
}


void handleClient(SOCKET ClientSocket) {
    char recvbuf[DEFAULT_BUFLEN];
    int iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);

    if (iResult > 0) {
        std::istringstream requestStream(std::string(recvbuf, iResult));
        std::string method, path, version;
        requestStream >> method >> path >> version;

        std::string body;
        std::getline(requestStream, body);
        while (std::getline(requestStream, body) && body != "\r") {}
        std::getline(requestStream, body);

        Json::CharReaderBuilder reader;
        Json::Value requestData;
        std::string errs;
        std::istringstream s(body);
        Json::parseFromStream(reader, s, &requestData, &errs);

        if (path == "/createCar" && method == "POST") {
            handleCreateCar(ClientSocket, requestData);
        }
        else if (path == "/getCars" && method == "GET") {
            handleGetCars(ClientSocket);
        }
        else {
            std::string httpResponse =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";
            send(ClientSocket, httpResponse.c_str(), (int)httpResponse.length(), 0);
        }
    }
    closesocket(ClientSocket);
}

int main() {
    setlocale(LC_ALL, "Russian");
    initializeWinsock();

    SOCKET ListenSocket = createListenSocket();

    std::cout << "Ожидание подключения клиента..." << std::endl;

    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "Ошибка принятия соединения: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            exit(1);
        }

        handleClient(ClientSocket);
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
