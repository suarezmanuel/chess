#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdlib>
#include <chrono>

namespace fs = std::filesystem;

// Function to shuffle the color palette
void shuffle_colors(std::vector<std::string>& colors) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(colors.begin(), colors.end(), g);
}

// Function to extract the plot title from filename
std::string extract_title(const std::string& filename) {
    // Assuming the filename format is data_<Title>.txt
    size_t prefix_len = std::string("data_").length();
    size_t suffix_pos = filename.find(".txt");
    if (filename.substr(0, prefix_len) != "data_" || suffix_pos == std::string::npos) {
        return "";
    }
    std::string title = filename.substr(prefix_len, suffix_pos - prefix_len);
    // Replace underscores with spaces for better readability
    std::replace(title.begin(), title.end(), '_', ' ');
    return title;
}

// Function to get current date and time in YYYY-MM-DD_HH-MM-SS format
std::string get_current_datetime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time);
#else
    localtime_r(&now_time, &now_tm);
#endif
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &now_tm);
    return std::string(buffer);
}

int main() {
    // Directories
    std::string records_dir = "records";
    std::string plots_dir = "plots";
    std::string resources_dir = "../../public/resources";

    // Create necessary directories if they don't exist
    fs::create_directories(plots_dir);
    fs::create_directories(resources_dir);

    // Define the color palette
    std::vector<std::string> color_palette = {
        "#6A5ACD",  // Slate Blue
        "#808000",  // Olive Green
        "#008080",  // Teal
        "#2E8B57",  // Sea Green
        "#708090",  // Slate Gray
        "#CD853F",  // Peru
        "#9370DB",  // Medium Purple
        "#2F4F4F",  // Dark Slate Gray
        "#FF8C00",  // Dark Orange
        "#20B2AA",  // Light Sea Green
        "#556B2F",  // Dark Olive Green
        "#8B4513",  // Saddle Brown
        "#4682B4",  // Steel Blue
        "#B8860B",  // Dark Goldenrod
        "#5F9EA0",  // Cadet Blue
        "#9ACD32",  // Yellow Green
        "#D2691E",  // Chocolate
        "#6495ED",  // Cornflower Blue
        "#DC143C",  // Crimson
        "#696969"   // Dim Gray
        // Add more colors if needed
    };

    // Shuffle the color palette for random assignment
    shuffle_colors(color_palette);

    // Collect all data_*.txt files in the records directory
    std::vector<fs::path> data_files;
    for (const auto& entry : fs::directory_iterator(records_dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("data_") == 0 && filename.substr(filename.size() - 4) == ".txt") {
                data_files.push_back(entry.path());
            }
        }
    }

    if (data_files.empty()) {
        std::cerr << "No data_*.txt files found in the '" << records_dir << "' directory.\n";
        return 1;
    }

    // Assign a unique color to each data file
    std::map<std::string, std::string> file_color_map;
    size_t color_count = color_palette.size();
    size_t file_count = data_files.size();

    if (file_count > color_count) {
        std::cerr << "Warning: Number of data files (" << file_count << ") exceeds the number of available colors (" << color_count << "). Colors will be reused.\n";
    }

    for (size_t i = 0; i < data_files.size(); ++i) {
        std::string filename = data_files[i].filename().string();
        std::string title = extract_title(filename);
        if (title.empty()) {
            std::cerr << "Filename '" << filename << "' does not conform to the expected format. Skipping.\n";
            continue;
        }
        std::string color = color_palette[i % color_count];
        file_color_map[filename] = color;
    }

    // Generate Gnuplot script
    std::string gnuplot_script = "set terminal pngcairo size 800,600 enhanced font 'Verdana,10'\n";
    gnuplot_script += "set output 'plots/plot_generated.png'\n";
    gnuplot_script += "set xlabel 'Depth'\n";
    gnuplot_script += "set ylabel 'Nodes'\n";
    gnuplot_script += "set title 'Negamax and Perft Performance'\n";
    gnuplot_script += "set grid\n";
    gnuplot_script += "set logscale y\n"; // Apply logarithmic scaling to the y-axis
    gnuplot_script += "set format y '10^{%L}'\n"; // Format y-axis labels as powers of 10

    // Start the plot command
    gnuplot_script += "plot ";

    bool first = true;
    for (const auto& [filename, color] : file_color_map) {
        std::string filepath = "./records/" + filename;
        std::string title = extract_title(filename);

        // Escape double quotes in title
        size_t pos = 0;
        while ((pos = title.find('"', pos)) != std::string::npos) {
            title.insert(pos, "\\");
            pos += 2;
        }

        if (!first) {
            gnuplot_script += ", \\\n     ";
        }
        first = false;

        gnuplot_script += "\"" + filepath + "\" using 1:2 with linespoints lw 2 lc rgb \"" + color + "\" title \"" + title + "\"";
    }

    gnuplot_script += "\n";

    // Write the Gnuplot script to a file
    std::string gnuplot_script_path = "plots/plot_generated.gnu";
    std::ofstream script_file(gnuplot_script_path);
    if (!script_file.is_open()) {
        std::cerr << "Failed to create Gnuplot script at '" << gnuplot_script_path << "'.\n";
        return 1;
    }
    script_file << gnuplot_script;
    script_file.close();

    // Execute Gnuplot with the generated script
    std::string gnuplot_command = "gnuplot plots/plot_generated.gnu";
    int ret = std::system(gnuplot_command.c_str());
    if (ret != 0) {
        std::cerr << "Gnuplot execution failed. Please ensure Gnuplot is installed and accessible from the command line.\n";
        return 1;
    }

    std::cout << "Gnuplot script executed successfully. Plot saved as 'plots/plot_generated.png'.\n";

    // Optionally, copy the latest plot to 'public/resources/comparison.png'
    std::string destination_plot = resources_dir + "/comparison.png";
    try {
        fs::copy_file("plots/plot_generated.png", destination_plot, fs::copy_options::overwrite_existing);
        std::cout << "Latest comparison plot copied to '" << destination_plot << "'.\n";
    } catch (fs::filesystem_error& e) {
        std::cerr << "Failed to copy plot: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
