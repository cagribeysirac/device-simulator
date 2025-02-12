#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <csignal>

using namespace std;

const int MIN_VALUE = 1;
const int MAX_VALUE = 200000;
const double VARIATION_PERCENTAGE = 0.05;  // ±5%
const double MAX_START_PERCENTAGE = 0.5;
const chrono::seconds SLEEP_DURATION(2);

volatile sig_atomic_t stop_generator = 0;

void handle_signal(int sig) {
    stop_generator = 1;
}

int main() {
    signal(SIGINT, handle_signal);
    random_device rd;
    mt19937 gen(rd());
    
    uniform_int_distribution<> first_number(MIN_VALUE, static_cast<int>(MAX_VALUE * MAX_START_PERCENTAGE));
    int previous_number = first_number(gen);
    
    
    while (!stop_generator) {
        double lower_limit = previous_number * (1.0 - VARIATION_PERCENTAGE);
        double upper_limit = previous_number * (1.0 + VARIATION_PERCENTAGE);
        
        // Limit values (ensure they stay within 1-200000)
        lower_limit = max(static_cast<double>(MIN_VALUE), lower_limit);
        upper_limit = min(static_cast<double>(MAX_VALUE), upper_limit);
        
        // Create uniform distribution within this range
        uniform_real_distribution<> range_distribution(lower_limit, upper_limit);
        
        // Generate new number and round to integer
        int new_number = round(range_distribution(gen));
        
        // Get current time
        auto now = chrono::system_clock::now();
        auto epoch_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        cout << epoch_ms << " " << new_number << endl;
        
        // Store new number for next iteration
        previous_number = new_number;
        
        // Sleep for 2 seconds
        this_thread::sleep_for(SLEEP_DURATION);
    }
    
    return 0;
}
