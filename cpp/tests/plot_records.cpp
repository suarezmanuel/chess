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

// Function to extract depth from filename
int extract_depth(const std::string& filename) {
    // Assuming the filename format is prefix_depth_timestamp.csv
    // Split by '_' and extract the second part as depth
    size_t first_underscore = filename.find('_');
    if (first_underscore == std::string::npos) return -1;
    size_t second_underscore = filename.find('_', first_underscore + 1);
    if (second_underscore == std::string::npos) return -1;
    std::string depth_str = filename.substr(first_underscore + 1, second_underscore - first_underscore - 1);
    try {
        return std::stoi(depth_str);
    } catch (...) {
        return -1;
    }
}

// Function to extract prefix from filename
std::string extract_prefix(const std::string& filename) {
    size_t first_underscore = filename.find('_');
    if (first_underscore == std::string::npos) return "";
    return filename.substr(0, first_underscore);
}

// Function to get current date in YYYY-MM-DD format
std::string get_current_date() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        #ifdef _WIN32
            localtime_s(&now_tm, &now_time);
        #else
             localtime_r(&now_time, &now_tm);
        #endif

        char time_buffer[20];
        std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", &now_tm);
        std::string timestamp(time_buffer);
        return timestamp;
}

// Function to escape underscores and other special characters for Gnuplot
std::string escape_for_gnuplot(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '_') {
            output += "\\_";  // Insert a backslash before the underscore
        } else {
            output += c;
        }
    }
    return output;
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
        "#6A5ACD",  // 1 - Slate Blue
        "#808000",  // 2 - Olive Green
        "#008080",  // 3 - Teal
        "#2E8B57",  // 4 - Sea Green
        "#708090",  // 5 - Slate Gray
        "#CD853F",  // 6 - Peru
        "#9370DB",  // 7 - Medium Purple
        "#2F4F4F",  // 8 - Dark Slate Gray
        "#FF8C00",  // 9 - Dark Orange
        "#20B2AA",  // 10 - Light Sea Green
        "#556B2F",  // 11 - Dark Olive Green
        "#8B4513",  // 12 - Saddle Brown
        "#4682B4",  // 13 - Steel Blue
        "#B8860B",  // 14 - Dark Goldenrod
        "#5F9EA0",  // 15 - Cadet Blue
        "#9ACD32",  // 16 - Yellow Green
        "#D2691E",  // 17 - Chocolate
        "#6495ED",  // 18 - Cornflower Blue
        "#DC143C",  // 19 - Crimson
        "#696969"   // 20 - Dim Gray
        // Add more colors if needed
    };

    // Shuffle the color palette for random assignment
    shuffle_colors(color_palette);

    // Collect all CSV files in the records directory
    std::vector<fs::path> csv_files;
    for (const auto& entry : fs::directory_iterator(records_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            csv_files.push_back(entry.path());
        }
    }

    if (csv_files.empty()) {
        std::cerr << "No CSV files found in the '" << records_dir << "' directory.\n";
        return 1;
    }

    // Extract unique prefixes
    std::vector<std::string> prefixes;
    for (const auto& file : csv_files) {
        std::string filename = file.filename().string();
        std::string prefix = extract_prefix(filename);
        if (!prefix.empty() && std::find(prefixes.begin(), prefixes.end(), prefix) == prefixes.end()) {
            prefixes.push_back(prefix);
        }
    }

    // Assign a unique color to each prefix
    std::map<std::string, std::string> prefix_color_map;
    size_t color_count = color_palette.size();
    size_t prefix_count = prefixes.size();

    if (prefix_count > color_count) {
        std::cerr << "Warning: Number of prefixes (" << prefix_count << ") exceeds the number of available colors (" << color_count << "). Colors will be reused.\n";
    }

    for (size_t i = 0; i < prefixes.size(); ++i) {
        std::string color;
        if (i < color_count) {
            color = color_palette[i];
        } else {
            // Cycle through the color palette if out of unique colors
            color = color_palette[i % color_count];
        }
        prefix_color_map[prefixes[i]] = color;
    }

    // Print the prefix-color mapping for debugging
    std::cout << "Prefix to Color Mapping:\n";
    for (const auto& [prefix, color] : prefix_color_map) {
        std::cout << "Prefix '" << prefix << "' -> Color '" << color << "'\n";
    }

    // Extract unique depths from filenames
    std::vector<int> depths;
    for (const auto& file : csv_files) {
        int depth = extract_depth(file.filename().string());
        if (depth != -1 && std::find(depths.begin(), depths.end(), depth) == depths.end()) {
            depths.push_back(depth);
        }
    }

    if (depths.empty()) {
        std::cerr << "No valid depths found in the CSV filenames.\n";
        return 1;
    }

    // Sort depths in ascending order
    std::sort(depths.begin(), depths.end());

    // Determine the layout for multiplot (up to 3 graphs per row)
    int graphs_per_row = 3;
    int total_depths = depths.size();
    int rows = (total_depths + graphs_per_row - 1) / graphs_per_row;

    // Get the current date for the output filename
    std::string current_date = get_current_date();

    // Define the output plot filename
    std::string output_plot = plots_dir + "/comparison_" + current_date + ".png";

    // Start building the Gnuplot script
    std::string gnuplot_script = "set datafile separator \",\"\n";
    gnuplot_script += "set terminal png size " + std::to_string(graphs_per_row * 1200) + "," + std::to_string(rows * 800) + " enhanced font 'Arial,10'\n";
    gnuplot_script += "set output '" + output_plot + "'\n";
    gnuplot_script += "set multiplot layout " + std::to_string(rows) + "," + std::to_string(graphs_per_row) + " title 'Perft Performance Comparison Across Depths'\n";

    // Iterate over each depth and construct plot commands
    for (const auto& depth : depths) {
        std::cout << "Processing Depth: " << depth << "\n";

        // Collect files for the current depth
        std::vector<fs::path> depth_files;
        for (const auto& file : csv_files) {
            int file_depth = extract_depth(file.filename().string());
            if (file_depth == depth) {
                depth_files.push_back(file);
            }
        }

        if (depth_files.empty()) {
            std::cerr << "No CSV files found for depth '" << depth << "'. Skipping.\n";
            continue;
        }

        // Start constructing the plot command for the current depth
        std::string plot_command = "";

        for (size_t i = 0; i < depth_files.size(); ++i) {
            std::string filepath = depth_files[i].string();
            std::string filename = depth_files[i].filename().string();

            // Extract prefix to get the assigned color
            std::string prefix = extract_prefix(filename);
            if (prefix.empty()) {
                std::cerr << "Filename '" << filename << "' does not have a valid prefix. Skipping.\n";
                continue;
            }

            std::string color = prefix_color_map[prefix];

            // Extract timestamp for the title
            // Assuming timestamp is everything after the second underscore and before .csv
            size_t first_underscore = filename.find('_');
            size_t second_underscore = filename.find('_', first_underscore + 1);
            if (first_underscore == std::string::npos || second_underscore == std::string::npos) {
                std::cerr << "Filename '" << filename << "' does not conform to the expected format.\n";
                continue;
            }
            std::string timestamp = filename.substr(second_underscore + 1, filename.size() - second_underscore - 6); // Remove .csv

            // Escape underscores and other special characters
            std::string escaped_timestamp = escape_for_gnuplot(timestamp);

            // Append to plot commands
            if (i != 0) {
                plot_command += ", ";
            }
            plot_command += "\"" + filepath + "\" using 1:2 with lines lw 2 lc rgb \"" + color + "\" title \"" + filename + "\"";
        }

        // Append the plot command for the current depth
        gnuplot_script += "set title 'Depth " + std::to_string(depth) + "'\n";
        gnuplot_script += "plot " + plot_command + "\n";
    }

    // End multiplot
    gnuplot_script += "unset multiplot\n";

    // Write the Gnuplot script to a temporary file
    std::string temp_script = "temp_plot.gp";
    std::ofstream script_file(temp_script);
    if (!script_file.is_open()) {
        std::cerr << "Failed to create temporary Gnuplot script.\n";
        return 1;
    }
    script_file << gnuplot_script;
    script_file.close();

    // Execute Gnuplot with the generated script
    std::string gnuplot_command = "gnuplot " + temp_script;
    int ret = std::system(gnuplot_command.c_str());
    if (ret != 0) {
        std::cerr << "Gnuplot execution failed.\n";
        return 1;
    }

    std::cout << "Multigraph comparison plot saved to '" << output_plot << "'.\n";

    // Copy the latest plot to 'public/resources/comparison.png'
    std::string destination_plot = resources_dir + "/comparison.png";
    try {
        fs::copy_file(output_plot, destination_plot, fs::copy_options::overwrite_existing);
        std::cout << "Latest comparison plot copied to '" << destination_plot << "'.\n";
    } catch (fs::filesystem_error& e) {
        std::cerr << "Failed to copy plot: " << e.what() << "\n";
        return 1;
    }

    // Optionally, remove the temporary Gnuplot script
    fs::remove(temp_script);

    return 0;
}
