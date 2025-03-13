#ifndef BROWSER_H
#define BROWSER_H

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class Browser {
public:
  fs::path rootDirectory;
  std::vector<std::filesystem::directory_entry> dir;

  Browser(std::string filePath) {
    rootDirectory = fs::path(filePath);
    Init();
  }
  void ListDir(fs::path filePath) {
    rootDirectory = filePath;
    for (auto &entry : fs::directory_iterator(filePath)) {
      dir.emplace_back(entry);
    }
  }
  void ListDir(std::string filePath) {
    rootDirectory = fs::path(filePath);
    for (auto &entry : fs::directory_iterator(filePath)) {
      dir.emplace_back(entry);
    }
  }

  void Init() { ListDir(rootDirectory); }
}; // namespace std::filesystem

#endif // !BROWSER_H