#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <ctime>
#include <sstream>
#include <string>
#include <filesystem>

#include "bass.h"

constexpr int default_time_minutes = 25;

void log_time(const std::string& foldername, int minutes_elapsed);
std::string get_current_date_string();

int main(int argc, char* argv[]) {
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        std::cerr << "BASS: Can't initialize device.\n";
        return 1;
    }

    const std::string sound_filename = "bell-ding.mp3";
    HSTREAM stream = BASS_StreamCreateFile(FALSE, sound_filename.c_str(), 0, 0, 0);
    if (!stream) {
        std::cerr << "BASS: Can't stream the file: " << sound_filename << std::endl;
        BASS_Free();
        return 1;
    }

    // "TinyPomodoro.exe -m <minutes>" or
    // "TinyPomodoro.exe -m <minutes> -nolog".
    int minutes_index = (argc > 2 && std::string(argv[1]) == "-m") ? 2 : -1;
    // "TinyPomodoro.exe -nolog -m <minutes>".
    minutes_index = (argc == 4 && std::string(argv[2]) == "-m") ? 3 : minutes_index;

    int minutes = default_time_minutes;
    if (minutes_index > 0) {
        try {
            minutes = std::stoi(argv[minutes_index]);
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid input for minutes, using default of 25.\n";
        }
    }

    std::cout << "Timer started for " << minutes << " minute(s)...\n";
    std::this_thread::sleep_for(std::chrono::minutes(minutes));
    std::cout << "Time is up!" << std::endl;

    // "TinyPomodoro.exe -nolog" or
    // "TinyPomodoro.exe -nolog -m <minutes>".
    bool nolog_passed = (argc > 1 && std::string(argv[1]) == "-nolog") ? true : false;
    // "TinyPomodoro.exe -m <minutes> -nolog".
    nolog_passed = (argc == 4 && std::string(argv[3]) == "-nolog") ? true : nolog_passed;
    if (!nolog_passed) {
        if (!std::filesystem::exists("log")) {
            std::filesystem::create_directory("log");
        }
        log_time("log", minutes);
    }

    if (!BASS_ChannelPlay(stream, FALSE)) {
        std::cerr << "BASS: Can't play stream.\n";
        BASS_StreamFree(stream);
        BASS_Free();
        return 1;
    }

    std::cout << "\nPress Enter to exit.\n";
    std::cin.get();

    BASS_StreamFree(stream);
    BASS_Free();

    return 0;
}

void log_time(const std::string& foldername, int minutes_elapsed) {
    const auto current_date_string = get_current_date_string();
    const auto filename = foldername + "/" + current_date_string + ".txt";
    const double hours_elapsed = minutes_elapsed / 60.0;

    double prev_hours_elapsed = 0.0;
    if (std::filesystem::exists(filename)) {
        std::ifstream file(filename);
        file >> prev_hours_elapsed;
    }

    std::ofstream file(filename, std::ios::trunc);
    file << (prev_hours_elapsed + hours_elapsed) << " hour(s).\n";
}

std::string get_current_date_string() {
    const auto now = std::chrono::system_clock::now();
    const auto now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm local_time;
    localtime_s(&local_time, &now_time_t);

    std::ostringstream oss;
    oss << local_time.tm_mday << '-'
        << (local_time.tm_mon + 1) << '-'
        << (local_time.tm_year + 1900);
    return oss.str();
}