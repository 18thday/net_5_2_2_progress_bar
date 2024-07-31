#include <atomic>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <Windows.h>

HANDLE h  = GetStdHandle(STD_OUTPUT_HANDLE);
void set_cursor_pos(int16_t x, int16_t y = 1) {
    SetConsoleCursorPosition(h, {x, y});
}

void update_progress_bar(size_t thread_number, int progress_percent, size_t bar_width,
                         std::optional<double> time,
                         std::string fill_sym = "|", std::string remainder_sym = " ") {
    set_cursor_pos(0, thread_number+1);
    std::cout << thread_number << '\t'
              << std::this_thread::get_id() << '\t' << '\t'
              << '[';
    size_t margin = progress_percent * bar_width / 100;
    for (size_t i = 0; i < bar_width; ++i) {
        if (i < margin) {
            std::cout << fill_sym;
        } else {
            std::cout << " ";
        }
    }
    std::cout << ']' << '\t' << progress_percent << '%';

    if (time) {
        std::cout << "\t\t" << time.value() << 's' << '\n';
    } else {
        std::cout << '\n';
    }
}

void calculation(size_t thread_number, size_t calc_length, std::atomic<size_t>& sum, std::mutex& m) {

    size_t bar_width = 20;
    auto start = std::chrono::steady_clock::now();

    int progress_percent = 0 ;
    for (size_t i = 0; i < calc_length; ++i) {
        if (i % 123456 == 0) {
            std::lock_guard<std::mutex> lg(m);
            progress_percent = static_cast<int>(static_cast<double>(i) / calc_length * 100);
            update_progress_bar(thread_number,  progress_percent, bar_width, {});
        }
        ++sum;
        if (i % 92348 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    progress_percent = 100;
    auto finish = std::chrono::steady_clock::now();
    std::chrono::duration<double> dur = finish - start;

    std::lock_guard<std::mutex> lg(m);
    update_progress_bar(thread_number,  progress_percent, bar_width, dur.count());
}

int main() {
    size_t thread_count = 4;
    std::vector<std::thread> thread_vec;
    thread_vec.reserve(thread_count);

    std::cout << "#\tid\t\tprogress bar\t\tpercent\t\ttime\n";
    std::atomic<size_t> sum = 0;
    std::mutex m;
    for (int i = 0; i < thread_count; ++i) {
        thread_vec.push_back(std::thread{calculation, i, 1'000'000, std::ref(sum), std::ref(m)});
    }
    for (auto& t : thread_vec) {
        t.join();
    }
    set_cursor_pos(0, thread_count+1);
    std::cout << "sum  = " << sum << std::endl;
    return 0;
}

