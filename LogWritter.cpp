#include "LogWritter.hpp"

void logWritter::writeLog(std::string logContent) {
  std::ofstream logStream;
  logStream.open("proxy.log", std::ios::app);
  logStream << logContent;
  logStream.close();
}
