#include <arpa/inet.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

constexpr int PORT = 8080;

std::string ReadFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);

  if (!file)
    return "";

  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

int main() {
  int server = socket(AF_INET, SOCK_STREAM, 0);

  if (server < 0) {
    std::cerr << "Failed to create socket\n";
    return 1;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server, (sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Bind failed\n";
    return 1;
  }

  listen(server, 10);

  std::cout << "Server running on port " << PORT << "\n";

  while (true) {
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    int clientSocket = accept(server, (sockaddr *)&client, &len);

    if (clientSocket < 0)
      continue;

    char buffer[4096] = {};

    int bytes = read(clientSocket, buffer, sizeof(buffer));

    if (bytes <= 0) {
      close(clientSocket);
      continue;
    }

    std::cout << buffer << "\n";


    std::istringstream request(buffer);

    std::string method;
    std::string path;
    std::string version;

    request >> method >> path >> version;

    if (path == "/")
      path = "/index.html";

    std::string file = "www" + path;

    std::string body = ReadFile(file);

    std::string response;

    if (body.empty()) {
      body = "<h1>404 Not Found</h1>";

      response = "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: " +
                 std::to_string(body.size()) + "\r\n\r\n" + body;
    } else {
      response = "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: " +
                 std::to_string(body.size()) + "\r\n\r\n" + body;
    }

    write(clientSocket, response.c_str(), response.size());

    close(clientSocket);
  }

  close(server);

  return 0;
}