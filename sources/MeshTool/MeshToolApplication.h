#pragma once

#include <cxxopts.hpp>

class MeshToolApplication {
 public:
  MeshToolApplication();

  void execute(int argc, char* argv[]);

 private:
  void importMesh(const cxxopts::ParseResult& options);
  void importSkeleton(const cxxopts::ParseResult& options);
  void importAnimation(const cxxopts::ParseResult& options);
  void importCollisions(const cxxopts::ParseResult& options);
  void importScene(const cxxopts::ParseResult& options);

};
