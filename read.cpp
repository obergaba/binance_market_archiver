#include <iostream>
#include <string>
#include <fstream>

struct TradeData {
    char stream[50];
    char price[20];
    long long trade_time;
    long long current_time_millis;
    long long latency;
};

void read_from_binary(const std::string& fileName) {
    std::ifstream file("build/" + fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading" << std::endl;
        return;
    }

    TradeData data;
    while (file.read(reinterpret_cast<char*>(&data), sizeof(TradeData))) {
        std::cout << "Stream: " << data.stream 
                  << ", Price: " << data.price 
                  << ", Trade Time: " << data.trade_time 
                  << ", Current Time (ms): " << data.current_time_millis
                  << ", Latency: " << data.latency << "\n";
    }
    file.close();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string fileName = argv[1];
    read_from_binary(fileName);  
    return 0;
}
