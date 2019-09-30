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

#include "AAAcivilACOambulance.h"
#include "Vehicle.h"
#include "Pheromone.h"
#include "Traffic.h"

Define_Module(AAAcivilACOambulance);

void AAAcivilACOambulance::initialize() {
	netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getParentModule()->getSubmodule("netmanager"));

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
	pheromoneDecayTime = getParentModule()->getParentModule()->par("pheromoneDecayTime");
	pheromoneDecayFactor = getParentModule()->getParentModule()->par("pheromoneDecayFactor");
	decayPheromoneValue = registerSignal("decayPheromoneValue");


	pheromone = new Pheromone(pheromoneDecayTime, pheromoneDecayFactor);

	pheromoneEmergency = new Pheromone(pheromoneDecayTime,pheromoneDecayFactor);

	// Traffic
	traffic = new Traffic();

	//Subscription per pheromon decay
	simulation.getSystemModule()->subscribe("decayPheromoneValue", this);

}

AAAcivilACOambulance::~AAAcivilACOambulance() {
	delete pheromone;
	delete pheromoneEmergency;
	delete traffic;
}


void AAAcivilACOambulance::handleMessage(cMessage *msg) {
	Vehicle *pk = check_and_cast<Vehicle *>(msg);
	int destAddr = pk->getDestAddr();
	int trafficWeight = pk->getWeight();

	// Topologies
	cTopology* topo = netmanager->getTopo();
	cTopology* topoEmergency = netmanager->getTopoEmergency();

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
		// Emit traffic signal
		emit(signalTraffic[pk->getChosenGate()], traffic->getTraffic(pk->getChosenGate()));

	} else {

		//Weighted Dijkstra topo
		int destination = pk->getDestAddr();
		cTopology::Node *node = topo->getNode(myAddress);
		cTopology::Node *nodeEmergency = topoEmergency->getNode(myAddress);

		cTopology::Node *targetnode = topo->getNode(destination);
		cTopology::Node *targetnodeEmergency = topoEmergency->getNode(destination);

//		 Assegna il peso del traffico corrente (escluso il veicolo nuovo) ai canali in uscita
		for (int i = 0; i < node->getNumOutLinks(); i++) {
			node->getLinkOut(i)->setWeight(1 + (pheromone->getPheromone(i) / 20));  // AAA Civil
			nodeEmergency->getLinkOut(i)->setWeight(netmanager->getStartingChannelWeight() - pheromone->getPheromone(i)); // ACO Ambulance
		}

		//Weighted or unweighted dijkstra to target

		//If it's an ambulance
		if (pk->getSpecialVehicle() == 1){
			topoEmergency->calculateWeightedSingleShortestPathsTo(targetnodeEmergency);
			if (nodeEmergency->getNumPaths() == 0){
				EV << "I am: " << myAddress << " No path to destination.My dest is: " << targetnode->getModule()->getFullPath()  << endl;
				for (int i = 0; i < node->getNumOutLinks(); i++) {
					EV << "Local gates: " << node->getLinkOut(i)->getLocalGate()->getFullPath() << endl;
				}
				return;
			}
		}
		//else it's a civil or truck
		else{
			topo->calculateWeightedSingleShortestPathsTo(targetnode);
			if (node->getNumPaths() == 0 )  {
				EV << "No path to destination. My dest is:" << targetnode->getModule()->getFullPath()  << endl;
				pk->setDestAddr(netmanager->getClosestExitNode(myAddress));
				int destination = pk->getDestAddr();
				cTopology::Node *targetnode = topo->getNode(destination);
				topo->calculateWeightedSingleShortestPathsTo(targetnode);
			}
		}

			//there are paths available

		int id;

		if (pk->getSpecialVehicle() == 1) {
			cTopology::LinkOut *pathEmergency = nodeEmergency->getPath(0);
			pk->setChosenGate(pathEmergency->getLocalGate()->getIndex());
			//Need to save the LinkOut id of the localgate to id
			for (id = 0;
					id < topoEmergency->getNode(myAddress)->getNumOutLinks();
					id++) {
				if (topoEmergency->getNode(myAddress)->getLinkOut(id)->getLocalGate()->getIndex() == pk->getChosenGate())
					break;
			}
		}

		else {
			cTopology::LinkOut *path = node->getPath(0);
			pk->setChosenGate(path->getLocalGate()->getIndex());
			//Need to save the LinkOut id of the localgate to id
			for (id = 0; id < topo->getNode(myAddress)->getNumOutLinks();
					id++) {
				if (topo->getNode(myAddress)->getLinkOut(id)->getLocalGate()->getIndex() == pk->getChosenGate())
					break;
			}
		}

		// Update Pheromone and Traffic
		pheromone->increasePheromone(pk->getChosenGate(), pk->getWeight());
		traffic->increaseTraffic(pk->getChosenGate(), pk->getWeight());

		int pkChosenGate = pk->getChosenGate();
		// Update weights
		topo->getNode(myAddress)->getLinkOut(id)->setWeight(1 + (pheromone->getPheromone(pkChosenGate) / 20)); // AAA
		topoEmergency->getNode(myAddress)->getLinkOut(id)->setWeight(netmanager->getStartingChannelWeight() - pheromone->getPheromone(pkChosenGate)); // ACO

		// Traffic delay logic

		int distanceToTravel = 0;
		if (pk->getChosenGate() % 2 == 1)  	// Odd gates are horizontal
			distanceToTravel = xChannelLength;
		else
			// Even gates are vertical
			distanceToTravel = yChannelLength;

		simtime_t channelTravelTime = distanceToTravel / pk->getSpeed();

		simtime_t trafficDelay = simTime().dbl() + (distanceToTravel / pk->getSpeed()) * (1 + (traffic->trafficInfluence(pk->getChosenGate())));
		if (trafficDelay < simTime())
			trafficDelay = simTime(); // .dbl() doesn't work

		pk->setCurrentTraveledTime(pk->getCurrentTraveledTime() + channelTravelTime.dbl() + trafficDelay.dbl() - simTime().dbl());

		EV << "Messaggio ritardato a " << trafficDelay + channelTravelTime << " di " << trafficDelay - simTime().dbl() << " s" << "  Traffic infl:" << (traffic->trafficInfluence(pk->getChosenGate())) << endl;
		EV << "++Travel Time: " << channelTravelTime << endl;
		scheduleAt(channelTravelTime + trafficDelay, msg);

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

		pk->setHopCount(pk->getHopCount() + 1);
	}
}



void AAAcivilACOambulance::receiveSignal(cComponent* source, simsignal_t signalID, bool value) {
	if (signalID == decayPheromoneValue) {
		pheromone->decayPheromone();
		// Emit pheromone signal
		for (int i = 0; i<4;i++)
		emit(signalFeromone[i], pheromone->getPheromone(i));
	}
}
