#include "HTTPCache.hpp"

#include <iostream>

#include "HTTPResponse.hpp"
bool HTTPCache::exists(std::string key) {
  for (std::list<std::string>::iterator it = keyList.begin(); it != keyList.end(); ++it) {
    if (*it == key) {
      return true;
    }
  }
  return false;
}

const HTTPResponse & HTTPCache::get(std::string key) {
  keyList.remove(key);
  keyList.push_front(key);
  return table.find(key)->second;
}

void HTTPCache::add(std::pair<std::string, HTTPResponse> p) {
  //if exists key overrite and put it at front
  if (exists(p.first)) {
    keyList.remove(p.first);
    table[p.first] = p.second;
    keyList.push_front(p.first);
  }
  //if not exist overrite the last one and put at front
  else {
    if (keyList.size() == maxLen) {
      table.erase(keyList.back());
      keyList.pop_back();
    }
    keyList.push_front(p.first);
    table.insert(p);
  }
}
void HTTPCache::printKeyList() {
  std::cout << "Print Key List..." << std::endl;
  for (std::list<std::string>::iterator it = keyList.begin(); it != keyList.end(); ++it) {
    std::cout << *it << std::endl;
  }
  std::cout << "End of Key List" << std::endl;
}
void HTTPCache::printMap() {
  std::cout << "Print Map..." << std::endl;
  for (std::map<std::string, HTTPResponse>::iterator it = table.begin();
       it != table.end();
       ++it) {
    std::cout << it->first << ": " << it->second.getStartLine() << std::endl;
  }
  std::cout << "End of Map" << std::endl;
}
