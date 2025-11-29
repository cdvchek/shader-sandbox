#include "renderer.hpp"
#include <iostream>

int main() {
  std::cout << "Initializing Renderer" << std::endl;
  Renderer renderer;
  std::cout << "Starting Renderer" << std::endl;
  renderer.startRenderer();
  return 0;
}
