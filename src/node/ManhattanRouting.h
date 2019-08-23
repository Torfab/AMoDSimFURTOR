/*
########################################################
##           __  __       _____   _____ _             ##
##     /\   |  \/  |     |  __ \ / ____(_)            ##
##    /  \  | \  / | ___ | |  | | (___  _ _ __ ___    ##
##   / /\ \ | |\/| |/ _ \| |  | |\___ \| | '_ ` _ \   ##
##  / ____ \| |  | | (_) | |__| |____) | | | | | | |  ##
## /_/    \_\_|  |_|\___/|_____/|_____/|_|_| |_| |_|  ##
##                                                    ##
## Author:                                            ##
##    Andrea Di Maria                                 ##
##    <andrea.dimaria90@gmail.com>                    ##
########################################################
*/

#ifndef __AMOD_SIMULATOR_MANHATTANROUTING_H_
#define __AMOD_SIMULATOR_MANHATTANROUTING_H_

#include <omnetpp.h>
#include "Pheromone.h"

class ManhattanRouting : public cSimpleModule
{
private:
    int myAddress;
    int myX;
    int myY;
    int rows;
    int columns;
    double xChannelLength;
    double yChannelLength;



    double feromone_N;
    double feromone_S;
    double feromone_W;
    double feromone_E;

	// Feromone
	double pheromoneDecayTime;
	double pheromoneDecayFactor;
	Pheromone *pheromone;// = nullptr;
	// Traffico
	double traffic_N;
	double traffic_S;
	double traffic_W;
    double traffic_E;

    double intervalloDecadimentoTraffico;
    double fattoreDecadimentoTraffico;

    double lastUpdateTime;

    //Feromone related signals
    simsignal_t signalFeromoneE;
    simsignal_t signalFeromoneN;
    simsignal_t signalFeromoneS;
    simsignal_t signalFeromoneW;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
