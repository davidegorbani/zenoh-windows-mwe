#include "zenoh.hxx"
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>

std::atomic<bool> isClosing{false};

int main(int argc, char** argv)
{
	std::cerr << "pub.exe: " << std::endl;
    double periodInSec = 0.01;

    zenoh::Config config;
    auto session = zenoh::expect<zenoh::Session>(open(std::move(config)));
    int periodInUs = periodInSec * 1e6;
    auto Ts = std::chrono::microseconds(periodInUs);

    while (!isClosing)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        std::vector<double> accNode = {1.0, 2.0, 3.0};
        std::vector<double> angVelNode = {0.0, 0.0, 0.0};
        std::vector<double> rotNode = {1.0, 0.0, 0.0, 0.0};

        for (int i = 1; i <= 12; i++)
        {
            session.put("dummyData/IMU" + std::to_string(i) + "/linAcc", accNode);
            session.put("dummyData/IMU" + std::to_string(i) + "/angVel", angVelNode);
            session.put("dummyData/IMU" + std::to_string(i) + "/orientation", rotNode);
            session.put("dummyData/IMU" + std::to_string(i) + "/magnetometer", rotNode);
        }

        auto endTime = std::chrono::high_resolution_clock::now();

        auto nextTime = startTime + Ts;

        auto remaining_ms
            = std::chrono::duration_cast<std::chrono::milliseconds>(nextTime - endTime);

        if (remaining_ms.count() > 0)
        {
            std::this_thread::sleep_until(nextTime);
        }
    }

    return 0;
}
