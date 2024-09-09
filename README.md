# WebSocket Data Archiver

## Description

This project connects to Binance WebSockets to stream data for both spot and futures markets. It operates with two separate threads, each dedicated to one of the markets. The spot market thread handles the connection to Binance WebSocket for spot market data, while the futures market thread handles the connection for futures market data.

Each thread in the system maintains its own queue of market data. This data includes the pair name, the top level of the order book—specifically, the best bid price and size, the best ask price and size, and their respective timestamps. The timestamp recorded represents when the message was received by our machine. Additionally, for futures channels, Binance provides its own server timestamp in milliseconds. This allows us to calculate the latency between Binance's server and our own machine.

Every 20 seconds, there are two distinct threads that handle the respective queues. The spot data thread retrieves data from the spot market queue and writes it to a binary file, while the futures data thread retrieves data from the futures market queue and writes it to a separate binary file.

## Build

To build the project, follow these steps:

1. **Install Dependencies**

   Ensure you have the necessary libraries installed:
   - OpenSSL
   - CMake
   - Wesocketpp
   - Nlohmann-json

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

After approximately 20 seconds, two files will be created in the build directory: ```market_data_spot.bin``` and ```market_data_futures.bin```. To view the contents of these files, use the provided ```read.cpp```file. Navigate one directory up with ```cd ..```, compile the file using ```g++ read.cpp```, and then run the resulting executable with a command line argument specifying the file you want to read, for example: ```./a.out market_data_futures.bin```.

## Issues and Consideration

For getting the top level order book in binance there are 3 channel to consider:
* From the bookTicker channel: provides real-time updates for the top level of the order book. However, it does not include event or transaction timestamps for the spot market (for futures includes both). As a result, while this channel offers real-time updates, it does not allow for the calculation of latency for the spot markets (for futures is ok).
* From the ticker channel provides: updates for the top level of the order book along with timestamps. However, it sends data every 1000 milliseconds 
* From the Differential Depth Stream: provides incremental updates for the entire order book, it requires to manage all the order books locally. This stream includes timestamps for each update, but it does not update in real-time. Instead, it pushes updates every 100 milliseconds.

Currently this system is using the bookTicker channel.

For reference: https://dev.binance.vision/t/add-event-time-field-to-spot-websocket-book-streams/1390