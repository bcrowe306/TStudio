#ifndef BROWSER_H
#define BROWSER_H

#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include "text/choc_StringUtilities.h"

using std::vector;
namespace fs = std::filesystem;
using fs::path;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::async;
using std::future;
using choc::text::contains;
using choc::text::toLowerCase;

const vector<string> file_extensions = {
  ".wav",
  ".ogg",
  ".aac",
  ".flac",
  ".aif",
  ".aiff",
};

namespace fs = std::filesystem;
class BrowserNode{
  public:
    string fileName;
    path filePath;
    vector<BrowserNode> children;

    BrowserNode(const string &fileName){
      this->filePath = path(fileName);
      this->fileName = this->filePath.filename().string();
      this->init();
    }
    BrowserNode(const string &fileName, string &filter){
      this->filePath = path(fileName);
      this->fileName = this->filePath.filename().string();
      this->fileNameFilter = filter;
      this->init();
    }
    BrowserNode(path filePath)
    {
      this->filePath = filePath;
      this->fileName = this->filePath.filename().string();
      this->init();
    }

    bool isLoading(){
      // bool isChildLoading = false;
      // for(auto &child : children){
      //   if(child.isLoading())
      //     isChildLoading = true;
      //     break;
      // }
      // if (loading || isChildLoading){
      //   return true;
      // }else{
      //   return false;
      // }
      return loading;
    }
    void clearChildren(){
      if(children.size() > 0){
        children.clear();
        childFileCount = 0;
      }
    }
    bool filter(path fPath){
      bool isMatch = false;
      for(auto &ext: file_extensions){
        if (fPath.extension().string() == ext )
          isMatch = true;
      }
      if(isMatch){

        isMatch = contains(toLowerCase(fPath.filename().string()), toLowerCase(fileNameFilter));
      }
      return isMatch;
    }
    void loadChildren(vector<BrowserNode> * childs, path childPath, string fileFilter){
      auto node = BrowserNode(childPath, fileFilter);
      if (node.hasChildren())
        childs->push_back(node);
    }
    void init(){
      clearChildren();
      loading = true;
      if (fs::is_directory(filePath)){

        // Add directories first
        for (auto &entry : filesystem::directory_iterator(filePath))
        {
          if(entry.is_directory()){
            async(launch::async, &BrowserNode::loadChildren, this, &children, entry.path(), fileNameFilter);
            // bool hasMatchedFiles = false;
            // auto node = BrowserNode(entry.path(), fileNameFilter);
            // if(node.hasChildren())
            //   children.push_back(node);
          }
        }

        // Add files next
        for (auto &entry : filesystem::directory_iterator(filePath))
        {
          if(!entry.is_directory()){
            auto p = entry.path();
            if(filter(p)){
              children.emplace_back(BrowserNode(p));
              childFileCount++;
            }
          }
        }
      }
      loading = false;
    }
    void addNode(BrowserNode &node){
      children.emplace_back(node);
    };

    bool hasChildren(){
      return children.size() > 0;
    }

    void setFilter(string filter){
      fileNameFilter = filter;
      printf(fileNameFilter.c_str());
      init();
    }
    int getChildFileCount(){
      return childFileCount;
    }
  private:
    string fileNameFilter = "";
    bool loading = false;
    int childFileCount = 0;
};

class Browser {
public:
  vector<BrowserNode> browserItems;
  
  Browser() = default;
  ~Browser() = default;

  // Methods
  void addBrowserItem(string &fileName){
    browserItems.emplace_back(BrowserNode(fileName));
  }

  void removeBrowserItem(string &fileName){
    auto it = erase_if(browserItems, [fileName](BrowserNode &node){
      return node.fileName == fileName;
    });
  }
private:
};

#endif // !BROWSER_H