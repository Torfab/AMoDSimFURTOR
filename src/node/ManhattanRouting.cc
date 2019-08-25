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

void ManhattanRouting::initialize() {
	signalFeromone = new simsignal_t[4];
	/* ---- REGISTER SIGNALS ---- */
	signalFeromone[0] = registerSignal("signalFeromoneN");
	signalFeromone[1] = registerSignal("signalFeromoneE");
	signalFeromone[2] = registerSignal("signalFeromoneS");
	signalFeromone[3] = registerSignal("signalFeromoneW");
	/*****************************/

	myAddress = getParentModule()->par("address");
	myX = getParentModule()->par("x");
	myY = getParentModule()->par("y");
	rows = getParentModule()->getParentModule()->par("width");
	columns = getParentModule()->getParentModule()->par("height");

	xChannelLength = getParentModule()->getParentModule()->par("xNodeDistance");
	yChannelLength = getParentModule()->getParentModule()->par("yNodeDistance");

	speed = 9.7; //wsrediojpkugtretipjkyt'pi9oertgijpokertgopèjkretojipèerkjoèrfwèekqlporfkelèwporfèklpwerfèplkewèlrkpewlèp

	EV << "I am node " << myAddress << ". My X/Y are: " << myX << "/" << myY
				<< endl;

	//lastUpdateTime = simTime().dbl();

	//Pheromone
	pheromoneDecayTime = getParentModule()->getParentModule()->par(
			"pheromoneDecayTime");
	pheromoneDecayFactor = getParentModule()->getParentModule()->par(
			"pheromoneDecayFactor");

	pheromone = new Pheromone(pheromoneDecayTime, pheromoneDecayFactor);

	traffic = new Traffic();
}

ManhattanRouting::~ManhattanRouting() {
	delete pheromone;
}

void ManhattanRouting::handleMessage(cMessage *msg) {
	Vehicle *pk = check_and_cast<Vehicle *>(msg);
	int destAddr = pk->getDestAddr();

	//If this node is the destination, forward the vehicle to the application level
	if (destAddr == myAddress) {
		EV << "Vehicle arrived in the stop point " << myAddress
					<< ". Traveled distance: " << pk->getTraveledDistance()
					<< endl;
		send(pk, "localOut");
		return;
	}
	if (msg->isSelfMessage()) { //The vehicle has waited a delay to simulate the traffic in chosen channel

		int pkChosenGate = pk->getChosenGate();
		pk->setHopCount(pk->getHopCount() + 1);

		int distance = 0;
		if (pkChosenGate % 2 == 1)  	// Odd gates are horizontal
			distance = xChannelLength;
		else
			// Even gates are vertical
			distance = yChannelLength;

		pk->setTraveledDistance(pk->getTraveledDistance() + distance);
		//send the vehicle to the next node
		send(pk, "out", pkChosenGate);

		traffic->decay(pkChosenGate);

	} else {

//    simtime_t st= simTime().dbl() + 10;
//    if (intuniform(0, 10, 0) % 2 == 0) {
//    	scheduleAt(st, msg);
//    }
//    if (msg->isSelfMessage()){
//    	EV << 'SELF!!' << endl;
//    }

//    int distance; 		// vecchia variabile
//		int outGateIndex; 	// vecchia variabile
		int destX = pk->getDestAddr() % rows;
		int destY = pk->getDestAddr() / rows;

		// todo: memorizzare tempo e decadimento feromone

		// non e' la verita': il feromone viene aggiornato solo quando un veicolo attraversa il nodo. Il valore e' corretto ma non aggiornato in maniera continua nel tempo

		//double nextDecadenceTime = decadimentoFeromone * fattoreFeromone

		int n = (simTime().dbl() - lastUpdateTime) / pheromoneDecayTime;

		if (n != 0) {
			EV << "n: [ " << n << " ]" << "=" << simTime().dbl() << "-"
						<< lastUpdateTime << "/" << pheromoneDecayTime << endl;
			for (int i = 0; i < n; i++) {
				pheromone->decayPheromone();
			}
			for (int i = 0; i < pheromone->getNumberOfGates(); i++) {
				emit(signalFeromone[i], pheromone->getPheromone(i));
			}

			lastUpdateTime = simTime().dbl();
		}

		if (myX < destX) {
			pheromone->increasePheromone(1);
			traffic->increaseTraffic(1);
			pk->setChosenGate(1); //right
//        distance = xChannelLength;
			emit(signalFeromone[1], pheromone->getPheromone(1));

		} else if (myX > destX) {
			pheromone->increasePheromone(3);
			traffic->increaseTraffic(3);
			pk->setChosenGate(3); //left

//            distance = xChannelLength;
			emit(signalFeromone[3], pheromone->getPheromone(3));

		} else if (myY < destY) {
			pheromone->increasePheromone(2);
			traffic->increaseTraffic(2);

			pk->setChosenGate(2); //sud

//            distance = yChannelLength;
			emit(signalFeromone[2], pheromone->getPheromone(2));

		} else {
			pheromone->increasePheromone(0);
			traffic->increaseTraffic(0);

			pk->setChosenGate(0); //north
//            distance = yChannelLength;
			emit(signalFeromone[0], pheromone->getPheromone(0));
		}

		EV << "Nodo " << myAddress << " Pheromone N E S W: ";
		for (int i = 0; i < 4; i++) {
			EV << pheromone->getPheromone(i) << " || ";
		}
		EV << endl;

		EV << "Nodo " << myAddress << " Traffico N E S W: ";
		for (int i = 0; i < 4; i++) {
			EV << traffic->getTraffic(i) << " || ";
		}
		EV << endl;

//    pk->setHopCount(pk->getHopCount()+1);
//    pk->setTraveledDistance(pk->getTraveledDistance() + distance);
//
//    //send the vehicle to the next node
//    send(pk, "out", outGateIndex);

		// Traffic delay logic
		// Send self message

		int distanceToTravel = xChannelLength;

//		(xNodeDistance)/(speed)
		simtime_t trafficDelay = simTime().dbl() + (distanceToTravel / speed) * (traffic->trafficInfluence((pk->getChosenGate())-1));
		EV << "Messaggio ritardato per " << trafficDelay  << " di " << trafficDelay - simTime().dbl() << " s" << "  Traffic infl:" << (traffic->trafficInfluence((pk->getChosenGate())-1)) << endl;
		scheduleAt(trafficDelay, msg);
	}
}
