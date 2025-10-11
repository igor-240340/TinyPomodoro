#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string>
#include <future>
#include <format>

#ifdef __linux__
#include <termios.h>
#endif

#ifdef _WIN32
#include <conio.h>
#endif

#include "bass.h"

constexpr int default_time_minutes = 25;
std::atomic<bool> timer_stopped{ false };

void log_time(const std::string& foldername, int minutes_elapsed);
std::string get_current_date_string();
int timer_job(int minutes);
void input_job();

int main(int argc, char* argv[]) {
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		std::cout << "BASS_Init failed\n";
		return 1;
	}

	std::string sound_filename = "bell-ding.mp3";
	HSTREAM stream = BASS_StreamCreateFile(FALSE, sound_filename.c_str(), 0, 0, 0);
	if (!stream) {
		std::cout << "BASS_StreamCreateFile failed " << sound_filename << std::endl;
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
			std::cout << "Invalid input for minutes, using default of 25.\n";
		}
	}

	std::thread input_thread(input_job);
	std::future<int> timer_job_res = std::async(std::launch::async, timer_job, minutes);
	int minutes_elapsed = timer_job_res.get();
	input_thread.join();

	// "TinyPomodoro.exe -nolog" or
	// "TinyPomodoro.exe -nolog -m <minutes>".
	bool nolog_passed = (argc > 1 && std::string(argv[1]) == "-nolog") ? true : false;
	// "TinyPomodoro.exe -m <minutes> -nolog".
	nolog_passed = (argc == 4 && std::string(argv[3]) == "-nolog") ? true : nolog_passed;
	if (!nolog_passed) {
		if (!std::filesystem::exists("log")) {
			std::filesystem::create_directory("log");
		}
		log_time("log", minutes_elapsed);
	}

	if (!BASS_ChannelPlay(stream, FALSE)) {
		std::cout << "BASS: Can't play stream.\n";
		BASS_StreamFree(stream);
		BASS_Free();
		return 1;
	}

	std::cout << "\nWaiting for BASS finishing playing sound...\n";
	while (BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING)	{}

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

		std::string t;
		file >> t >> t >> prev_hours_elapsed;
	}

	std::ofstream file(filename, std::ios::trunc);
	file << "Total time: " << (prev_hours_elapsed + hours_elapsed) << " hour(s).\n";
}

std::string get_current_date_string() {
	const auto now = std::chrono::system_clock::now();
	const auto now_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm local_time = *std::localtime(&now_time_t);

	return std::format("{:02}-{:02}-{}",
					   local_time.tm_mday,
					   local_time.tm_mon + 1,
					   local_time.tm_year + 1900);
}

int timer_job(int minutes) {
	std::cout << "Timer started for " << minutes << " minute(s)...\n";
	std::cout << "Press 'Esc' to exit (elapsed minutes will be counted if -nolog hasn't been specified)\n";

	auto start = std::chrono::steady_clock::now();
	auto end = start + std::chrono::minutes(minutes);
	int minutes_elapsed = 0;
	while (std::chrono::steady_clock::now() < end && !timer_stopped) {
		auto now = std::chrono::steady_clock::now();
		auto remaining_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - now);

		int minutes_left = remaining_seconds.count() / 60;
		int seconds_left = remaining_seconds.count() % 60;

		minutes_elapsed = (std::chrono::seconds(minutes * 60) - remaining_seconds).count() / 60;

		std::cout << "\033[?25l"; // Hide cursor.
		std::cout << "\rRemaining time: ";
		std::cout
			<< std::setw(2) << std::setfill('0') << minutes_left
			<< ":" << std::setw(2) << std::setfill('0') << seconds_left
			<< std::flush;
		//std::cout << "\033[?25h"; // Show cursor.
	}

	// Tell input_job that timer has stopped naturally.
	timer_stopped = true;

	return minutes_elapsed;
}

#ifdef __linux__
void input_job() {
	termios prev_attr, new_attr;
    tcgetattr(STDIN_FILENO, &prev_attr);
    new_attr = prev_attr;
    new_attr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);

    while (!timer_stopped) {
        int c = getchar();
        if (c == 0x1b) { // Esc.
			timer_stopped = true;
			break;
		}
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &prev_attr);
}
#endif

#ifdef _WIN32
void input_job() {
	while (!timer_stopped) {
		if (_kbhit()) {
			char c = _getch();
			if (c == 0x1b) { // Esc.
				timer_stopped = true; // Tell timer_job that timer has been stopped by user.
				break;
			}
		}
	}
}
#endif
