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

#include "DijkstraUnweighted.h"
#include "Vehicle.h"
#include "Pheromone.h"
#include "Traffic.h"

Define_Module(DijkstraUnweighted);

void DijkstraUnweighted::initialize() {

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

	EV << "I am node " << myAddress << ". My X/Y are: " << myX << "/" << myY << endl;

	//Pheromon related parameters
	pheromoneDecayTime = getParentModule()->getParentModule()->par("pheromoneDecayTime");
	pheromoneDecayFactor = getParentModule()->getParentModule()->par("pheromoneDecayFactor");
	decayPheromoneValue = registerSignal("decayPheromoneValue");

	pheromone = new Pheromone(pheromoneDecayTime, pheromoneDecayFactor);
	pheromoneEmergency = new Pheromone(pheromoneDecayTime, pheromoneDecayFactor);

	// Traffic
	traffic = new Traffic();

	//Subscription per pheromon decay
	simulation.getSystemModule()->subscribe("decayPheromoneValue", this);
}

DijkstraUnweighted::~DijkstraUnweighted() {
	delete pheromone;
	delete pheromoneEmergency;
	delete traffic;
}

void DijkstraUnweighted::handleMessage(cMessage *msg) {

	Vehicle *pk = check_and_cast<Vehicle *>(msg);
	int destAddr = pk->getDestAddr();
	int trafficWeight = pk->getWeight(); //get vehicle weight

	// Topology from netmanager
	cTopology* topo = netmanager->getTopo();

	//If this node is the destination, forward the vehicle to the application level
	if (destAddr == myAddress) {
		EV << "Vehicle arrived in the stop point " << myAddress << ". Traveled distance: " << pk->getTraveledDistance() << endl;
		send(pk, "localOut");
		return;
	}
	//The vehicle has waited a delay to simulate the traffic in chosen channel
	if (msg->isSelfMessage()) {
		int pkChosenGate = pk->getChosenGate();
		pk->setHopCount(pk->getHopCount() + 1);

		int distance = 0;
		if (pkChosenGate % 2 == 1)  	// Odd gates are horizontal
			distance = xChannelLength;
		else							// Even gates are vertical
			distance = yChannelLength;

		pk->setTraveledDistance(pk->getTraveledDistance() + distance); // updates traveled distance

		send(pk, "out", pkChosenGate);	//Send the vehicle to the next node
		// Decay the traffic
		traffic->decay(pkChosenGate, trafficWeight);
		// Emit traffic signal
		emit(signalTraffic[pk->getChosenGate()], traffic->getTraffic(pk->getChosenGate()));

	} else {

		int destination = pk->getDestAddr();
		EV << "I'm going to" << pk->getDestAddr();

		cTopology::Node *node = topo->getNode(myAddress);
		cTopology::Node *targetnode = topo->getNode(destination);

		topo->calculateUnweightedSingleShortestPathsTo(targetnode); // Unweighted Dijkstra to target

		if (node->getNumPaths() == 0) {
			EV << "No path to destination.\n";
			return;
		} else {

			cTopology::LinkOut *path = node->getPath(0);
			EV << "Taking gate " << path->getLocalGate()->getFullName() << " we arrive in " << path->getRemoteNode()->getModule()->getFullPath() << " on its gate " << path->getRemoteGate()->getFullName() << endl;
			pk->setChosenGate(path->getLocalGate()->getIndex());
		}

		// Traffic delay logic
		int distanceToTravel = 0;
		if (pk->getChosenGate() % 2 == 1)  	// Odd gates are horizontal
			distanceToTravel = xChannelLength;
		else
											// Even gates are vertical
			distanceToTravel = yChannelLength;

		simtime_t channelTravelTime = distanceToTravel / pk->getSpeed(); // Calculates the time to travel the channel

		simtime_t trafficDelay = simTime().dbl() + (distanceToTravel / pk->getSpeed()) *(1 + (traffic->trafficInfluence(pk->getChosenGate())));
		if (trafficDelay < simTime())
			trafficDelay = simTime(); // .dbl() doesn't work

		pk->setCurrentTraveledTime(pk->getCurrentTraveledTime() + channelTravelTime.dbl() + trafficDelay.dbl() - simTime().dbl());

		EV << "Message delayed to " << trafficDelay + channelTravelTime << " of " << trafficDelay - simTime().dbl() << " s" << "  Traffic influence:" << (traffic->trafficInfluence(pk->getChosenGate())) << endl;
		EV << "- Channel Travel Time: " << channelTravelTime << endl;
		scheduleAt(channelTravelTime + trafficDelay, msg);

		// Update Pheromone and Traffic
		pheromone->increasePheromone(pk->getChosenGate(), pk->getWeight());
		traffic->increaseTraffic(pk->getChosenGate(), pk->getWeight());





		// Emit pheromone signal
		emit(signalFeromone[pk->getChosenGate()], pheromone->getPheromone(pk->getChosenGate()));
		// Emit traffic signal
		emit(signalTraffic[pk->getChosenGate()], traffic->getTraffic(pk->getChosenGate()));


		EV << "Node " << myAddress << " Pheromon N E S W: ";
		for (int i = 0; i < 4; i++) {
			EV << pheromone->getPheromone(i) << " || ";
		}
		EV << endl;

		EV << "Node " << myAddress << " Traffic N E S W: ";
		for (int i = 0; i < 4; i++) {
			EV << traffic->getTraffic(i) << " || ";
		}
		EV << endl;
		// updates hop
		pk->setHopCount(pk->getHopCount() + 1);
	}
}
void DijkstraUnweighted::receiveSignal(cComponent* source, simsignal_t signalID, bool value) {
	if (signalID == decayPheromoneValue) {
		pheromone->decayPheromone();
		// Emit pheromone signal
		for (int i = 0; i<4;i++)
		emit(signalFeromone[i], pheromone->getPheromone(i));
	}

}
