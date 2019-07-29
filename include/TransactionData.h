#ifndef TransactionData_h
#define TransactionData_h
#include <string>
#include <time.h>
struct TransactionData
{
    std::string message;
    time_t timestamp;
    
    TransactionData(){};
    
    TransactionData(std::string msg, time_t time)
    {
        message = msg;
        timestamp = time;
    };
};

#endif
