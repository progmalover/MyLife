//
// io_control.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IO_CONTROL_HPP
#define ASIO_DETAIL_IO_CONTROL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/push_options.hpp"

#include "asio/detail/push_options.hpp"
#include <cstddef>
#include <boost/config.hpp>
#include "asio/detail/pop_options.hpp"

#include "asio/detail/socket_types.hpp"

namespace asio {
namespace detail {
namespace io_control {

// IO control command for non-blocking I/O.
class non_blocking_io
{
public:
  // Default constructor.
  non_blocking_io()
    : value_(0)
  {
  }

  // Construct with a specific command value.
  non_blocking_io(bool value)
    : value_(value ? 1 : 0)
  {
  }

  // Get the name of the IO control command.
  int name() const
  {
    return FIONBIO;
  }

  // Set the value of the I/O control command.
  void set(bool value)
  {
    value_ = value ? 1 : 0;
  }

  // Get the current value of the I/O control command.
  bool get() const
  {
    return value_ != 0;
  }

  // Get the address of the command data.
  detail::ioctl_arg_type* data()
  {
    return &value_;
  }

  // Get the address of the command data.
  const detail::ioctl_arg_type* data() const
  {
    return &value_;
  }

private:
  detail::ioctl_arg_type value_;
};

// I/O control command for getting number of bytes available.
class bytes_readable
{
public:
  // Default constructor.
  bytes_readable()
    : value_(0)
  {
  }

  // Construct with a specific command value.
  bytes_readable(std::size_t value)
    : value_(static_cast<detail::ioctl_arg_type>(value))
  {
  }

  // Get the name of the IO control command.
  int name() const
  {
    return FIONREAD;
  }

  // Set the value of the I/O control command.
  void set(std::size_t value)
  {
    value_ = static_cast<detail::ioctl_arg_type>(value);
  }

  // Get the current value of the I/O control command.
  std::size_t get() const
  {
    return static_cast<std::size_t>(value_);
  }

  // Get the address of the command data.
  detail::ioctl_arg_type* data()
  {
    return &value_;
  }

  // Get the address of the command data.
  const detail::ioctl_arg_type* data() const
  {
    return &value_;
  }

private:
  detail::ioctl_arg_type value_;
};

} // namespace io_control
} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IO_CONTROL_HPP
