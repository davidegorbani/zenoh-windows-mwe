#include "zenoh.hxx"
#include <atomic>
#include <chrono>
#include <thread>

std::atomic<bool> isClosing = false;
std::chrono::high_resolution_clock::time_point previousTime;

void data_handler(const zenoh::Sample& sample)
{
    std::string keyExprStr(sample.get_keyexpr().as_string_view());

    if (keyExprStr == "dummyData/IMU1/linAcc")
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::cout
            << "Received data from " << keyExprStr << " at "
            << std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime).count()
            << " ms" << std::endl;
        previousTime = now;
    }
}

int main()
{
    zenoh::Config config;
    std::vector<std::string> jointsList;
    auto session = zenoh::expect<zenoh::Session>(open(std::move(config)));
    auto subscriber = zenoh::expect<zenoh::Subscriber>(
        session.declare_subscriber("dummyData/**", data_handler));
    previousTime = std::chrono::high_resolution_clock::now();

    while (!isClosing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
