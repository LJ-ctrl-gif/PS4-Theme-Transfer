#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr int kPort = 8080;
constexpr size_t kMaxUpload = 32 * 1024 * 1024;
const char* kRoot = "/data/PS4ThemeCreator/Received";

std::string categoryPath(const std::string& category) {
  if (category == "pictures") return std::string(kRoot) + "/Pictures";
  if (category == "music") return std::string(kRoot) + "/Music";
  if (category == "sound-effects") return std::string(kRoot) + "/SoundEffects";
  if (category == "other") return std::string(kRoot) + "/Other";
  return "";
}

bool makeDirectories(const std::string& path) {
  std::string current;
  for (char ch : path) {
    current += ch;
    if (ch == '/' && current.size() > 1) mkdir(current.c_str(), 0777);
  }
  return mkdir(path.c_str(), 0777) == 0 || errno == EEXIST;
}

std::string safeName(const std::string& input) {
  std::string output;
  for (unsigned char ch : input) {
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '_') {
      output += static_cast<char>(ch);
    } else if (ch == ' ' || ch == '(' || ch == ')') {
      output += '_';
    }
  }
  while (output.find("..") != std::string::npos) {
    output.replace(output.find(".."), 2, "_");
  }
  if (output.empty() || output == "." || output == "..") output = "received_file";
  if (output.size() > 120) output.resize(120);
  return output;
}

void sendResponse(int client, int code, const char* label,
                  const std::string& body, const char* type = "application/json") {
  char header[512];
  std::snprintf(header, sizeof(header),
      "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n"
      "Content-Length: %zu\r\nConnection: close\r\n"
      "Access-Control-Allow-Origin: *\r\n\r\n",
      code, label, type, body.size());
  send(client, header, std::strlen(header), 0);
  if (!body.empty()) send(client, body.data(), body.size(), 0);
}

bool sendAllFile(int client, FILE* file) {
  char buffer[8192];
  size_t count;
  while ((count = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
    size_t sent = 0;
    while (sent < count) {
      const ssize_t n = send(client, buffer + sent, count - sent, 0);
      if (n <= 0) return false;
      sent += static_cast<size_t>(n);
    }
  }
  return std::ferror(file) == 0;
}

void handleClient(int client) {
  std::string request;
  char buffer[4096];
  while (request.find("\r\n\r\n") == std::string::npos && request.size() < 16384) {
    const ssize_t n = recv(client, buffer, sizeof(buffer), 0);
    if (n <= 0) return;
    request.append(buffer, static_cast<size_t>(n));
  }
  const size_t split = request.find("\r\n\r\n");
  if (split == std::string::npos) {
    sendResponse(client, 400, "Bad Request", "{\"error\":\"headers too large\"}");
    return;
  }

  const std::string headers = request.substr(0, split);
  const size_t lineEnd = headers.find("\r\n");
  const std::string firstLine = headers.substr(0, lineEnd);
  const size_t firstSpace = firstLine.find(' ');
  const size_t secondSpace = firstLine.find(' ', firstSpace + 1);
  if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
    sendResponse(client, 400, "Bad Request", "{\"error\":\"invalid request\"}");
    return;
  }
  const std::string method = firstLine.substr(0, firstSpace);
  const std::string target = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);

  if (method == "GET" && target == "/api/v1/status") {
    sendResponse(client, 200, "OK",
      "{\"name\":\"PS4 Theme Transfer Receiver\",\"protocol\":1,"
      "\"categories\":[\"pictures\",\"music\",\"sound-effects\",\"other\"]}");
    return;
  }

  if (method != "POST" || target.find("/api/v1/upload?category=") != 0) {
    sendResponse(client, 404, "Not Found", "{\"error\":\"endpoint not found\"}");
    return;
  }

  const std::string category = target.substr(std::strlen("/api/v1/upload?category="));
  const std::string directory = categoryPath(category);
  if (directory.empty()) {
    sendResponse(client, 400, "Bad Request", "{\"error\":\"invalid category\"}");
    return;
  }

  size_t contentLength = 0;
  std::string fileName = "received_file";
  size_t pos = headers.find("\r\n");
  while (pos != std::string::npos && pos < headers.size()) {
    const size_t end = headers.find("\r\n", pos + 2);
    const std::string line = headers.substr(pos + 2, end - (pos + 2));
    const std::string lower = line;
    if (lower.find("Content-Length:") == 0 || lower.find("content-length:") == 0) {
      contentLength = static_cast<size_t>(std::strtoull(line.c_str() + 15, nullptr, 10));
    }
    if (lower.find("X-Filename:") == 0 || lower.find("x-filename:") == 0) {
      fileName = line.substr(11);
      while (!fileName.empty() && fileName[0] == ' ') fileName.erase(0, 1);
    }
    pos = end;
  }

  if (contentLength == 0 || contentLength > kMaxUpload) {
    sendResponse(client, 413, "Payload Too Large", "{\"error\":\"empty file or upload exceeds 32 MiB\"}");
    return;
  }
  if (!makeDirectories(directory)) {
    sendResponse(client, 500, "Internal Server Error", "{\"error\":\"cannot create storage directory\"}");
    return;
  }

  const std::string finalPath = directory + "/" + safeName(fileName);
  const std::string tempPath = finalPath + ".part";
  FILE* output = std::fopen(tempPath.c_str(), "wb");
  if (!output) {
    sendResponse(client, 500, "Internal Server Error", "{\"error\":\"cannot open destination\"}");
    return;
  }

  size_t received = 0;
  const size_t bodyStart = split + 4;
  if (request.size() > bodyStart) {
    const size_t initial = request.size() - bodyStart;
    const size_t take = initial < contentLength ? initial : contentLength;
    received += std::fwrite(request.data() + bodyStart, 1, take, output);
  }
  while (received < contentLength) {
    const size_t wanted = contentLength - received < sizeof(buffer)
        ? contentLength - received : sizeof(buffer);
    const ssize_t n = recv(client, buffer, wanted, 0);
    if (n <= 0) break;
    received += std::fwrite(buffer, 1, static_cast<size_t>(n), output);
  }
  std::fclose(output);

  if (received != contentLength) {
    std::remove(tempPath.c_str());
    sendResponse(client, 400, "Bad Request", "{\"error\":\"incomplete upload\"}");
    return;
  }
  if (std::rename(tempPath.c_str(), finalPath.c_str()) != 0) {
    std::remove(tempPath.c_str());
    sendResponse(client, 500, "Internal Server Error", "{\"error\":\"could not finalize file\"}");
    return;
  }
  sendResponse(client, 201, "Created", "{\"ok\":true}");
}
}  // namespace

int main() {
  const int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) return 1;
  int reuse = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(kPort);
  if (bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 ||
      listen(server, 4) < 0) {
    close(server);
    return 2;
  }

  while (true) {
    const int client = accept(server, nullptr, nullptr);
    if (client < 0) continue;
    handleClient(client);
    close(client);
  }
  close(server);
  return 0;
}
