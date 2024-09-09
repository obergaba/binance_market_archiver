#include <iostream>
#include <string>
#include <fstream>

struct MarketData {
    char pair[10];
    char BestBidPrice[10];
    char BestAskPrice[10];
    char BestBidSize[10];
    char BestAskSize[10];
    long long CurrentTime;
    long long EventTime;
    long long latency;
};

void read_from_binary(const std::string& fileName, bool isFuture) {
    std::ifstream file("build/" + fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading" << std::endl;
        return;
    }

    MarketData data;
    while (file.read(reinterpret_cast<char*>(&data), sizeof(MarketData))) {
        std::cout << "Stream: " << data.pair 
                << ", Bbp: " << data.BestBidPrice 
                << ", Bbs: " << data.BestBidSize 
                << ", Bap: " << data.BestAskPrice 
                << ", Bas: " << data.BestAskSize 
                << ", Current Time (ms): " << data.CurrentTime;
                if (isFuture) {
                    std::cout << ", Event Time: " << data.EventTime 
                            << ", Latency: " << data.latency;
                }
                std::cout << "\n";
    }
    file.close();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    bool isFuture = false;
    std::string fileName = argv[1];
    if(fileName == "market_data_futures.bin"){
        isFuture = true;
    }
    read_from_binary(fileName, isFuture);  
    return 0;
}
