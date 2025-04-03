#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <thread>

class Timer
{
public:
    void setFrequency(double frequency_hz)
    {
        // Set the desired frequency
        frequency_hz_ = frequency_hz;
        // Calculate the duration between iterations based on the desired frequency
        iteration_duration_ = std::chrono::duration<double>(1.0 / frequency_hz_);
    }

    void start()
    {
        // Start the timer
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    void wait()
    {
        // Wait until the next iteration time
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = current_time - start_time_;

        if (elapsed_time < iteration_duration_)
        {
            // Sleep to maintain the fixed frequency
            std::this_thread::sleep_for(iteration_duration_ - elapsed_time);
        }

        // Update the start time for the next iteration
        start_time_ = current_time;
    }

private:
    double frequency_hz_;                              // Desired frequency in hertz
    std::chrono::duration<double> iteration_duration_; // Duration between iterations
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};

#endif