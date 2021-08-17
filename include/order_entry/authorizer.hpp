// The credentials manager for the order entry system.
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

#ifndef ORDER_ENTRY_AUTHORIZER_HPP
#define ORDER_ENTRY_AUTHORIZER_HPP

#include "system_account.hpp"
#include <string>
#include <map>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// @brief Logic for looking up and validating credentials for new connections.
/// @tparam Handler the handler type for the system accounts being managed
///
template <typename Handler>
class Authorizer {
 private:
    /// a mapping of user-names to account
    std::map<std::string, SystemAccount<Handler>> accounts;

 public:
    /// @brief Return the number of accounts in the manager.
    inline std::size_t size() const { return accounts.size(); }

    /// @brief Create a new user in the account manager.
    ///
    /// @param username the username for the new account
    /// @param password the password for the new account
    ///
    void new_user(std::string username, std::string password) {
        if (accounts.find(username) != accounts.end())  // username found
            throw "username already exists in system";
        accounts[username] = {username, password};
    }

    /// @brief Return true if the username exists in the database
    ///
    /// @param username the username to look up in the database
    /// @returns true if the user exists, false otherwise
    ///
    inline bool has_account(std::string username) const {
        return accounts.find(username) != accounts.end();
    }

    /// @brief Return the account for the given username
    ///
    /// @param username the username to look up in the database
    /// @returns the account for the given username
    ///
    inline SystemAccount<Handler>* get_account(std::string username) {
        return &accounts.at(username);
    }

    /// @brief Return true if the username and password combination is valid.
    ///
    /// @param username to username for the account
    /// @param password the password for the user's account
    /// @returns true if the combination is valid, false otherwise
    ///
    inline bool is_valid(
        const std::string& username,
        const std::string& password
    ) const {
        // make sure the username exists before looking up the password
        if (not accounts.count(username)) return false;
        // check if the password matches for the given username
        return accounts.at(username).password.compare(password) == 0;
    }

    /// @brief Write the data from the authorizer to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param authorizer the authorizer to write to the stream
    /// @returns the stream with data from the authorizer written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const Authorizer& authorizer
    ) {
        stream << SystemAccount<Handler>::table_header() << std::endl;
        for (auto pair : authorizer.accounts) stream << pair.second << std::endl;
        return stream;
    }
};

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_AUTHORIZER_HPP
