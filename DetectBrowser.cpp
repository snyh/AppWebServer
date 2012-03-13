#include "DetectBrowser.hpp"
#include <string>

bool Browser::open(int port)
{
#ifdef WIN32
  std::string cmd("start http://localhost:");
#else
  std::string cmd("xdg-open http://localhost:");
#endif
  cmd.append(std::to_string(port));
  if (system(cmd.c_str()) == 0)
	return true;
  else
	return false;
}
