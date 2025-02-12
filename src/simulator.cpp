// Copyright 2025 Cagribey

#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>

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
    NumberGenerator generator;
    const std::chrono::seconds SLEEP_DURATION;
    volatile sig_atomic_t* stop_flag_;

 public:
    Simulator(volatile sig_atomic_t* stop_flag, int interval, int min_val, int max_val,
              double variation, double max_start)
        : generator(min_val, max_val, variation, max_start),
          SLEEP_DURATION(interval),
          stop_flag_(stop_flag) {}

    void run() {
        while (!*stop_flag_) {
            auto now = std::chrono::system_clock::now();
            auto epoch_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
                    .count();

            std::cout << epoch_ms << " " << generator.generateNext() << std::endl;
            std::this_thread::sleep_for(SLEEP_DURATION);
        }
    }
};

volatile sig_atomic_t stop_generator = 0;

void handle_signal(int sig) {
    stop_generator = 1;
}

std::map<std::string, std::string> parse_args(int argc, const char* const argv[]) {
    std::map<std::string, std::string> args;

    for (int i = 1; i < argc; i += 2) {
        std::string flag = argv[i];
        if (i + 1 < argc) {
            if (flag == "--min")
                args["min"] = argv[i + 1];
            else if (flag == "--max")
                args["max"] = argv[i + 1];
            else if (flag == "--var-percentage")
                args["var-percentage"] = argv[i + 1];
            else if (flag == "--start")
                args["start"] = argv[i + 1];
            else if (flag == "--interval")
                args["interval"] = argv[i + 1];
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

    // Parse arguments
    auto args = parse_args(argc, argv);

    // Update values
    if (args.count("min")) min_val = std::stoi(args["min"]);
    if (args.count("max")) max_val = std::stoi(args["max"]);
    if (args.count("start")) max_start = std::stod(args["start"]);
    if (args.count("var-percentage")) variation = std::stod(args["var-percentage"]);
    if (args.count("interval")) interval = std::stoi(args["interval"]);

    std::signal(SIGINT, handle_signal);

    Simulator simulator(&stop_generator, interval, min_val, max_val, variation, max_start);
    simulator.run();

    return 0;
}
