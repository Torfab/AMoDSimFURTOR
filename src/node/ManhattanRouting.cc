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
#include "Traffic.h"

Define_Module(ManhattanRouting);

void ManhattanRouting::initialize() {
	signalFeromone = new simsignal_t[4];
	/* ---- REGISTER SIGNALS ---- */
	signalFeromone[0] = registerSignal("signalFeromoneN");
	signalFeromone[1] = registerSignal("signalFeromoneE");
	signalFeromone[2] = registerSignal("signalFeromoneS");
	signalFeromone[3] = registerSignal("signalFeromoneW");
	/*****************************/
	signalTraffic = new simsignal_t[4];
	/* ---- REGISTER SIGNALS ---- */
	signalTraffic[0] = registerSignal("signalTrafficN");
	signalTraffic[1] = registerSignal("signalTrafficE");
	signalTraffic[2] = registerSignal("signalTrafficS");
	signalTraffic[3] = registerSignal("signalTrafficW");
	/*****************************/

	myAddress = getParentModule()->par("address");
	myX = getParentModule()->par("x");
	myY = getParentModule()->par("y");
	rows = getParentModule()->getParentModule()->par("width");
	columns = getParentModule()->getParentModule()->par("height");

	xChannelLength = getParentModule()->getParentModule()->par("xNodeDistance");
	yChannelLength = getParentModule()->getParentModule()->par("yNodeDistance");


	EV << "I am node " << myAddress << ". My X/Y are: " << myX << "/" << myY
				<< endl;

	//lastUpdateTime = simTime().dbl();

	//Pheromone
	pheromoneDecayTime = getParentModule()->getParentModule()->par(
			"pheromoneDecayTime");
	pheromoneDecayFactor = getParentModule()->getParentModule()->par(
			"pheromoneDecayFactor");

	pheromone = new Pheromone(pheromoneDecayTime, pheromoneDecayFactor);

	pheromoneEmergency = new Pheromone(pheromoneDecayTime,pheromoneDecayFactor);

	// Traffic
	traffic = new Traffic();

	// Topology
	topo = new cTopology("topo");
	std::vector<std::string> nedTypes;
	nedTypes.push_back("src.node.Node");
	topo->extractByNedTypeName(nedTypes);
}

ManhattanRouting::~ManhattanRouting() {
	delete pheromone;
	delete topo;
}

bool ManhattanRouting::checkAvailableGate(int proposal){
	// Check if gates exist
	cTopology::Node *node = topo->getNode(myAddress);
	for (int j = 0; j < node->getNumOutLinks(); j++) {
				cGate *gate = node->getLinkOut(j)->getLocalGate();
				if (proposal == gate->getIndex())
					return true;
			}
	return false;
}

void ManhattanRouting::handleMessage(cMessage *msg) {
	Vehicle *pk = check_and_cast<Vehicle *>(msg);
	int destAddr = pk->getDestAddr();
	int trafficWeight = pk->getTrafficWeight();
	//If this node is the destination, forward the vehicle to the application level
	if (destAddr == myAddress) {
		EV << "Vehicle arrived in the stop point " << myAddress	<< ". Traveled distance: " << pk->getTraveledDistance()		<< endl;
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

		traffic->decay(pkChosenGate,trafficWeight);

	} else {
		int destX = pk->getDestAddr() % rows;
		int destY = pk->getDestAddr() / rows;

		// todo: memorizzare tempo e decadimento feromone
		// non e' la verita': il feromone viene aggiornato solo quando un veicolo attraversa il nodo. Il valore e' corretto ma non aggiornato in maniera continua nel tempo

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




		if (myX < destX && checkAvailableGate(1)) {
			pk->setChosenGate(1); //right


		} else if (myX > destX && checkAvailableGate(3)) {
			pk->setChosenGate(3); //left

		} else if (myY < destY && checkAvailableGate(2)) {
			pk->setChosenGate(2); //sud

		} else if (checkAvailableGate(0)){
			pk->setChosenGate(0); //north
		}
		else
			return;

		// Traffic delay logic

		int distanceToTravel = 0;
		if (pk->getChosenGate() % 2 == 1)  	// Odd gates are horizontal
			distanceToTravel = xChannelLength;
		else
			// Even gates are vertical
			distanceToTravel = yChannelLength;


		simtime_t channelTravelTime = distanceToTravel / pk->getSpeed();

//		(xNodeDistance)/(speed)
		simtime_t trafficDelay = simTime().dbl() + (distanceToTravel / pk->getSpeed()) * (traffic->trafficInfluence(pk->getChosenGate())) ; //TODO: (check) FIX:
		if (trafficDelay < simTime() )
			trafficDelay = simTime(); // .dbl() doesn't work


		EV << "Messaggio ritardato a " << trafficDelay + channelTravelTime  << " di " << trafficDelay - simTime().dbl() << " s" << "  Traffic infl:" << (traffic->trafficInfluence(pk->getChosenGate())) << endl;
		EV << "++Travel Time: " << channelTravelTime << endl;
		scheduleAt(channelTravelTime + trafficDelay, msg);



		// Update Pheromone and Traffic
		pheromone->increasePheromone(pk->getChosenGate());
		traffic->increaseTraffic(pk->getChosenGate(),pk->getTrafficWeight());

		// Emit pheromone signal
		emit(signalFeromone[pk->getChosenGate()], pheromone->getPheromone(pk->getChosenGate()));

		// Emit traffic signal
		emit(signalTraffic[pk->getChosenGate()], traffic->getTraffic(pk->getChosenGate()));

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


	}
}
