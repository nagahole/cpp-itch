#pragma once

#include <functional>
#include <utility>
#include <string>
#include <unordered_map>
#include <vector>

namespace pubsub {

using Callback = std::function<void(const std::string& msg)>;

class Broker {
public:
  void subscribe(const std::string& topic, Callback callback) {
    subscribers_[topic].push_back(std::move(callback));
  }

  void publish(const std::string& topic, const std::string& msg) {
    auto it = subscribers_.find(topic);

    if (it != subscribers_.end()) {
      for (const auto& cb : it->second) {
        cb(msg);
      }
    }
  }

private:
  std::unordered_map<std::string, std::vector<Callback>> subscribers_;
};

} // namespace pubsub
