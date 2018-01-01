// Copyright (c) 2018 AlertAvert.com. All rights reserved.


#include <iostream>
#include <cstring>
#include "version.h"


int main(int argc, const char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "-f") == 0)
    std::cout << RELEASE_STR << std::endl;
  else
    std::cout << VERSION_STR << std::endl;
  return EXIT_SUCCESS;
}
