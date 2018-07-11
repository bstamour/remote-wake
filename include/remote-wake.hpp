//------------------------------------------------------------------------------

#ifndef BST_REMOTE_WAKE_HPP_
#define BST_REMOTE_WAKE_HPP_

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------

namespace bst {

namespace ipc = boost::interprocess;

constexpr auto file_name  = "bryan-shared-test";
constexpr auto mutex_name = "bryan_named_mutex";
constexpr auto server_ip  = "192.168.0.100";

//------------------------------------------------------------------------------

class remote_machine
{
public:
  explicit remote_machine(std::string_view);
  remote_machine(remote_machine const&) = delete;
  remote_machine(remote_machine&&)      = delete;
  remote_machine& operator=(remote_machine const&) = delete;
  remote_machine& operator=(remote_machine&&) = delete;
  ~remote_machine() noexcept;

private:
  std::string_view address_;

  bool is_machine_awake() const { return false; }
  void shut_down_machine() const {}
};

//------------------------------------------------------------------------------

remote_machine::remote_machine(std::string_view address)
  : address_{ std::move(address) }
{
  ipc::named_mutex mutex(ipc::open_or_create, mutex_name);
  ipc::scoped_lock<ipc::named_mutex> lock(mutex);

  if (!std::filesystem::exists(file_name)) {
    // We must be the first ones here.
    std::ofstream ifs{ file_name };
    auto awake = is_machine_awake();
    ifs << 1 << ' ' << awake << std::endl;
  } else {
    std::fstream ifs{ file_name };
    int current_count;
    bool awake;
    ifs >> current_count >> awake;
    ++current_count;
    ifs.seekp(0);
    ifs << current_count << ' ' << awake << std::endl; // Bump.
  }
}

remote_machine::~remote_machine() noexcept
{
  ipc::named_mutex mutex(ipc::open_or_create, mutex_name);
  ipc::scoped_lock<ipc::named_mutex> lock(mutex);

  std::fstream ifs{ file_name };
  int current_count;
  bool awake;
  ifs >> current_count >> awake;
  --current_count;

  if (current_count == 0) {
    std::filesystem::remove(file_name);
  } else {
    // Otherwise update the file.
    ifs.seekp(0);
    ifs << current_count << ' ' << awake << std::endl;
  }

  if (!awake) {
    // Our responsibility to shut the machine down.
  }
}

//------------------------------------------------------------------------------
}

#endif
