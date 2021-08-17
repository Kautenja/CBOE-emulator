// A client for sending data to a financial instrument exchange micro-service.
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

#include "order_entry/client.hpp"
#include "order_entry/handlers/client_cli.hpp"
#include "cli.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using OrderEntry::Client;
using OrderEntry::Handlers::ClientCLI;

int main(int argc, char* argv[]) {
    if (argc != 2) {  // wrong number of command line arguments
        std::cerr << "Usage:\n\tclient <config>" << std::endl;
        std::cerr << "Example:\n\tclient config.json" << std::endl;
        return 1;
    }
    try {
        // create a stream to read the config file
        std::string config_filename(argv[1]);
        nlohmann::json config;
        std::ifstream(config_filename) >> config;
        // std::cout << "using configuration:" << std::endl;
        // std::cout << std::setw(4) << config << std::endl;

        // get the host from the JSON parameters
        auto host = config["host"].get<std::string>();
        // get the port from the JSON parameters
        auto port = std::to_string(config["port"].get<uint16_t>());

        // create the IO context for the application
        asio::io_context context;
        // create the order entry client
        auto client = Client(context, host, port);
        // create the command line interface
        auto cli_handler = ClientCLI(client);
        CLI<ClientCLI> cli(context, cli_handler);
        // start the service
        context.run();
    }
    catch (std::exception& e) {  // print any errors and exit
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
