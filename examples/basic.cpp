#include <pubsub/broker.hpp>
#include <iostream>

int main() {
  pubsub::Broker broker;

  int print_no = 0;

  broker.subscribe("prices", [&print_no](const std::string& msg) {
    std::cout << "got: " << std::to_string(print_no++) << " " << msg << std::endl;
  });

  for (int i = 0; i < 10; i++) {
    broker.publish("prices", "AAPL: 99.95");
  }
}
