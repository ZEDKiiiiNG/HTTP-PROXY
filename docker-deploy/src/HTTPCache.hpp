#include <list>
#include <map>
#include <string>
#include <utility>

#include "HTTPResponse.hpp"
class HTTPCache {
 private:
  size_t maxLen;
  std::list<std::string> keyList;
  std::map<std::string, HTTPResponse> table;

 public:
  HTTPCache(size_t m) : maxLen(m){};
  //always use exists before get
  bool exists(std::string key);

  //precondition: key exists in table
  const HTTPResponse & get(std::string key);
  void add(std::pair<std::string, HTTPResponse>);

  void printKeyList();
  void printMap();
};
