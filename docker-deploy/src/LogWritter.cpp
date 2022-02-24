#include "LogWritter.hpp"

void logWritter::writeLog(std::string logContent) {
  std::ofstream logStream;
  logStream.open("/var/log/erss/proxy.log", std::ios::app);
  logStream << logContent;
  logStream.close();
}
