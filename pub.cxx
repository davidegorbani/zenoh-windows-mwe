#include "zenoh.hxx"
#include <ConfigFolderPath.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

std::atomic<bool> isClosing{false};

int main(int argc, char **argv) {
  std::cerr << "pub.exe: " << std::endl;
  double periodInSec = 0.017;

  zenoh::Config config = zenoh::expect(zenohc::config_from_file(
      (getConfigPath() + "/zenohConfigPub.json5").c_str()));
  auto session = zenoh::expect<zenoh::Session>(zenoh::open(std::move(config)));
  int periodInUs = periodInSec * 1e6;
  auto Ts = std::chrono::microseconds(periodInUs);
  std::vector<zenohc::Publisher> publishersNodes;
  std::vector<zenohc::Publisher> publishersShoes;
  std::vector<int> nodes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::vector<int> shoes = {1, 2};

  for (auto i : nodes) {
    session.declare_publisher("iFeel/node" + std::to_string(i) + "/linAcc");
    session.declare_publisher("iFeel/node" + std::to_string(i) + "/angVel");
    session.declare_publisher("iFeel/node" + std::to_string(i) +
                              "/orientation");
    publishersNodes.emplace_back(
        zenohc::expect<zenohc::Publisher>(session.declare_publisher(
            "iFeel/node" + std::to_string(i) + "/linAcc")));
    publishersNodes.emplace_back(
        zenohc::expect<zenohc::Publisher>(session.declare_publisher(
            "iFeel/node" + std::to_string(i) + "/angVel")));
    publishersNodes.emplace_back(
        zenohc::expect<zenohc::Publisher>(session.declare_publisher(
            "iFeel/node" + std::to_string(i) + "/orientation")));
  }

  for (auto i : shoes) {
    session.declare_publisher("iFeel/shoe" + std::to_string(i) + "/FT");
    publishersShoes.emplace_back(zenohc::expect<zenohc::Publisher>(
        session.declare_publisher("iFeel/shoe" + std::to_string(i) + "/FT")));
  }

  while (!isClosing) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<double> accNode = {1.0, 2.0, 3.0};
    std::vector<double> angVelNode = {0.0, 0.0, 0.0};
    std::vector<double> rotNode = {1.0, 0.0, 0.0, 0.0};
    std::vector<double> FTShoe = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    for (int i = 1; i <= nodes.size(); i++) {
      publishersNodes[3 * i - 3].put(accNode);
      publishersNodes[3 * i - 2].put(angVelNode);
      publishersNodes[3 * i - 1].put(rotNode);
    }

    for (int i = 1; i <= shoes.size(); i++) {
      publishersShoes[i - 1].put(FTShoe);
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    auto nextTime = startTime + Ts;

    auto remaining_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        nextTime - endTime);

    if (remaining_ms.count() > 0) {
      std::this_thread::sleep_until(nextTime);
    }
  }

  return 0;
}
