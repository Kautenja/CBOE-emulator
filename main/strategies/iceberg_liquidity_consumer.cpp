// A liquidity consumer agent based on an iceberg trading scheme.
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

#include "strategies/iceberg_liquidity_consumer.hpp"
#include <fstream>
#include <thread>

int main(int argc, char* argv[]) {
    if (argc != 2) {  // wrong number of command line arguments
        std::cerr << "Usage:\n\ticeberg_trader <config>" << std::endl;
        std::cerr << "Example:\n\ticeberg_trader config.json" << std::endl;
        return 1;
    }
    try {
        // create a stream to read the config file
        std::string config_filename(argv[1]);
        nlohmann::json config;
        std::ifstream(config_filename) >> config;
        // std::cout << "using configuration:" << std::endl;
        // std::cout << std::setw(4) << config << std::endl;
        // start the strategy
        asio::io_context feed_context;
        asio::io_context context;
        Strategies::IcebergLiquidityConsumer strategy(feed_context, context, config);
        std::thread feed_thread([&](){ feed_context.run(); });
        context.run();
        feed_thread.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
