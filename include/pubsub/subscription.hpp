#pragma once

#include <functional>
#include <utility>

namespace pubsub{

class Subscription {

public:
  Subscription(const Subscription&) = delete;
  Subscription& operator=(const Subscription&) = delete;

  Subscription(Subscription&& other) noexcept : cleanup_(std::move(other.cleanup_)) {}
  Subscription& operator=(Subscription&& other) noexcept {
    if (this != &other) {
      if (cleanup_) {
        cleanup_(); // clean up current resource
      }
      cleanup_ = std::move(other.cleanup_);
    }
    return *this;
  }

  Subscription(std::function<void()> cleanup) : cleanup_(std::move(cleanup)) { }

  ~Subscription() {
    if (cleanup_) {
      cleanup_();
    }
  }

private:

  std::function<void()> cleanup_;
};

} // namespace pubsub
