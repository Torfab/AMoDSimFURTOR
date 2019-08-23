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

#include "ManhattanRouting.h"
#include "Vehicle.h"
#include "Pheromone.h"


Define_Module(ManhattanRouting);

void ManhattanRouting::initialize()
{
    /* ---- REGISTER SIGNALS ---- */
    signalFeromoneE = registerSignal("signalFeromoneE");
    signalFeromoneN = registerSignal("signalFeromoneN");
    signalFeromoneS = registerSignal("signalFeromoneS");
    signalFeromoneW = registerSignal("signalFeromoneW");
    /*****************************/

    myAddress = getParentModule()->par("address");
    myX = getParentModule()->par("x");
    myY = getParentModule()->par("y");
    rows = getParentModule()->getParentModule()->par("width");
    columns = getParentModule()->getParentModule()->par("height");

    xChannelLength = getParentModule()->getParentModule()->par("xNodeDistance");
    yChannelLength = getParentModule()->getParentModule()->par("yNodeDistance");

    EV << "I am node " << myAddress << ". My X/Y are: " << myX << "/" << myY << endl;

    //lastUpdateTime = simTime().dbl();


	pheromoneDecayTime = getParentModule()->getParentModule()->par("intervalloDecadimentoFeromone");
	pheromoneDecayFactor = getParentModule()->getParentModule()->par("fattoreDecadimentoFeromone");

	Pheromone pheromone(pheromoneDecayTime,pheromoneDecayFactor);
}

void ManhattanRouting::handleMessage(cMessage *msg)
{
    Vehicle *pk = check_and_cast<Vehicle *>(msg);
    int destAddr = pk->getDestAddr();


    //If this node is the destination, forward the vehicle to the application level
    if (destAddr == myAddress)
    {
        EV << "Vehicle arrived in the stop point " << myAddress << ". Traveled distance: " << pk->getTraveledDistance() << endl;
        send(pk, "localOut");
        return;
    }


    int distance;
    int outGateIndex;
    int destX = pk->getDestAddr() % rows;
    int destY = pk->getDestAddr() / rows;

    // todo: memorizzare tempo e decadimento feromone

    // non e' la verita': il feromone viene aggiornato solo quando un veicolo attraversa il nodo. Il valore e' corretto ma non aggiornato in maniera continua nel tempo


	//double nextDecadenceTime = decadimentoFeromone * fattoreFeromone

	int n = (simTime().dbl() - lastUpdateTime) / pheromoneDecayTime;


	if (n != 0) {
		EV << "n: [ " << n << " ]" << "=" << simTime().dbl() << "-" <<lastUpdateTime << "/" << pheromoneDecayTime <<endl;
		for (int i = 0; i < n; i++) {
			pheromone->decayPheromone();
			/*
			feromone_E = feromone_E - feromone_E * fattoreDecadimentoFeromone; //pheromone al tempo t+1
			feromone_N = feromone_N - feromone_N * fattoreDecadimentoFeromone;
			feromone_S = feromone_S - feromone_S * fattoreDecadimentoFeromone;
			feromone_W = feromone_W - feromone_W * fattoreDecadimentoFeromone;
			*/
		}
		for (int i = 0; i < pheromone->getNumberOfGates(); i++) {
		emit(signalFeromoneE, pheromone->getPheromone(i));
		}
	/*	emit(signalFeromoneW, feromone_W);
		emit(signalFeromoneS, feromone_S);
		emit(signalFeromoneN, feromone_N);*/

		lastUpdateTime = simTime().dbl();
	}



    if(myX < destX)
    {
    	pheromone->increasePheromone(1);
        //feromone_E++;
        outGateIndex = 2; //right
        distance = xChannelLength;
        emit(signalFeromoneE, feromone_E);
    }
    else
        if(myX > destX)
        {
        	pheromone->increasePheromone(3);
            outGateIndex = 3; //left
            distance = xChannelLength;
    		emit(signalFeromoneW, feromone_W);

        }
    else
        if(myY < destY)
        {
        	pheromone->increasePheromone(2);
            outGateIndex = 0; //sud
            distance = yChannelLength;
    		emit(signalFeromoneS, feromone_S);

        }
        else
        {
        	pheromone->increasePheromone(0);
            outGateIndex = 1; //north
            distance = yChannelLength;
    		emit(signalFeromoneN, feromone_N);
        }
    EV << "Nodo " << myAddress << " feromoni N E S W: " << feromone_N << " | "<< feromone_E << " | " << feromone_S << " | "<< feromone_W << " | " << endl;
    pk->setHopCount(pk->getHopCount()+1);
    pk->setTraveledDistance(pk->getTraveledDistance() + distance);

    //send the vehicle to the next node
    send(pk, "out", outGateIndex);
}
