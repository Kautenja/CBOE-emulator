// A server for hosting a financial instrument exchange micro-service.
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

#include "order_entry/server.hpp"
#include "order_entry/handlers/server_cli.hpp"
#include "order_entry/extensions/server_heartbeat.hpp"
#include "cli.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <thread>

using OrderEntry::Handlers::ServerCLI;
using OrderEntry::Extensions::ServerHeartbeat;

int main(int argc, char* argv[]) {
    if (argc != 2) {  // wrong number of command line arguments
        std::cerr << "Usage:\n\tserver <config>" << std::endl;
        std::cerr << "Example:\n\tserver config.json" << std::endl;
        return 1;
    }
    try {
        // create a stream to read the config file
        std::string config_filename(argv[1]);
        nlohmann::json config;
        std::ifstream(config_filename) >> config;
        // std::cout << "using configuration:" << std::endl;
        // std::cout << std::setw(4) << config << std::endl;

        // get the parameters from the JSON file
        auto port = config["port"].get<uint16_t>();
        auto feed_group = asio::ip::make_address(config["feed"]["group"].get<std::string>());
        auto feed_port = config["feed"]["port"].get<uint16_t>();
        auto accounts = config["accounts"].get<std::vector<nlohmann::json>>();

        // create the ASIO context for the services
        asio::io_context context;
        // create the data feed
        auto feed = DataFeed::Sender(context, feed_group, feed_port);
        if (config["feed"].contains("ttl"))  // set the TTL
            feed.set_ttl(config["feed"]["ttl"].get<uint32_t>());
        if (config["feed"].contains("outbound_interface")) {  // set the interface
            auto addr_str = config["feed"]["outbound_interface"].get<std::string>();
            auto addr = asio::ip::address_v4::from_string(addr_str);
            feed.set_outbound_interface(addr);
        }
        // create the limit order book with the data feed handler
        auto book = OrderEntry::LOB::LimitOrderBook(&feed);
        // create the server
        OrderEntry::Server server(context, book, port, accounts);
        // create the command line interface
        auto cli_handler = ServerCLI(server, feed);
        CLI<ServerCLI> cli(context, cli_handler);
        ServerHeartbeat serverHeartbeat(context, server, feed);
        // start the IO context
        context.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
