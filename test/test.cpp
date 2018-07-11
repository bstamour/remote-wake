#include "remote_wake.hpp"

constexpr auto server_ip  = "192.168.0.100";

int main()
{
  bst::remote_machine m{server_ip};

  // Do stuff.
}
