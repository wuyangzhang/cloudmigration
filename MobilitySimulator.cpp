//
//  MobilitySimulator.cpp
//  MarkovProcess
//
//  Created by Wuyang on 5/7/16.
//  Copyright © 2016 Wuyang. All rights reserved.
//
/*
    version 1.2 revisions:
    1. client add personalized mobilityPattern & mobilieTrace & timeSlot
    2. server manage range, each server support near N client locations
 
*/

#include "MobilitySimulator.hpp"

MobilitySimulator::MobilitySimulator(int totalCloud, int totalClientNumber, int totalClientPosition){
    _totalCloud = totalCloud;
    _totalClientNumber = totalClientNumber;
    _totalClientPosition = totalClientPosition;
    _pattern.left = 0, _pattern.stay = 0, _pattern.right = 1;
    _readModel = false;
    _queryModel = 0; //0-Markov query, 1-workload query, 2-neverMigrate, 3-closest // in controller, runMarkov() to change nerver migrate.
    
    _hopCost = 22;
    _baseResp = 120;
}

MobilitySimulator::~MobilitySimulator(){
    delete _controller;
}

/*
    time slot based simulation: 
*/
void
MobilitySimulator::simulate(){
    //initialize mdp
    MarkovProcess* mdp = new MarkovProcess(_totalCloud,_totalClientPosition);
    mdp->setGamma(0);
    mdp->setTransitionProbability(_pattern.left, _pattern.stay, _pattern.right);
    mdp->singleHopCost = _hopCost;
    mdp->baseResponse = _baseResp;
    
    _controller = new SimulatedCentralController(mdp);

    //initialize edge clouds
    for(int i = 0; i < _totalCloud; i++){
        SimulatedEdgeCloud* cloud = new SimulatedEdgeCloud(i);
        _controller->cloudList()->push_back(cloud);
    }
    
    for(vector<SimulatedEdgeCloud*>::iterator it = _controller->cloudList()->begin(); it != _controller->cloudList()->end(); ++it){
        (*it)->init(_controller,_readModel, _hopCost, _baseResp);
    }
    
    //initialize clients
    for(int i = 0; i < _totalClientNumber; i++){
        SimulatedClient* client;
        if(!_readModel){
            client= new SimulatedClient(i, _totalClientPosition, _totalCloud, 1, _pattern.left, _pattern.stay, _pattern.right); // pathlength, mobilityPattern
        }
        else{
            // readModel uses real trace, read mobility pattern, read
            client= new SimulatedClient(i, _totalClientPosition, _totalCloud);
        }
        
        _controller->clientList()->push_back(client);
        client->init(_controller->cloudList(), _controller);
    }
    
    /*
        start simulate mobility based on timeSlot
    */
    
    for ( int timeSlot = 0; timeSlot < 1; timeSlot++){
        /*
        //check the clients in the current timeSlot
        vector<int>* client_slot = _controller->checkClient_timeSlot(timeSlot);
        vector<double> responseTimeCollect;
        // client_slot->at(i) represent one client id
        cout<<"timeSlot: "<<timeSlot<<endl;
        for( int i = 0; i< client_slot->size(); i++){
            SimulatedClient* client = _controller->clientList()->at(client_slot->at(i));
            //client update its current location
            client->updateCurrentLocation_TimeSlot(timeSlot);
            
            double responseTime = 0.0;
            bool firstConnect = client->firstConnect();
            int previousServer = 0;
            if(firstConnect){
                previousServer = 0;
                client->queryConnectServer(3); // first query always return the nearest server
                client->connectServer(client->migrateServer());
            }else{
                previousServer = client->connectedServer();
                //client->disconnectServer(client->connectedServer());
                client->connectServer(client->migrateServer());
                responseTime = client->computeResponseTime();
                responseTimeCollect.push_back(responseTime);
                
            }
            
            client->queryConnectServer(_queryModel);
            
            if(_queryModel == 0){
                _controller->cloudList()->at(client->connectedServer())->reportConnectedClient();
                _controller->cloudList()->at(client->migrateServer())->reportConnectedClient();
                _controller->updateMDP_TransitionProbability(client->getMobilityPattern());
                _controller->runMarkovDecision();
            }
            
            
            cout<<"c"<<client_slot->at(i)<<" rsp:"<<responseTime<<" l"<<client->currentClientPosition()<<" svr:"<<client->connectedServer()<<" num:"<<_controller->cloudList()->at(client->connectedServer())->totalClientNumber()<<" m"<<client->_nextConnectedServerName<<endl;
            
            
           
        }
        
        for( int i = 0; i< client_slot->size(); i++){
            SimulatedClient* client = _controller->clientList()->at(client_slot->at(i));
            client->disconnectServer(client->connectedServer());
            _controller->cloudList()->at(client->connectedServer())->reportConnectedClient();

        }
        
        double totalResponseTime = 0;
        for(int i = 0; i< responseTimeCollect.size(); i++){
            totalResponseTime += responseTimeCollect.at(i);
        }
        
        double averageResponseTime = totalResponseTime / responseTimeCollect.size();
        
        cout <<timeSlot<<","<<averageResponseTime<<endl;
        _controller->addTimeSlotResponseTime(averageResponseTime);
        responseTimeCollect.clear();

    }
    
    _controller->printTimeSlotResponseTime();
    _controller->printServiceCount();
    
    
    _controller->computeAverageResponseTime();
    //_controller->printAverageResponseTime();
         
    */
        int round = 0;
    for(auto i = 0; i < _controller->clientList()->size(); i++){
        int serverConnect = 0;
        SimulatedClient* client = _controller->clientList()->at(i);
        if(_queryModel != 2){
            serverConnect =  client->queryConnectServer(1); // first query always return the server with lowest load.
            client->connectServer(client->migrateServer());
        }else{
            client->connectServer(client->generateRandomServer());
        }
        
        serverConnect =  client->queryConnectServer(_queryModel);
        //printf("client postion %d, query server %d\n", client->currentClientPosition(), serverConnect);
        // exclusive event of MDP. update server info
        if(_queryModel == 0){
            _controller->cloudList()->at(client->connectedServer())->reportConnectedClient();
            _controller->cloudList()->at(client->migrateServer())->reportConnectedClient();
            _controller->runMarkovDecision();
        }

        _controller->getMDP()->printOptimizedActions();

        client->computeResponseTime();
        client->moveToNextClientPosition();
    }
    //_controller->printConnectedClient();
         
   
            
    while( !_controller->clientList()->back()->terminateMove()){
        round++;
        //printf("step %d \n", round++);
        for(auto i = 0; i < _controller->clientList()->size(); i++){
            SimulatedClient* client = _controller->clientList()->at(i);
            int server = 0;
        
            client->disconnectServer(client->connectedServer());
            client->connectServer(client->migrateServer());
            server = client->queryConnectServer(_queryModel);
            
            if(_queryModel == 0){
                _controller->cloudList()->at(client->connectedServer())->reportConnectedClient();
                _controller->cloudList()->at(client->migrateServer())->reportConnectedClient();
                _controller->runMarkovDecision();
            }
            
            client->computeResponseTime();
            client->moveToNextClientPosition();
                   }

    }
    
    
    
    //_controller->computeAveragePredictTime();
    //_controller->printAveragePredictTime();
    
    //SimulatedClient* client = _controller->clientList()->at(20);
    //client->printComputeTime();
    }
}

