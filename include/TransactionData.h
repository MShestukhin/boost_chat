//
//  TransactionData.h
//  BlockChainExample
//
//  Created by Jon on 1/10/18.
//  Copyright © 2018 Crypto Enthusiasts. All rights reserved.
//

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

#endif /* TransactionData_h */
