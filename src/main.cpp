// Copyright 2025 Cagribey

#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>

#include "include/mqtt_client.h"

namespace {  // Use anonymous namespace instead of global variables
std::string MQTT_BROKER_ADDRESS = "mqtt://127.0.0.1:1883";
std::string MQTT_TOPIC = "local/sim";
}  // namespace

class NumberGenerator {
 private:
    const int MIN_VALUE;
    const int MAX_VALUE;
    const double VARIATION_PERCENTAGE;
    const double MAX_START_PERCENTAGE;

    std::random_device rd;
    std::mt19937 gen;
    int previous_number;

 public:
    NumberGenerator(int min_val, int max_val, double variation, double max_start)
        : MIN_VALUE(min_val),
          MAX_VALUE(max_val),
          VARIATION_PERCENTAGE(variation),
          MAX_START_PERCENTAGE(max_start),
          gen(std::mt19937(rd())) {
        initializeFirstNumber();
    }

    int generateNext() {
        double lower_limit = previous_number * (1.0 - VARIATION_PERCENTAGE);
        double upper_limit = previous_number * (1.0 + VARIATION_PERCENTAGE);

        lower_limit = std::max(static_cast<double>(MIN_VALUE), lower_limit);
        upper_limit = std::min(static_cast<double>(MAX_VALUE), upper_limit);

        std::uniform_real_distribution<> range_distribution(lower_limit, upper_limit);
        int new_number = std::round(range_distribution(gen));
        previous_number = new_number;

        return new_number;
    }

 private:
    void initializeFirstNumber() {
        std::uniform_int_distribution<> first_number(
            MIN_VALUE, static_cast<int>(MAX_VALUE * MAX_START_PERCENTAGE));
        previous_number = first_number(gen);
    }
};

class Simulator {
 private:
    const std::string ID;
    NumberGenerator generator;
    const std::chrono::seconds SLEEP_DURATION;
    volatile sig_atomic_t* stop_flag_;
    MqttClient mqtt_client;

 public:
    Simulator(const std::string& id, volatile sig_atomic_t* stop_flag, int interval, int min_val,
              int max_val, double variation, double max_start)
        : ID(id),
          generator(min_val, max_val, variation, max_start),
          SLEEP_DURATION(interval),
          stop_flag_(stop_flag),
          mqtt_client(MQTT_BROKER_ADDRESS, ID) {}

    void run() {
        try {
            // Connect to broker
            mqtt_client.connect();

            while (!*stop_flag_) {
                auto now = std::chrono::system_clock::now();
                auto epoch_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
                        .count();

                std::string payload =
                    std::to_string(epoch_ms) + " " + std::to_string(generator.generateNext());

                mqtt_client.publish(MQTT_TOPIC + "/" + ID, payload);

                std::this_thread::sleep_for(SLEEP_DURATION);
            }

            // Disconnect from broker
            std::cout << "Disconnecting from broker..." << std::endl;
            mqtt_client.disconnect();
            std::cout << "Disconnected from broker." << std::endl;
        } catch (const mqtt::exception& exc) {
            std::cerr << "MQTT Error: " << exc.what() << std::endl;
        }
    }
};

volatile sig_atomic_t stop_generator = 0;

void handle_signal(int sig) {
    std::cout << "\nReceived signal " << sig << std::endl;
    stop_generator = 1;
}

void print_help() {
    std::cout << "Device Simulator Usage:\n"
              << "------------------------\n"
              << "Options:\n"
              << "  -h, --help            Show this help message\n"
              << "  --min VALUE           Set minimum value (default: 1)\n"
              << "  --max VALUE           Set maximum value (default: 200000)\n"
              << "  --var-percentage VAL  Set variation percentage (default: 0.05)\n"
              << "  --start VALUE         Set max start percentage (default: 0.5)\n"
              << "  --interval VALUE      Set interval in seconds (default: 2)\n\n"
              << "  --id VALUE           Set id (default: random uuid)\n\n"
              << "  --broker-address VALUE Set broker address (default: mqtt://127.0.0.1:1883)\n\n"
              << "  --topic VALUE        Set topic (default: local/sim)\n\n"
              << "Example:\n"
              << "  ./DeviceSimulator --min 100 --max 1000 --interval 1\n"
              << "  (Generates values between 100-1000 every 1 second)\n\n"
              << "Note:\n"
              << "  - var-percentage: Defines the maximum change between consecutive values\n"
              << "  - start: Defines the maximum initial value as a percentage of max value\n"
              << "  - Press Ctrl+C to stop the simulator\n";
}

std::map<std::string, std::string> parse_args(int argc, const char* const argv[]) {
    std::map<std::string, std::string> args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_help();
            exit(0);
        }

        if (i + 1 < argc) {
            if (arg == "--min") {
                args["min"] = argv[++i];
            } else if (arg == "--max") {
                args["max"] = argv[++i];
            } else if (arg == "--var-percentage") {
                args["var-percentage"] = argv[++i];
            } else if (arg == "--start") {
                args["start"] = argv[++i];
            } else if (arg == "--interval") {
                args["interval"] = argv[++i];
            } else if (arg == "--id") {
                args["id"] = argv[++i];
            } else if (arg == "--broker-address") {
                args["broker-address"] = argv[++i];
            } else if (arg == "--topic") {
                args["topic"] = argv[++i];
            } else {
                std::cerr << "Unknown argument: " << arg << "\n"
                          << "Use --help for usage information\n";
                exit(1);
            }
        }
    }
    return args;
}

int main(int argc, const char* const argv[]) {
    // Default values
    int min_val = 1;
    int max_val = 200000;
    double variation = 0.05;
    double max_start = 0.5;
    int interval = 2;

    // Generate random id
    boost::uuids::random_generator gen;
    boost::uuids::uuid uuid = gen();
    std::string id = "SIM-" + boost::uuids::to_string(uuid);

    // Parse arguments
    auto args = parse_args(argc, argv);

    // Update values
    if (args.count("min")) min_val = std::stoi(args["min"]);
    if (args.count("max")) max_val = std::stoi(args["max"]);
    if (args.count("start")) max_start = std::stod(args["start"]);
    if (args.count("var-percentage")) variation = std::stod(args["var-percentage"]);
    if (args.count("interval")) interval = std::stoi(args["interval"]);
    if (args.count("id")) id = args["id"];
    if (args.count("broker-address")) MQTT_BROKER_ADDRESS = args["broker-address"];
    if (args.count("topic")) MQTT_TOPIC = args["topic"];
    std::signal(SIGINT, handle_signal);

    Simulator simulator(id, &stop_generator, interval, min_val, max_val, variation, max_start);
    simulator.run();

    return 0;
}
