// CSV IO functions.
// Copyright 2019 Christian Kauten
//

#ifndef CSV
#define CSV

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace CSV {

/// a type for a CSV row
typedef std::vector<std::string> Row;
/// a type for a CSV data-frame
typedef std::vector<Row> DataFrame;

/// Return the next line from a CSV file.
///
/// @param stream the stream to read the CSV line from
/// @returns the next line from the CSV stream as a vector of data items
///
Row _next_line(std::istream& stream) {
    Row result;
    std::string line;
    std::getline(stream, line);
    std::stringstream line_stream(line);
    std::string cell;
    while (std::getline(line_stream, cell, ','))  // iterate over the cells
        result.push_back(cell);
    if (!line_stream && cell.empty())  // trailing comma with no data detected
        // add an empty data element
        result.push_back("");
    return result;
}

/// Read data from a CSV file.
///
/// @param stream the stream with CSV data
/// @returns the CSV data as a matrix of data items
///
DataFrame load(std::istream& stream) {
    DataFrame result;
    std::string line;
    while (getline(stream, line)) {
        auto line_stream = std::istringstream(line);
        result.push_back(_next_line(line_stream));
    }
    return result;
}

/// Print the given CSV file.
///
/// @param csv the CSV file to print to the console
/// @param output the output stream to write the CSV to
///
void print(DataFrame csv, std::ostream& output = std::cout) {
    for (auto row : csv) {  // iterate over each row in the CSV
        for (std::size_t i = 0; i < row.size(); i++) {  // iterate over cells
            if (i == 0)  // first cell in the row
                output << row[i];
            else  // non-first cell in the row
                output << "," << row[i];
        }
        output << "\n";
    }
}

}  // namespace CSV

#endif  // CSV
