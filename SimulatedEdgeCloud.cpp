//
//  SimulatedEdgeCloud.cpp
//  MarkovProcess
//
//  Created by Wuyang on 5/5/16.
//  Copyright © 2016 Wuyang. All rights reserved.
//

#include <ctime>
#include <fstream>

#include "SimulatedEdgeCloud.hpp"
#include "SimulatedCentralController.hpp"


SimulatedEdgeCloud::SimulatedEdgeCloud(const int addr){
    _myAddr = addr;
   
    totalClientNumber(0);
    _controller = new SimulatedCentralController();
    
    //define cost
    _baseResponse = 100;
    _singleHopCost = 100;
    _unitLoadCost = 100;
}

void
SimulatedEdgeCloud::init(SimulatedCentralController* const control, bool const readLoad){
    if(!readLoad){
        generateBaseWorkload();
        writeBaseWorkload();
    }else{
        readBaseWorkload();
    }
    appWorkload(0);
    totalWorkload(baseWorkload());
    
    _controller = control;
    _totalEdgeCloudNumber = _controller->cloudList()->size();
}

SimulatedEdgeCloud::~SimulatedEdgeCloud(){
    
}

void
SimulatedEdgeCloud::myAddr(const int addr){
    _myAddr = addr;
}

int
SimulatedEdgeCloud::myAddr() const{
    return _myAddr;
}

void
SimulatedEdgeCloud::totalWorkload(const double load){
    _totalWorkload = load;
}

double
SimulatedEdgeCloud::totalWorkload() const{
    return _totalWorkload;
}

void
SimulatedEdgeCloud::baseWorkload(const double load){
    _baseWorkload = load;
}

double
SimulatedEdgeCloud::baseWorkload()const {
    return _baseWorkload;
}

void
SimulatedEdgeCloud::writeBaseWorkload(){
    
    char fileName[100];
    std::sprintf(fileName, "./server/loadSample_%d", _myAddr);
    
    std::ofstream out;
    out.open (fileName);
    out << baseWorkload()<<"\n";
    out.close();
    
}

void
SimulatedEdgeCloud::readBaseWorkload(){
    
    char fileName[100];
    std::sprintf(fileName, "./server/loadSample_%d", _myAddr);
    std::ifstream in;
    in.open(fileName);
    
    std::string line;
    
    if (in.is_open())
    {
        while ( getline (in,line) )
        {
            baseWorkload(atof(line.c_str()));
        }
        in.close();
    }
    
}

void
SimulatedEdgeCloud::appWorkload(const double load){
    _appWorkload = load;
}

double
SimulatedEdgeCloud::appWorkload() const{
    return _appWorkload;
}

void
SimulatedEdgeCloud::totalClientNumber(const int num){
    _totalClientNumber = num;
}

int
SimulatedEdgeCloud::totalClientNumber() const{
    return _totalClientNumber;
}

void
SimulatedEdgeCloud::printConnectedClient(){
    printf("[SimulatedEdgeCloud %d] connected client: \n", _myAddr);
    for(std::vector<int>::iterator it = clientAddr.begin(); it!= clientAddr.end(); ++it){
        printf("%d\n", (*it));
    }
}
void
SimulatedEdgeCloud::generateBaseWorkload(){
    // baseload ( 0, 0.2)
    srand(_myAddr);
    double load = rand() / double(RAND_MAX) / 5;
    baseWorkload(load);
}

void
SimulatedEdgeCloud::getConnected(int cliAddr){
    //add client, updateworkload, report workload
    clientAddr.push_back(cliAddr);
    int totalClient = totalClientNumber();
    totalClientNumber(++totalClient);
    updateWorkload();
}

void
SimulatedEdgeCloud::disconnect(int cliAddr){
    clientAddr.erase(std::remove(clientAddr.begin(), clientAddr.end(),cliAddr), clientAddr.end());
    int totalClient = totalClientNumber();
    totalClientNumber(--totalClient);
    updateWorkload();
}

void
SimulatedEdgeCloud::updateWorkload(){
    double appLoad = totalClientNumber() * _clientLoadWeight;
    appWorkload(appLoad);
    totalWorkload(baseWorkload()+appWorkload());
}

void
SimulatedEdgeCloud::reportWorkload(){
    _controller->updateWorkload(_myAddr, totalWorkload());
}

double
SimulatedEdgeCloud::computeResponseTime(int clientAddr){
    /* calculate cost */
    int clientCloudDistance = std::abs(clientAddr - _myAddr);
    clientCloudDistance =  min(clientCloudDistance, _totalEdgeCloudNumber - clientCloudDistance);
    return _baseResponse + _singleHopCost * clientCloudDistance + totalWorkload() / 0.1 * _unitLoadCost;
}