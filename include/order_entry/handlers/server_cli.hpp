// A command line interface to the server service.
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

#ifndef HANDLERS_SERVER_CLI_HPP
#define HANDLERS_SERVER_CLI_HPP

#include "order_entry/server.hpp"
#include "data_feed/sender.hpp"
#include <regex>
#include <sstream>
#include <iostream>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// Logic for handling CLI messages for the Client / Server.
namespace Handlers {

/// A CLI handler for interacting a Server.
class ServerCLI {
 private:
    /// the server the CLI is interacting with
    Server& server;
    /// the data feed the server is hosting
    DataFeed::Sender& feed;
    /// whether the trading session is active
    bool is_session_active = false;

    /// a regular expression for the start of session command
    const std::regex START_OF_SESSION = std::regex("^start\n$");

    /// @brief Handle a start of session command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void start_of_session(const std::string& line) {
        if (is_session_active) {
            std::cout << "session already active" << std::endl;
            return;
        }
        feed.send<DataFeed::Messages::StartOfSession>();
        is_session_active = true;
    }

    /// a regular expression for the end of session command
    const std::regex END_OF_SESSION = std::regex("^end\n$");

    /// @brief Handle a end of session command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void end_of_session(const std::string& line) {
        if (not is_session_active) {
            std::cout << "session not active" << std::endl;
            return;
        }
        feed.send<DataFeed::Messages::EndOfSession>();
        is_session_active = false;
    }

 public:
    /// @brief Initialize a new server handler for a CLI.
    ///
    /// @param server_ the server to interact with
    /// @param feed_ the data feed to send messages on
    ///
    ServerCLI(Server& server_, DataFeed::Sender& feed_) :
        server(server_),
        feed(feed_) { }

    /// @brief Process a command from the console.
    ///
    /// @param line the command to parse
    ///
    void parse(const std::string& line) {
        if (std::regex_match(line, START_OF_SESSION))
            start_of_session(line);
        else if (std::regex_match(line, END_OF_SESSION))
            end_of_session(line);
        else
            std::cout << "invalid command" << std::endl;
    }
};

}  // namespace Handlers

}  // namespace OrderEntry

#endif  // HANDLERS_SERVER_CLI_HPP
