//  Created by Wuyang on 4/5/16.
//  Copyright © 2016 Wuyang. All rights reserved.

#include <iostream>

#include "ValueIteration.hpp"
#include "MobilitySimulator.hpp"

//writeCLient does not work

int main(){
   
    MobilitySimulator* ms = new MobilitySimulator(5, 6, 6);
    ms->simulate();
    ms->~MobilitySimulator();

    return 0;
}
