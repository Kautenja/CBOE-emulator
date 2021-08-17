// A Multi-cast data feed receiver that displays the current LOB statistics.
// Copyright 2020 Christian Kauten
//
// Author: Christian Kauten (kautenja@auburn.edu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "data_feed/receiver.hpp"
#include "data_feed/handlers/noop.hpp"
#include "data_feed/extensions/heartbeat.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <thread>

using DataFeed::Receiver;
using DataFeed::Handlers::NoOp;
using DataFeed::Extensions::Heartbeat;

int main(int argc, char* argv[]) {
    if (argc != 2) {  // wrong number of command line arguments
        std::cerr << "Usage:\n\theartbeat <config>" << std::endl;
        std::cerr << "Example:\n\theartbeat config.json" << std::endl;
        return 1;
    }
    try {
        // create a stream to read the config file
        std::string config_filename(argv[1]);
        nlohmann::json config;
        std::ifstream(config_filename) >> config;
        std::cout << "using configuration:" << std::endl;
        std::cout << std::setw(4) << config << std::endl;

        // get the parameters from the JSON file
        auto listen = asio::ip::make_address(config["listen"].get<std::string>());
        auto group = asio::ip::make_address(config["group"].get<std::string>());
        auto port = config["port"].get<uint16_t>();

        // create the IO context
        asio::io_context context;
        // initialize the receiver with the IO context
        NoOp handler;
        Receiver<NoOp> receiver(context, listen, group, port, handler);
        // std::thread receiver_thread([&context](){ context.run(); });
        Heartbeat<NoOp> printer(context, receiver);
        // std::thread printer_thread([&context](){ context.run(); });
        // start the service
        context.run();
        // receiver_thread.join();
        // printer_thread.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
