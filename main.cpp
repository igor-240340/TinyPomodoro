#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <ctime>
#include <sstream>
#include <string>
#include <filesystem>
#include <optional>

#include "bass.h"

constexpr int default_time_minutes = 25;

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

std::optional<std::pair<std::string, double>> read_last_entry(const std::string& filename) {
    std::ifstream file(filename);
    std::string last_line, line;

    while (std::getline(file, line)) {
        last_line = line;
    }

    if (!last_line.empty()) {
        std::istringstream iss(last_line);

        std::string date;
        double hours_elapsed;
        if (iss >> date >> hours_elapsed) {
            return std::make_pair(date, hours_elapsed);
        }
    }

    return std::nullopt;
}

void log_time(const std::string& foldername, int minutes_elapsed) {
    const std::string current_date_string = get_current_date_string();
    const std::string filename = foldername + "/" + current_date_string + ".txt";
    const double hours_elapsed = minutes_elapsed / 60.0;

    double prev_hours_elapsed = 0.0;
    if (std::filesystem::exists(filename)) {
        std::ifstream file(filename);
        file >> prev_hours_elapsed;
    }

    std::ofstream file(filename, std::ios::trunc);
    file << (prev_hours_elapsed + hours_elapsed) << " hour(s).\n";
}

int main(int argc, char* argv[]) {
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        std::cerr << "BASS: Can't initialize device." << std::endl;
        return 1;
    }

    const std::string sound_filename = "bell-ding.mp3";
    HSTREAM stream = BASS_StreamCreateFile(FALSE, sound_filename.c_str(), 0, 0, 0);
    if (!stream) {
        std::cerr << "BASS: Can't stream the file: " << sound_filename << std::endl;
        BASS_Free();
        return 1;
    }

    // TinyPomodoro.exe -m <minutes> or 
    // TinyPomodoro.exe -m <minutes> -nolog.
    const bool minutes_passed_1 = (argc > 2 && std::string(argv[1]) == "-m");
    // TinyPomodoro.exe -nolog -m <minutes>
    const bool minutes_passed_2 = (argc == 4 && std::string(argv[2]) == "-m");
    int minutes_index = -1;
    if (minutes_passed_1)
        minutes_index = 2;
    else if (minutes_passed_2)
        minutes_index = 3;

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
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Time is up!" << std::endl;

    const bool nolog_passed_1 = (argc > 1 && std::string(argv[1]) == "-nolog");     // TinyPomodoro.exe -nolog or TinyPomodoro.exe -nolog -m <minutes>.
    const bool nolog_passed_2 = (argc == 4 && std::string(argv[3]) == "-nolog");    // TinyPomodoro.exe -m <minutes> -nolog.
    if (!(nolog_passed_1 || nolog_passed_2)) {
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
