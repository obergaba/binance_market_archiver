#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

using json = nlohmann::json;

using websocketpp::client;
using websocketpp::config::asio_tls_client;
using websocketpp::connection_hdl;
using message_ptr = websocketpp::config::asio_tls_client::message_type::ptr;

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

template<typename T>
class ThreadSafeQueue {
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }

    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

// Globals
ThreadSafeQueue<MarketData> market_queue;
ThreadSafeQueue<MarketData> market_queue_futures;
bool running = true;
bool x = false;

class WebsocketClient {
public:
    WebsocketClient(bool isFuture = false) : connected(false), isFuture(isFuture) {}

    void connect(const std::string& stream) {
        try {
            client.init_asio();
            client.set_tls_init_handler([this](websocketpp::connection_hdl) {
                auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
                try {
                    ctx->set_options(boost::asio::ssl::context::default_workarounds |
                                    boost::asio::ssl::context::no_sslv2 |
                                    boost::asio::ssl::context::no_sslv3 |
                                    boost::asio::ssl::context::single_dh_use);
                } catch (std::exception &e) {
                    std::cout << "Error in context pointer: " << e.what() << std::endl;
                }
                return ctx;
            });

            client.set_message_handler([this](connection_hdl hdl, message_ptr msg) {
                handle_message(hdl, msg);
            });
            client.set_open_handler([this](connection_hdl hdl) {
                this->hdl = hdl;
                this->connected = true;
                std::cout << "WebSocket connection established." << std::endl;
            });
            client.set_fail_handler([this](connection_hdl hdl) {
                this->connected = false;
                std::cout << "WebSocket connection failed." << std::endl;
            });
            client.set_close_handler([this](connection_hdl hdl) {
                this->connected = false;
                std::cout << "WebSocket connection closed." << std::endl;
            });

            websocketpp::lib::error_code ec;
            auto con = client.get_connection(get_uri(stream), ec);
            if (ec) {
                std::cerr << "Could not create connection: " << ec.message() << std::endl;
                return;
            }

            client.connect(con);
            client.run();
        } catch (const std::exception& e) {
            std::cerr << "Error connecting to WebSocket: " << e.what() << std::endl;
        }
    }

    void send_req(const std::string& req) {
        if (connected) {
            websocketpp::lib::error_code ec;
            client.send(hdl, req, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cerr << "Error sending request: " << ec.message() << std::endl;
            }
        } else {
            std::cerr << "No WebSocket connection to send request." << std::endl;
        }
    }

private:
    websocketpp::client<websocketpp::config::asio_tls_client> client; 
    websocketpp::connection_hdl hdl;    
    bool connected;
    bool isFuture;

    std::string get_uri(const std::string& stream) const {
        if (isFuture) {
            return "wss://fstream.binance.com/stream?streams=" + stream;
        } else {
            return "wss://stream.binance.com:9443/stream?streams=" + stream;
        }
    }

    void handle_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        MarketData t;
        long long latency;
        try {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            
            auto json_payload = json::parse(msg->get_payload());
            auto data = json_payload["data"];

            std::string pair = data["s"];
            std::string bbp = data["b"];
            std::string bbs = data["B"];
            std::string bap = data["a"];
            std::string bas = data["A"];

            if(isFuture){
                long long eventTime = data["E"].get<long long>();     
                latency = millis - eventTime;
                t.EventTime = eventTime;
                t.latency = latency;
            }

            t.CurrentTime = millis;

            std::strncpy(t.pair, pair.c_str(), sizeof(t.pair) - 1);
            t.pair[sizeof(t.pair) - 1] = '\0'; 

            std::strncpy(t.BestBidPrice, bbp.c_str(), sizeof(t.BestBidPrice) - 1);
            t.BestBidPrice[sizeof(t.BestBidPrice) - 1] = '\0'; 

            std::strncpy(t.BestBidSize, bbs.c_str(), sizeof(t.BestBidSize) - 1);
            t.BestBidSize[sizeof(t.BestBidSize) - 1] = '\0'; 

            std::strncpy(t.BestAskPrice, bap.c_str(), sizeof(t.BestAskPrice) - 1);
            t.BestAskPrice[sizeof(t.BestAskPrice) - 1] = '\0'; 

            std::strncpy(t.BestAskSize, bas.c_str(), sizeof(t.BestAskSize) - 1);
            t.BestAskSize[sizeof(t.BestAskSize) - 1] = '\0'; 

            // std::cout << "Stream: " << t.pair << std::endl;
            // std::cout << "Bbp: " << t.BestBidPrice << std::endl;
            // std::cout << "Bbs: " << t.BestBidSize << std::endl;
            // std::cout << "Bap: " << t.BestAskPrice << std::endl;
            // std::cout << "Bas: " << t.BestAskSize << std::endl;
            // std::cout << "Current Time (ms): " << t.CurrentTime << std::endl;
            // if(isFuture){
            //     std::cout << "Event Time: " << t.EventTime << std::endl;
            //     std::cout << "Latency: " << t.latency << std::endl;
            // }
            // std::cout << "-------------------" <<std::endl;

            if (isFuture) {
                market_queue_futures.push(t);
            } else {
                market_queue.push(t);
            }

        } catch (const json::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
    }
};

void flush_queue_to_binary(std::string arg, bool isFutures) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(20));

        std::ofstream file(arg, std::ios::app | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error opening file for writing" << std::endl;
            continue;
        }
        MarketData data;
        if(!isFutures){
        while (market_queue.try_pop(data)) {
            file.write(reinterpret_cast<char*>(&data), sizeof(MarketData));
            }
        }else{
            while (market_queue_futures.try_pop(data)) {
            file.write(reinterpret_cast<char*>(&data), sizeof(MarketData));
            }
        }
        file.close();
    }
}

int main() {

    std::string spotFileName = "market_data_spot.bin";
    std::string futuresFileName = "market_data_futures.bin";
    std::thread binary_flusher_spot(flush_queue_to_binary, spotFileName, false);
    std::thread binary_flusher_futures(flush_queue_to_binary, futuresFileName, true);

    WebsocketClient ws_client_spot(false);  
    WebsocketClient ws_client_futures(true); 

    std::thread spot_thread([&ws_client_spot]() {
        ws_client_spot.connect("btcusdt@bookTicker/ethusdt@bookTicker"); //add more pairs from here
    });

    std::thread futures_thread([&ws_client_futures]() {
        ws_client_futures.connect("btcusdt@bookTicker/ethusdt@bookTicker"); //add more pairs from here
    });

    while(running) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if(x){
            running = false;
        }
    }

    spot_thread.join();
    futures_thread.join();
    binary_flusher_spot.join();
    binary_flusher_futures.join();

    return 0;
}
