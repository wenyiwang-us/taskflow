#include <iostream>

int main() {

  size_t head = 0;
  size_t mask = 15;
  std::cout << "head: " << head << std::endl;
  std::cout << "head - 1: " << (head - 1) << std::endl;
  std::cout << "head - 1 & mask: " << (head - 1 & mask) << std::endl;
  return 0;
}
