#include "Config.hpp"

int main(int argc, char **argv)
{
  Config  conf;

  if (argc > 2)
  {
    std::cout << "Too many arguments. Max. one argument for config file path.";
    std::cout << std::endl;
    return (1);
  }
  if (argc == 2) // User provided config file path
    conf.setPath(argv[1]);
  if (!conf.parseFile())
  {
    std::cout << "Invalid config file." << std::endl;
    return (1);
  }
  return (0);
}
