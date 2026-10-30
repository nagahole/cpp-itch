#include <array>
#include <vector>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <pubsub/broker.hpp>
#include <pubsub/subscription.hpp>


TEST_CASE("subscriber receives published message") {
  pubsub::Broker<std::string> broker;
  std::vector<std::string> received;

  pubsub::Subscription sub = broker.subscribe("prices", [&received](const std::string& msg) {
    received.push_back(msg);
  });

  broker.publish("prices", "AAPL 190.5");

  REQUIRE(received == std::vector<std::string>{"AAPL 190.5"});
}

TEST_CASE("multiple subscribers receives published message") {
  constexpr size_t SUBSCRIBERS = 15;

  pubsub::Broker<std::string> broker;
  std::vector<pubsub::Subscription> subs;

  std::array<std::string, SUBSCRIBERS> received;
  size_t n_received = 0;

  for (size_t i = 0; i < SUBSCRIBERS; i++) {
    subs.push_back(broker.subscribe("prices", [&received, &n_received, i](const std::string& msg) {
      received[i] = msg;
      n_received++;
    }));
  }

  broker.publish("prices", "AAPL 55.6");

  REQUIRE(n_received == SUBSCRIBERS);

  for (size_t i = 0; i < SUBSCRIBERS; i++) {
    REQUIRE(received[i] == "AAPL 55.6");
  }
}

TEST_CASE("publishing to an unknown topic does nothing") {
  pubsub::Broker<std::string> broker;
  std::vector<std::string> received;

  pubsub::Subscription sub = broker.subscribe("prices", [&received](const std::string& msg) {
    received.push_back(msg);
  });

  broker.publish("TRADES", "MSFT 100.5");

  REQUIRE(received.empty());
}

TEST_CASE("publishing to an unknown topic does nothing with no subscribers at all") {
  pubsub::Broker<std::string> broker;

  REQUIRE_NOTHROW(broker.publish("TRADES", "MSFT 100.5"));
}

TEST_CASE("messages do not go to other topic's subscribers") {
  pubsub::Broker<std::string> broker;
  std::vector<std::string> received_prices, received_fulfillment;

  pubsub::Subscription price_sub = broker.subscribe("prices", [&received_prices](const std::string& msg) {
    received_prices.push_back(msg);
  });

  pubsub::Subscription fulfillment_sub = broker.subscribe("fulfillment", [&received_fulfillment](const std::string& msg) {
    received_fulfillment.push_back(msg);
  });

  broker.publish("prices", "AAPL 90.5");
  broker.publish("fulfillment", "(B) AAPL 90.49");

  REQUIRE(received_prices == std::vector<std::string>{"AAPL 90.5"});
  REQUIRE(received_fulfillment == std::vector<std::string>{"(B) AAPL 90.49"});
}

TEST_CASE("messages are delivered in publish order") {
  constexpr size_t MESSAGES = 5;

  pubsub::Broker<std::string> broker;
  std::vector<std::string> received;
  std::vector<std::string> expected;

  pubsub::Subscription sub = broker.subscribe("prices", [&received](const std::string& msg) {
    received.push_back(msg);
  });

  for (size_t i = 0; i < MESSAGES; i++) {
    const std::string str = std::to_string(i) + ": AAPL 0.05";
    broker.publish("prices", str);
    expected.push_back(str);
  }

  REQUIRE(received == expected);
}

TEST_CASE("callback subscribed twice calls twice") {
  pubsub::Broker<std::string> broker;

  std::vector<std::string> received;

  auto callback = [&received](const std::string& msg) {
    received.push_back(msg);
  };

  pubsub::Subscription sub_1 = broker.subscribe("prices", callback);
  pubsub::Subscription sub_2 = broker.subscribe("prices", callback);

  broker.publish("prices", "AAPL 15.4");

  REQUIRE(received == std::vector<std::string>(2, "AAPL 15.4"));
}

TEST_CASE("non-string broker template type") {
  pubsub::Broker<int> broker;

  std::vector<int> received;

  pubsub::Subscription sub = broker.subscribe("timestamp", [&received](const int& msg) {
    received.push_back(msg);
  });

  broker.publish("timestamp", 12345678);
  broker.publish("timestamp", 87654321);

  REQUIRE(received == std::vector<int>{12345678, 87654321});
}

TEST_CASE("subscription unsubscribes on destruction") {
  pubsub::Broker<std::string> broker;

  std::vector<std::string> received;

  {
    pubsub::Subscription sub = broker.subscribe("prices", [&received](const std::string& msg) {
      received.push_back(msg);
    });
  }

  broker.publish("prices", "APPL 0.05");

  REQUIRE(received == std::vector<std::string>{});
}

TEST_CASE("subscription outlives broker") {

  {
    pubsub::Subscription sub([](){});

    {
      pubsub::Broker<std::string> broker;
      sub = broker.subscribe("prices", [](const std::string&){});
    }
  }
  // no crash when `sub.cleanup_()` runs after broker is gone
}

