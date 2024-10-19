#!/bin/bash

# Bash script to plot perft records for multiple depths using Gnuplot
# Features:
# - Creates a multi-graph layout with up to 3 graphs per row, each graph comparing all prefixes for a given depth
# - Ensures the same prefix has the same color across all graphs
# - Saves historical plots with date in 'plots' directory
# - Copies the latest plot to 'public/resources/comparison.png' for README display

# Directories
RECORDS_DIR="records"
PLOTS_DIR="plots"
RESOURCES_DIR="../../public/resources"

# Create necessary directories if they don't exist
mkdir -p "$PLOTS_DIR"
mkdir -p "$RESOURCES_DIR"

# Check if Gnuplot is installed
if ! command -v gnuplot &> /dev/null
then
    echo "Gnuplot could not be found. Please install Gnuplot and try again."
    exit 1
fi

# Function to extract unique depths from filenames
extract_depths() {
    find "$RECORDS_DIR" -type f -name '*_*_*.csv' | \
    sed -n 's/.*_\([0-9]\+\)_.*\.csv/\1/p' | \
    sort -n | \
    uniq
}

# Extract unique depths
depths=($(extract_depths))

# Check if any depths were found
if [ ${#depths[@]} -eq 0 ]; then
    echo "No CSV files found matching '*_<depth>_*.csv' in '$RECORDS_DIR' directory."
    exit 1
fi

# Determine the layout for multiplot (up to 3 graphs per row)
graphs_per_row=3
total_depths=${#depths[@]}
rows=$(( (total_depths + graphs_per_row - 1) / graphs_per_row ))

# Get the current date in YYYY-MM-DD format for the output filename
current_date=$(date +"%Y-%m-%d")

# Define the output plot filename with the current date
output_plot="$PLOTS_DIR/comparison_${current_date}.png"

# Start building Gnuplot commands
# Dynamically set the terminal size based on the number of rows and graphs per row
image_width=$((graphs_per_row * 1200))
image_height=$((rows * 800))
gnuplot_commands="set datafile separator \",\"
set terminal png size ${image_width},${image_height} enhanced font 'Arial,10'
set output '$output_plot'
set multiplot layout $rows,$graphs_per_row title 'Perft Performance Comparison Across Depths'
"

# Declare an associative array for prefix colors to ensure consistency
declare -A color_palette

# Function to assign a deterministic color based on prefix name
assign_color() {
    local prefix="$1"
    # Generate an MD5 hash of the prefix, take the first 6 characters for the color
    local hash=$(echo -n "$prefix" | md5sum | head -c6)
    echo "#$hash"
}

# Loop over each depth and construct plot commands
for depth in "${depths[@]}"; do
    echo "Processing Depth: $depth"
    
    # Find all CSV files for the current depth
    matching_files=("$RECORDS_DIR"/*_"$depth"_*.csv)
    
    # Check if there are any matching files for this depth
    if [ ${#matching_files[@]} -eq 0 ]; then
        echo "No CSV files found for depth '$depth'. Skipping."
        continue
    fi
    
    # Extract unique prefixes for the current depth
    declare -A prefixes
    for file in "${matching_files[@]}"; do
        filename=$(basename "$file")
        prefix=${filename%%_"$depth"_*}
        prefixes["$prefix"]=1
    done
    
    # Prepare plot commands for the current depth
    plot_commands=""
    count=0
    
    for prefix in "${!prefixes[@]}"; do
        # Find all files with this prefix and depth
        specific_files=("$RECORDS_DIR"/"${prefix}"_"$depth"_*.csv)
        
        for file in "${specific_files[@]}"; do
            # Extract the timestamp part for the title
            filename=$(basename "$file")
            timestamp=${filename#${prefix}_$depth_}
            timestamp=${timestamp%.csv}
            
            # Escape underscores in timestamp to prevent subscript formatting
            escaped_timestamp=$(echo "$timestamp" | sed 's/_/\\_/g')
            
            # Escape spaces in filename for Gnuplot
            escaped_file=$(echo "$file" | sed 's/ /\\ /g')
            
            # Assign a unique color based on prefix
            if [ -z "${color_palette["$prefix"]}" ]; then
                color=$(assign_color "$prefix")
                color_palette["$prefix"]="$color"
            else
                color="${color_palette["$prefix"]}"
            fi
            
            # Append to plot commands
            if [ $count -gt 0 ]; then
                plot_commands+=", "
            fi
            plot_commands+='"'"$escaped_file"'" using 1:2 with linespoints lc rgb "'"$color"'" title "'"$prefix"': '"$escaped_timestamp"'"'
            count=$((count + 1))
        done
    done
    
    # Add the plot command for the current depth
    gnuplot_commands+="
set title 'Depth $depth'
plot $plot_commands
"
done

# End multiplot
gnuplot_commands+="
unset multiplot
"

# Execute Gnuplot with the constructed commands
gnuplot <<< "$gnuplot_commands"

echo "Multigraph comparison plot saved to '$output_plot'."

# Copy the latest plot to 'public/resources/comparison.png'
cp "$output_plot" "$RESOURCES_DIR/comparison.png"

echo "Latest comparison plot copied to '$RESOURCES_DIR/comparison.png'."
