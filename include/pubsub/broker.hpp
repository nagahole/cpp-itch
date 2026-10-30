#pragma once

#include <functional>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

namespace pubsub {

template<typename T>
class Broker {

public:
  using Callback = std::function<void(const T&)>;

  void subscribe(const std::string& topic, Callback callback) {
    subscribers_[topic].push_back(std::move(callback));
  }

  void publish(const std::string& topic, const T& msg) {
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
