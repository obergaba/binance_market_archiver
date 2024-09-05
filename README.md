# WebSocket Data Archiver

## Description

This project connects to Binance WebSockets to stream data for both spot and futures markets. It operates with two separate threads, each dedicated to one of the markets. The spot market thread handles the connection to Binance WebSocket for spot market data, while the futures market thread handles the connection for futures market data.

Each thread maintains its own queue of trade data. This data includes the WebSocket stream channel, the asset's price, the timestamp provided by Binance's server in milliseconds, and the local timestamp in milliseconds when the message was received.

Every 60 seconds, there are two distinct threads that handle the respective queues. The spot data thread retrieves data from the spot market queue and writes it to a binary file, while the futures data thread retrieves data from the futures market queue and writes it to a separate binary file.

## Build

To build the project, follow these steps:

1. **Install Dependencies**

   Ensure you have the necessary libraries installed:
   - OpenSSL
   - CMake
   - wesocketpp

2. **Clone the Repository**
   ```sh
   git clone https://github.com/obergaba/binance_market_archiver.git
   cd binance_market_archiver
3. **Create the build directory**
   ```sh
    mkdir build
    cd build
4. **Generate the build files**
   ```sh
    cmake ..
5. **Build the project**
    ```sh
    make
6. **Run the executable**
    ```sh
    ./market_data_archiver

## Viewing the Data

After approximately 60 seconds, two files will be created in the build directory: ```trade_data_spot.bin``` and ```trade_data_futures.bin```. To view the contents of these files, use the provided ```read.cpp file```. Navigate one directory up with ```cd ..```, compile the file using ```g++ read.cpp```, and then run the resulting executable with a command line argument specifying the file you want to read, for example: ```./a.out trade_data_spot.bin```.