#include "zenoh.hxx"

#include "spdlog/spdlog.h"
#include <ConfigFolderPath.h>
#include <algorithm>
#include <atomic>
#include <boost/format.hpp> // only needed for printing
#include <boost/histogram.hpp>
#include <chrono>
#include <numeric>
#include <sstream>
#include <thread>

std::atomic<bool> isClosing = false;
std::chrono::high_resolution_clock::time_point previousTime;
std::atomic<bool> firstRun = true;

std::vector<double> g_allTimeMicroseconds;
std::vector<double> g_TimeErrorMicroseconds;

void printVectorStats(const std::vector<double> &inputData) {
  std::vector<double> localDataCopy = inputData;

  // Sort the vector to calculate median and other statistics
  std::sort(localDataCopy.begin(), localDataCopy.end());

  // Calculate size
  size_t size = localDataCopy.size();

  // Calculate mean
  double sum = std::accumulate(localDataCopy.begin(), localDataCopy.end(), 0.0);
  double mean = sum / size;

  // Calculate median
  double median;
  if (size % 2 == 0) {
    median = (localDataCopy[size / 2 - 1] + localDataCopy[size / 2]) / 2;
  } else {
    median = localDataCopy[size / 2];
  }

  // Calculate min and max
  double min = localDataCopy.front();
  double max = localDataCopy.back();

  // Print the results
  double millisecondsInMicroseconds = 1000.0;
  spdlog::info("Size: {} samples", size);
  spdlog::info("Mean: {} ms", mean / millisecondsInMicroseconds);
  spdlog::info("Median: {} ms", median / millisecondsInMicroseconds);
  spdlog::info("Min: {} ms", min / millisecondsInMicroseconds);
  spdlog::info("Max: {} ms", max / millisecondsInMicroseconds);

  // Create a histogram using Boost.Histogram
  auto hist =
      boost::histogram::make_histogram(boost::histogram::axis::regular<>(
          20, 0,
          15.0 * millisecondsInMicroseconds)); // 10 bins from 0 to 100 ms

  // Fill the histogram with data
  for (const auto &value : localDataCopy) {
    hist(value);
  }

  std::ostringstream os;
  for (auto &&x :
       boost::histogram::indexed(hist, boost::histogram::coverage::all)) {
    os << boost::format("bin %2i [%4.3f, %4.3f): %i\n") % x.index() %
              (x.bin().lower() / millisecondsInMicroseconds) %
              (x.bin().upper() / millisecondsInMicroseconds) % *x;
  }

  spdlog::info("Histogram:\n {}", os.str());

  return;
}

void data_handler(const zenoh::Sample &sample) {
  std::string_view keyExprStr(sample.get_keyexpr().as_string_view());

  if (keyExprStr == "iFeel/node1/linAcc") {
    auto now = std::chrono::high_resolution_clock::now();
    if (!firstRun && !isClosing) {
      // This needs to be coherente with the value used in the publisher
      double aPrioriPeriodInMicro = 0.017 * 1000000.0;
      double timeElapsedFromLastRead =
          std::chrono::duration_cast<std::chrono::microseconds>(now -
                                                                previousTime)
              .count();
      g_allTimeMicroseconds.push_back(timeElapsedFromLastRead);
      g_TimeErrorMicroseconds.push_back(
          std::abs(timeElapsedFromLastRead - aPrioriPeriodInMicro));
    }
    previousTime = now;
    firstRun = false;
  }
}

int main() {
  zenoh::Config config = zenoh::expect(zenohc::config_from_file(
      (getConfigPath() + "/zenohConfigSub.json5").c_str()));
  std::vector<std::string> jointsList;
  auto session = zenoh::expect<zenoh::Session>(open(std::move(config)));
  std::vector<zenoh::Subscriber> subscribers;
  std::vector<int> nodes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::vector<int> shoes = {1, 2};
  for (auto i : nodes) {
    subscribers.emplace_back(
        zenoh::expect<zenoh::Subscriber>(session.declare_subscriber(
            "iFeel/node" + std::to_string(i) + "/*", data_handler)));
  }
  for (auto i : shoes) {
    subscribers.emplace_back(
        zenoh::expect<zenoh::Subscriber>(session.declare_subscriber(
            "iFeel/shoe" + std::to_string(i) + "/*", data_handler)));
  }
  //   auto subscriber = zenoh::expect<zenoh::Subscriber>(
  //       session.declare_subscriber("iFeel/**", data_handler));

  auto startProgram = std::chrono::high_resolution_clock::now();
  while (!isClosing) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto now = std::chrono::high_resolution_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(now - startProgram)
            .count() >= 10.0) {
      isClosing = true;
    }
  }

  // Print stats
  spdlog::info("=== Statistics for time between each read ===");
  printVectorStats(g_allTimeMicroseconds);
  spdlog::info("=== Statistics of error of time between each read and nominal "
               "period ===");
  printVectorStats(g_TimeErrorMicroseconds);

  return 0;
}
