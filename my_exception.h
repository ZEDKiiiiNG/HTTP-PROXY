#ifndef _MYEXCEPTION_H
#define _MYEXCEPTION_H

#include <exception>
#include <string>

class myException: public std::exception {
    private:
    std::string except;
    public: 
    myException(std::string str){
        except = str;
    }
    const char * what () const throw (){
    return except;
    }
};

#endif