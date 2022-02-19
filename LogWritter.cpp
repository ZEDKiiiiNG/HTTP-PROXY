#include "LogWritter.hpp"

void logWritter::writeLog(std::string logContent){
    std::ofstream logStram;
    logStram.open("/var/log/erss/proxy.log", std::ios::app);// ios::app 以追加的方式打开文件
    logStram << logContent;
    logStram.close();
}
