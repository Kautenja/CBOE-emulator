// Exceptions that can be thrown by the application.
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

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>

/// An exception class.
class Exception: public std::exception {
 protected:
    /// the error message.
    std::string msg_;

 public:
    /// @brief Constructor (C strings).
    /// @param message C-style string error message. The string contents are
    /// copied upon construction. Hence, responsibility for deleting the char*
    /// lies with the caller.
    explicit Exception(const char* message) : msg_(message) { }

    /// @brief Constructor (C++ STL strings).
    /// @param message The error message.
    explicit Exception(const std::string& message) : msg_(message) { }

    /// @brief Destroy this exception.
    virtual ~Exception() throw () { }

    /// @brief Returns a pointer to the (constant) error description.
    /// @returns A pointer to a const char*. The underlying memory is in
    /// possession of the Exception object. Callers must not attempt to free
    /// the memory.
    virtual const char* what() const throw () { return msg_.c_str(); }
};

#endif  // EXCEPTIONS_HPP
