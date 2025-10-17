#include <bitset>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Class representing a (15, 11) block code
// The parity bits are at positions 1, 2, 4, and 8
// Data bits are at positions 3, 5, 6, 7, 9, 10, 11, 12, 13, 14, and 15

class block_code { // Class for block code <15, 11>
    static constexpr int MAX_BLOCK_SIZE = 11;
    static constexpr int CODEWORD_SIZE = 15;
    static constexpr int MATRIX_SIZE = 16; // 4x4 matrix
  public:
    block_code(std::string input) : matrix_(MATRIX_SIZE, false) {
        if (input.length() > MAX_BLOCK_SIZE) {
            throw std::invalid_argument(
                "Input string exceeds maximum length of 11 bits.");
        }
        encode(input);
    }
    void encode(std::string input) {
        this->quantity_padding_bits_ = MAX_BLOCK_SIZE - input.length();
        // Initialize the matrix with input data bits
        int data_bit_index = 0;
        for (int i = 0; i < MATRIX_SIZE; ++i) {
            // Skip parity bit positions: 1, 2, 4, 8
            if (i == 0 || i == 1 || i == 2 || i == 4 || i == 8) {
                continue;
            }
            if (data_bit_index < input.length()) {
                this->matrix_[i] = (input[data_bit_index] == '1');
                ++data_bit_index;
            }
        }
        // Calculate parity bits
        this->matrix_[1] = calculate_parity(1);
        this->matrix_[2] = calculate_parity(2);
        this->matrix_[4] = calculate_parity(4);
        this->matrix_[8] = calculate_parity(8);
    }
    void print_codeword() const {
        std::cout << "Encoded (15, 11) codeword: ";
        std::cout << std::endl;
        std::cout << " ";
        for (int i = 1; i <= CODEWORD_SIZE; ++i) {
            if (i == 3 || i == 7 || i == 11 || i == 15) {
                std::cout << this->matrix_[i];
                std::cout << std::endl;
            } else {
                std::cout << this->matrix_[i];
            }
        }
        std::cout << std::endl;
    }
    void inject_error(int position) {
        if (position < 1 || position > CODEWORD_SIZE) {
            throw std::out_of_range("Error position is out of range.");
        }
        this->matrix_[position] = !this->matrix_[position];
    }

    std::string decode() {
        std::string decoded_data;
        // Verify and correct errors
        int error_position = 0;
        std::vector<bool> partities_data = {this->matrix_[1], this->matrix_[2],
                                            this->matrix_[4], this->matrix_[8]};

        std::vector<bool> calculated_partities = {
            calculate_parity(1), calculate_parity(2), calculate_parity(4),
            calculate_parity(8)};

        for (size_t i = 0; i < partities_data.size(); ++i) {
            if (partities_data[i] != calculated_partities[i]) {
                error_position += (1 << i);
            }
        }
        // Correct the error if found
        if (error_position != 0) {
            std::cout << "Error detected at position: " << error_position
                      << std::endl;
            // Correct the error modifying the matrix_
            this->matrix_[error_position] = !this->matrix_[error_position];
        }

        // Extract data bits
        for (int i = 1; i <= CODEWORD_SIZE; ++i) {
            if (i == 1 || i == 2 || i == 4 || i == 8) {
                continue; // Skip parity bits
                          // Append data bits to decoded_data
            }
            decoded_data += this->matrix_[i] ? '1' : '0';
        }
        // Remove padding bits
        if (this->quantity_padding_bits_ > 0) {
            decoded_data = decoded_data.substr(
                0, decoded_data.length() - this->quantity_padding_bits_);
        }
        return decoded_data;
    }
    ~block_code() = default;

  private:
    std::vector<bool> matrix_;
    int quantity_padding_bits_;
    bool calculate_parity(int position) {
        if (position == 1) {
            return this->matrix_[3] ^ this->matrix_[5] ^ this->matrix_[7] ^
                   this->matrix_[9] ^ this->matrix_[11] ^ this->matrix_[13] ^
                   this->matrix_[15];
        } else if (position == 2) {
            return this->matrix_[3] ^ this->matrix_[6] ^ this->matrix_[7] ^
                   this->matrix_[10] ^ this->matrix_[11] ^ this->matrix_[14] ^
                   this->matrix_[15];
        } else if (position == 4) {
            return this->matrix_[5] ^ this->matrix_[6] ^ this->matrix_[7] ^
                   this->matrix_[12] ^ this->matrix_[13] ^ this->matrix_[14] ^
                   this->matrix_[15];
        } else if (position == 8) {
            return this->matrix_[9] ^ this->matrix_[10] ^ this->matrix_[11] ^
                   this->matrix_[12] ^ this->matrix_[13] ^ this->matrix_[14] ^
                   this->matrix_[15];
        } else {
            throw std::invalid_argument("Invalid parity bit position.");
        }
    }
};

class hamming_code {
  public:
    hamming_code() = default;
    void encode(std::string input) {
        std::string binary_string;
        for (char c : input) {
            binary_string += char_to_binary(c);
        }
        // Split the binary string into 11-bit blocks and encode each block
        for (size_t i = 0; i < binary_string.length(); i += 11) {
            std::string block =
                binary_string.substr(i, std::min(static_cast<size_t>(11),
                                                 binary_string.length() - i));
            this->blocks_.emplace_back(block_code(block));
        }
    }
    std::string decode() {
        std::string decoded_binary;
        for (auto &block : blocks_) {
            decoded_binary += block.decode();
        }
        // Convert binary string back to characters
        std::string decoded_string;
        for (size_t i = 0; i < decoded_binary.length(); i += 8) {
            std::string byte =
                decoded_binary.substr(i, std::min(static_cast<size_t>(8),
                                                  decoded_binary.length() - i));
            decoded_string += binary_to_char(byte);
        }
        return decoded_string;
    }
    void inject_error(int block_index, int position) {
        if (block_index < 0 || block_index >= blocks_.size()) {
            throw std::out_of_range("Block index is out of range.");
        }
        blocks_[block_index].inject_error(position);
    }
    std::string binary_to_char(const std::string &binary) {
        char c = static_cast<char>(std::bitset<8>(binary).to_ulong());
        return std::string(1, c);
    }
    std::string char_to_binary(char c) {
        std::bitset<8> bits(static_cast<unsigned char>(c));
        return bits.to_string();
    }
    void print_codewords() const {
        for (const auto &block : blocks_) {
            block.print_codeword();
        }
    }

    ~hamming_code() = default;

  private:
    std::vector<block_code> blocks_;
};

int main() {
    std::string input;
    std::cout << "Enter a string: ";
    std::getline(std::cin, input);

    hamming_code hc;
    hc.encode(input);
    hc.print_codewords();

    // Inject an error for demonstration
    hc.inject_error(0, 5); // Inject error in the first block at position 5

    std::string decoded_string = hc.decode();
    std::cout << "Decoded string: " << decoded_string << std::endl;
    return 0;
}
