#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <map>
#include <filesystem>
#include <csignal> 
#include <SFML/Audio.hpp>

// Headers for different operating systems
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

//Global flags for Signal Handling
volatile sig_atomic_t running = true;
volatile sig_atomic_t resized = false;


// --- Type Aliases for Readability
using high_res_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<high_res_clock>;
using duration_ms = std::chrono::milliseconds;
using duration_sec = std::chrono::duration<double>;


// --- Default Configuration for a Calmer Storm
const float DEFAULT_LIGHTNING_CHANCE = 0.05f; //%chance per frame
const int FORK_CHANCE = 13;                   //lightning branching
const int SUBFORK_CHANCE = 5;                 //lightning branching of branching (nested branching :)
const duration_ms FRAME_DELAY(45);            //~22fps increasing it will make animation slower
const duration_ms LIGHTNING_GROWTH_DELAY(5);  //growth pacing
const duration_sec SEGMENT_LIFESPAN(0.9);     //bolt lifetime
const std::vector<char> LIGHTNING_CHARS = {'*', '+', '#'}; //different character as lightning lifetime varies 


//Represent Lightning Bolts
struct LightningSegment {
    int x, y;
    time_point creation_time;
};


class LightningBolt {
private:
    std::vector<LightningSegment> segments;
    time_point last_growth_time;
    bool is_growing = true;
    int max_y, max_x;
    size_t target_length;

public:
    LightningBolt(int start_x, int start_y, int term_width, int term_height, std::mt19937& generator) {
        max_x = term_width;
        max_y = term_height;
        last_growth_time = high_res_clock::now();
        std::uniform_int_distribution<int> length_dist(max_y / 2, max_y - 2);
        target_length = length_dist(generator);
        segments.push_back({start_x, start_y, high_res_clock::now()});
    }

    bool update(std::mt19937& generator) {
        auto current_time = high_res_clock::now();
        if (is_growing && (current_time - last_growth_time) > LIGHTNING_GROWTH_DELAY) {
            last_growth_time = current_time;
            if (segments.empty() || segments.size() >= target_length) {
                is_growing = false;
            } else {
                LightningSegment last_segment = segments.back();
                if (last_segment.y >= max_y - 1) {
                    is_growing = false;
                } else {
                    std::uniform_int_distribution<int> dir_dist(-1, 1);
                    int direction = dir_dist(generator);
                    int next_x = std::max(1, std::min(max_x - 1, last_segment.x + direction));
                    segments.push_back({next_x, last_segment.y + 1, current_time});
                    std::uniform_int_distribution<int> fork_check(0, 100);
                    if (fork_check(generator) < FORK_CHANCE) {
                         addFork(last_segment, generator);
                    }
                }
            }
        }
        if (!is_growing) {
            bool all_expired = true;
            for(const auto& seg : segments) {
                if (duration_sec(current_time - seg.creation_time) < SEGMENT_LIFESPAN) {
                    all_expired = false;
                    break;
                }
            }
            if (all_expired) return false;
        }
        return true;
    }

    void draw(const char* bright_color) {
        auto current_time = high_res_clock::now();
        for (const auto& seg : segments) {
            duration_sec age = current_time - seg.creation_time;
            if (age < SEGMENT_LIFESPAN) {
                double age_ratio = age.count() / SEGMENT_LIFESPAN.count();
                char character;
                const char* color_code;
                if (age_ratio < 0.33) {
                    character = LIGHTNING_CHARS[2];
                    color_code = bright_color;
                } else if (age_ratio < 0.66) {
                    character = LIGHTNING_CHARS[1];
                    color_code = bright_color;
                } else {
                    character = LIGHTNING_CHARS[0];
                    color_code = "\033[90m";
                }
                std::cout << color_code << "\033[" << seg.y << ";" << seg.x << "H" << character;
            }
        }
    }

private:
    void addFork(LightningSegment start_point, std::mt19937& generator) {
        std::uniform_int_distribution<int> dir_dist(-1, 1);
        std::uniform_int_distribution<int> len_dist(max_y / 6, max_y / 3);
        int fork_len = len_dist(generator);
        for (int i = 0; i < fork_len && start_point.y < max_y -1; ++i) {
            int direction = dir_dist(generator);
            start_point.x = std::max(1, std::min(max_x - 1, start_point.x + direction));
            start_point.y++;
            start_point.creation_time = high_res_clock::now();
            segments.push_back(start_point);
            std::uniform_int_distribution<int> subfork_check(0, 100);
            if (subfork_check(generator) < SUBFORK_CHANCE) {
                addFork(start_point, generator);
            }
        }
    }
};



void getTerminalSize(int& width, int& height);
void printHelp(const char* program_name);
std::string find_asset_path(const std::string& asset);
void signal_handler(int signum);


int main(int argc, char* argv[]) {
    // Register Signal Handlers
    #ifndef _WIN32
        signal(SIGINT, signal_handler);
        signal(SIGWINCH, signal_handler);
    #endif

    // Color Maps
    const std::map<std::string, const char*> color_map = {
        {"black", "\033[30m"}, {"red", "\033[31m"}, {"green", "\033[32m"},
        {"yellow", "\033[33m"}, {"blue", "\033[34m"}, {"magenta", "\033[35m"},
        {"cyan", "\033[36m"}, {"white", "\033[37m"}
    };
    const std::map<std::string, const char*> bright_color_map = {
        {"black", "\033[90m"}, {"red", "\033[91m"}, {"green", "\033[92m"},
        {"yellow", "\033[93m"}, {"blue", "\033[94m"}, {"magenta", "\033[95m"},
        {"cyan", "\033[96m"}, {"white", "\033[97m"}
    };

    // Argument Parsing
    std::string rain_color_name = "blue";
    std::string lightning_color_name = "yellow";
    float thunder_volume = 30.0f;
    float lightning_chance = DEFAULT_LIGHTNING_CHANCE; // Use default from config

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") { printHelp(argv[0]); return 0; }
        else if (arg == "--rain-color" && i + 1 < argc) { rain_color_name = argv[++i]; }
        else if (arg == "--lightning-color" && i + 1 < argc) { lightning_color_name = argv[++i]; }
        else if (arg == "--thunder-volume" && i + 1 < argc) {
            try { thunder_volume = std::stof(argv[++i]); }
            catch (const std::exception& e) { std::cerr << "Invalid volume. Please provide a number." << std::endl; return 1; }
        }
        else if (arg == "--lightning-chance" && i + 1 < argc) {
            try { lightning_chance = std::stof(argv[++i]); }
            catch (const std::exception& e) { std::cerr << "Invalid chance value. Please provide a number." << std::endl; return 1; }
        }
    }
    if (color_map.find(rain_color_name) == color_map.end() || bright_color_map.find(lightning_color_name) == bright_color_map.end()) {
        std::cerr << "Invalid color specified." << std::endl; printHelp(argv[0]); return 1;
    }
    const char* rain_color_code = color_map.at(rain_color_name);
    const char* lightning_color_code = bright_color_map.at(lightning_color_name);

    // Main Setup
    int width, height;
    getTerminalSize(width, height);

    std::string rain_path = find_asset_path("sounds/rain.wav");
    std::string thunder_path = find_asset_path("sounds/thunder.wav");
    if (rain_path.empty() || thunder_path.empty()) { std::cerr << "Error: Could not find sound assets." << std::endl; return 1; }

    sf::Music rainMusic;
    if (!rainMusic.openFromFile(rain_path)) { std::cerr << "Error loading rain.wav" << std::endl; return -1; }
    rainMusic.setLooping(true);
    rainMusic.setVolume(50.f);
    rainMusic.play();

    sf::SoundBuffer thunderBuffer;
    if (!thunderBuffer.loadFromFile(thunder_path)) { std::cerr << "Error loading thunder.wav" << std::endl; return -1; }
    sf::Sound thunderSound(thunderBuffer);
    thunderSound.setVolume(thunder_volume);

    std::vector<int> drops(width, -1);
    std::vector<LightningBolt> active_bolts;
    std::mt19937 generator(high_res_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> drop_dist(0, width);
    std::uniform_real_distribution<float> lightning_dist(0.0f, 100.0f);
    std::uniform_int_distribution<int> lightning_pos_dist(width / 4, width * 3 / 4);
    
    std::cout << "\033[?25l"; // Hide cursor

    // Main Loop
    while (running) {
        // Handle Window Resizing
        #ifdef _WIN32
            int old_width = width, old_height = height;
            getTerminalSize(width, height);
            if (width != old_width || height != old_height) { resized = true; }
        #endif
        if (resized) {
            #ifndef _WIN32
                getTerminalSize(width, height);
            #endif
            active_bolts.clear();
            drops.assign(width, -1);
            std::cout << "\033[2J";
            resized = false;
        }

        // Update Phase
        if (lightning_dist(generator) < lightning_chance) { // Use variable from args
            if (active_bolts.size() < 3) {
                thunderSound.play();
                active_bolts.emplace_back(lightning_pos_dist(generator), 1, width, height, generator);
            }
        }
        std::vector<LightningBolt> next_bolts;
        for (auto& bolt : active_bolts) {
            if (bolt.update(generator)) { next_bolts.push_back(std::move(bolt)); }
        }
        active_bolts = std::move(next_bolts);

        // Drawing Phase
        std::cout << "\033[H";
        std::cout << rain_color_code;
        for (int i = 0; i < width; ++i) {
            if (drops[i] == -1 && drop_dist(generator) < 2) drops[i] = 1;
        }
        for (int j = 1; j <= height; ++j) {
            for (int i = 0; i < width; ++i) {
                std::cout << (drops[i] == j ? "|" : " ");
            }
            if (j < height) std::cout << std::endl;
        }
        for (int i = 0; i < width; ++i) {
            if (drops[i] != -1) {
                drops[i]++;
                if (drops[i] > height) drops[i] = -1;
            }
        }
        for (auto& bolt : active_bolts) { bolt.draw(lightning_color_code); }

        std::cout.flush();
        std::this_thread::sleep_for(FRAME_DELAY);
    }

    // Graceful Exit Cleanup
    std::cout << "\033[?25h\033[0m\033[2J\033[H";
    return 0;
}

void getTerminalSize(int& width, int& height) {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;
    #endif
}

void printHelp(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  --rain-color <color>       Set the color of the rain.\n"
              << "  --lightning-color <color>  Set the color of the lightning.\n"
              << "  --thunder-volume <0-100>   Set the volume of the thunder.\n"
              << "  --lightning-chance <%>     Percentage chance of lightning per frame (e.g., 0.5).\n"
              << "  --help                     Show this help message.\n\n"
              << "Available colors: black, red, green, yellow, blue, magenta, cyan, white\n";
}

std::string find_asset_path(const std::string& asset) {
    if (std::filesystem::exists(asset)) {
        return asset;
    }
    #ifdef DATA_DIR
        std::string installed_path = std::string(DATA_DIR) + "/" + asset;
        if (std::filesystem::exists(installed_path)) {
            return installed_path;
        }
    #endif
    return "";
}

void signal_handler(int signum) {
    if (signum == SIGINT) {
        running = false;
    } else if (signum == SIGWINCH) {
        resized = true;
    }
}

