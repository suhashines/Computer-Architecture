#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>

// Function to convert hexadecimal to binary
std::string hex_to_binary(const std::string& hex_string) {
    std::stringstream ss;
    ss << std::hex << hex_string;
    unsigned int hex_value;
    ss >> hex_value;
    std::string binary_string = std::bitset<16>(hex_value).to_string();
    return binary_string;
}

int main() {
    std::ifstream input_file("hex.txt");
    std::ofstream output_file("output.txt");

    if (!input_file.is_open() || !output_file.is_open()) {
        std::cerr << "Error: Unable to open files!" << std::endl;
        return 1;
    }

    std::vector<std::string> binary_values;
    std::string hex_value;
    while (std::getline(input_file, hex_value)) {
        std::string binary_value = hex_to_binary(hex_value);
        binary_values.push_back(binary_value);
    }

    input_file.close();

    // Writing to output.txt
    output_file << "uint16_t arr[] = {";
    for (size_t i = 0; i < binary_values.size(); ++i) {
        if (i != 0)
            output_file << ", ";
        output_file << "0b" << binary_values[i];
    }
    output_file << "};" << std::endl;
    output_file << "int length = " << binary_values.size() << ";" << std::endl;

    output_file.close();

    return 0;
}
