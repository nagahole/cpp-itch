#pragma once

#include <functional>
#include <string>
#include <utility>
#include <unordered_map>

#include "subscription.hpp"

namespace pubsub {

template<typename T>
class Broker {

public:
  using Callback = std::function<void(const T&)>;

  Subscription subscribe(const std::string& topic, Callback cb) {
    int id = instances_++;

    subscribers_[topic].emplace(id, std::move(cb));

    std::weak_ptr<bool> weak = alive_;
    return Subscription([this, topic, id, weak]() {
      
      if (weak.lock()) {
        auto it = subscribers_.find(topic);

        if (it != subscribers_.end()) {
          it->second.erase(id);
        }
      }
    });
}

  void publish(const std::string& topic, const T& msg) {
    auto it = subscribers_.find(topic);

    if (it != subscribers_.end()) {
      for (auto& [_, cb] : it->second) {
        cb(msg);
      }
    }
  }

private:
  std::shared_ptr<bool> alive_ = std::make_shared<bool>(true);
  std::unordered_map<std::string, std::unordered_map<int, Callback>> subscribers_;

  int instances_ = 0;
};

} // namespace pubsub
