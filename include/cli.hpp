// A template-based asynchronous command line interface based on asio.
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

#ifndef CLI_HPP
#define CLI_HPP

#include <asio.hpp>
#include <iostream>

/// @brief A template-based asynchronous command line interface based on asio.
/// @tparam CommandHandler a callback handler type for handling string commands
///
template<typename CommandHandler>
class CLI {
 private:
    /// the size of the command line buffer
    static constexpr int BUFFER_SIZE = 1024;
    /// the IO context the CLI is running on
    asio::io_context& context;
    /// the input_stream for the CLI
    asio::posix::stream_descriptor input_stream;
    /// the input buffer
    asio::streambuf input_buffer;
    /// the command handler
    CommandHandler& handler;

    /// @brief Asynchronously read a line from the console
    void read_command_line() {
        asio::async_read_until(input_stream, input_buffer, '\n',
            [&](const std::error_code& error, std::size_t length){
                if (error) {  // TODO: handle error
                    std::cout << "CLI::read_command_line - " << error << std::endl;
                    return;
                }
                // get the data from the buffer
                auto data = asio::buffers_begin(input_buffer.data());
                // convert the data to string and parse the command
                handler.parse(std::string(data, data + length));
                // consume the data from the input buffer
                input_buffer.consume(length);
                // read another line from the console
                read_command_line();
            }
        );
    }

 public:
    /// @brief Initialize a new command line interface.
    ///
    /// @param context_ the context to create the CLI with
    /// @param handler_ the handler to pass commands to
    ///
    CLI(asio::io_context& context_, CommandHandler& handler_) :
        context(context_),
        input_stream(context, ::dup(STDIN_FILENO)),
        input_buffer(BUFFER_SIZE),
        handler(handler_) { read_command_line(); }
};

#endif  // CLI_HPP
