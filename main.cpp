#include <iostream>
#include <uwebsockets/App.h>
const std::string BROAD_CHANNEL = "broadcast";
const std::string SET_NAME = "SET_NAME::";
const std::string DIRECT = "DIRECT::";
const std::string TOALL = "TOALL::";

bool isToAllCommand(std::string_view message) {
  return message.find(TOALL) == 0;
}

std::string parseToAllText(std::string_view message) {
  return std::string(message.substr(TOALL.length()));
}

bool isSetNameCommand(std::string_view message) {
  return message.find(SET_NAME) == 0;
}

std::string parseName(std::string_view message) {
  return std::string(message.substr(SET_NAME.length()));
}

std::string parseRecieverId(std::string_view message) {
  std::string_view rest = message.substr(DIRECT.length());
  int pos = rest.find("::");
  std::string_view id = rest.substr(0, pos);
  return std::string(id);
}

std::string parseDirectMessage(std::string_view message) {
  std::string_view rest = message.substr(DIRECT.length());
  int pos = rest.find("::");
  std::string_view text = rest.substr(pos+2);
  return std::string(text);
}

bool isDirectCommand(std::string_view message) {
  return message.find(DIRECT) == 0;
}

std::string makeOnline(int user_id, std::string user_name) {
  return "ONLINE::" + std::to_string(user_id) + "::" + user_name;
}

std::string makeOffline(int user_id, std::string user_name) {
  return "OFFLINE::" + std::to_string(user_id) + "::" + user_name;
}

int main() {
  struct PerSocketData {
    int user_id;
    std::string name;
  };

  int last_user_id = 10;
  std::map<int, std::string> usersOnline;

  uWS::App()
    .ws<PerSocketData>("/*", {
    /* Settings */
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024,
    .idleTimeout = 600,
    .maxBackpressure = 1 * 1024 * 1024,

    /* Handlers */
    .upgrade = nullptr,
    .open = [&last_user_id, &usersOnline](auto* connection) {
      std::cout << "New connection created\n";
      PerSocketData* userData = (PerSocketData*)connection->getUserData();
      userData->user_id = last_user_id++;
      userData->name = "UNNAMED";
      usersOnline[userData->user_id] = userData->name;
      connection->subscribe(BROAD_CHANNEL);
      connection->subscribe("user#" + std::to_string(userData->user_id));
  },
  .message = [&usersOnline](auto* connection, std::string_view message, uWS::OpCode opCode) {
      std::cout << "New message recieved: \n" << message << '\n';
      PerSocketData* userData = (PerSocketData*)connection->getUserData();
      if (isSetNameCommand(message)) {
        std::cout << "User set their name\n";
        userData->name = parseName(message);
        usersOnline[userData->user_id] = userData->name;
        connection->publish(BROAD_CHANNEL, makeOnline(userData->user_id, userData->name));
        for (auto entry : usersOnline) {
          connection->send(makeOnline(entry.first, entry.second), uWS::OpCode::TEXT);
        }
      }
      if (isDirectCommand(message)) {
        std::cout << "User sent direct message\n";
        std::string id = parseRecieverId(message);
        std::string text = parseDirectMessage(message);
        connection->publish("user#" + id, "DIRECT::" + userData->name + "::" + text);
      }
      if (isToAllCommand(message)) {
        std::string text = parseToAllText(message);
        connection->publish(BROAD_CHANNEL, "TOALL::" + userData->name + "::" + text);
      }
    },
    .close = [&usersOnline](auto* connection, int /*code*/, std::string_view /*message*/) {
      std::cout << "Connection closed\n";
      PerSocketData* userData = (PerSocketData*)connection->getUserData();
      connection->publish(BROAD_CHANNEL, makeOffline(userData->user_id, userData->name));
      usersOnline.erase(userData->user_id);
    }
    }).listen(9001, [](auto* listen_socket) {
    if (listen_socket) {
      std::cout << "Listening on port " << 9001 << std::endl;
    }
  }).run();
}
