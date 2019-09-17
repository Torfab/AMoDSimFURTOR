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
 ## Extended by:                                       ##
 ##    Francesco Furnari                               ##
 ##    <franz.furnari@gmail.com>                       ##
 ##    Fabrizio Tornatore                              ##
 ##    <torfab94@gmail.com>                            ##
 ##                                                    ##
 ########################################################
 */
#include <omnetpp.h>
#include "BaseCoord.h"
#include "AbstractNetworkManager.h"
#include <algorithm>
#include <vector>

class NodeInitializer: public cSimpleModule {

private:
	// configuration
	int myAddress;
	int x_coord;
	int y_coord;

	AbstractNetworkManager *netmanager;

public:
	NodeInitializer();
	virtual ~NodeInitializer();

protected:
	virtual void initialize();
	bool disconnectChannelsAndCheckRedzone();
	bool propagateDistance(int distance);	//Destroy link propagation function
};

Define_Module(NodeInitializer);

NodeInitializer::NodeInitializer() {
	netmanager = NULL;
}

NodeInitializer::~NodeInitializer() {

}


/**
 * Return true if the value between 0 and (4 * distance) - 4 is == 0
 * -4 because we always want that an epicenter destroys every links
 *
 */
bool NodeInitializer::propagateDistance(int distance) {
	return intuniform(0, 4 * (distance) - 4) == 0;
}

/*
 * Disconnect channels
 * Check if the node is in redzone and return the bool
 *
 */
bool NodeInitializer::disconnectChannelsAndCheckRedzone() {

	bool hospital = netmanager->checkHospitalNode(myAddress);
	bool storagePoint = netmanager->checkStoragePointNode(myAddress);
	bool collectionPoint = netmanager->checkCollectionPointNode(myAddress);

	bool redZoneNode = false;

	int rows; //of the grid
	rows = getParentModule()->getParentModule()->par("width");
	int myX = myAddress % rows;
	int myY = myAddress / rows;

	// Destroying nodes part
	cTopology *topo = netmanager->getTopo();

	cTopology::Node* node = topo->getNode(myAddress);

	std::set<int> setOfEpicenters = netmanager->getSetOfEpicenters();

	// Each nodes has a chance of destroying the links, for each epicenter, based on the distance from him and the epicenter.
	for (auto elem : setOfEpicenters) {
		int epicX = elem % rows;
		int epicY = elem / rows;

		if (!hospital && !collectionPoint && !storagePoint) {

			int distance;

			for (int j = 0; j < node->getNumOutLinks(); j++) { //Looking at out connections

				switch (node->getLinkOut(j)->getLocalGate()->getIndex()) {

				case 1:  	// 1 is EAST
					distance = netmanager->getManhattanDistance(myAddress, elem);

					if (myX >= epicX) // if the node is >= the epicenter, since we're breaking right, it's like we do an hop more
						distance++;

					if (propagateDistance(distance)) { //if the propagation function returns true, it disconnects the node

						cGate *gate = node->getLinkOut(j)->getLocalGate();
						gate->disconnect();
						netmanager->insertRedZoneNode(myAddress);
						redZoneNode = true;

					}
					break;
				case 2: 	// SOUTH
					distance = netmanager->getManhattanDistance(myAddress, elem);

					if (myY >= epicY) // if the node is >= the epicenter, since we're breaking south, it's like we do an hop more
						distance++;

					if (propagateDistance(distance)) {

						cGate *gate = node->getLinkOut(j)->getLocalGate();
						gate->disconnect();
						netmanager->insertRedZoneNode(myAddress);
						redZoneNode = true;
					}
					break;

				default:
					break;
				}
			}

		}

		// Simmetric check to link in
		int guardiaW = -1;
		int guardiaN = -1;

		for (int j = 0; j < node->getNumInLinks(); j++) {
//			ev << "index del nodo" << node->getModule()->getIndex() << " : in: " << node->getLinkIn(j)->getLocalGate()->getIndex() << endl;

			if (node->getLinkIn(j)->getLocalGate()->getIndex() == 3 && node->getLinkIn(j)->getLocalGate()->isConnected()) {
				guardiaW = 1; //channel exists, nothing happens

			}
			if (node->getLinkIn(j)->getLocalGate()->getIndex() == 0 && node->getLinkIn(j)->getLocalGate()->isConnected()) {
				guardiaN = 1; //channel exists, nothing happens

			}

		}
		if (guardiaW == -1) {

			//if there isn't a link in, destroy the link out on West
			for (int k = 0; k < node->getNumOutLinks(); k++) {
				if (node->getLinkOut(k)->getLocalGate()->getIndex() == 3) {	// check if there's a linkout to a node
					cGate *gate = node->getLinkOut(k)->getLocalGate();
					gate->disconnect();
					netmanager->insertRedZoneNode(myAddress);
					redZoneNode = true;

				}
			}
		}
		if (guardiaN == -1) {

			//if there isn't a link in, destroy the link out on North
			for (int k = 0; k < node->getNumOutLinks(); k++) {
				if (node->getLinkOut(k)->getLocalGate()->getIndex() == 0) {
					cGate *gate = node->getLinkOut(k)->getLocalGate();
					gate->disconnect();
					netmanager->insertRedZoneNode(myAddress);
					redZoneNode = true;
				}
			}
		}
	}

	int disconnected = 0; //the node starts connected
	for (int k = 0; k < node->getNumOutLinks(); k++) { //check out every link out

		cGate *gate = node->getLinkOut(k)->getLocalGate();

		if (!gate->isConnected()) { //count the number of disconnected gates
			disconnected++;
		}
	}

	if (disconnected == node->getNumOutLinks()) { //if the number of disconnected gates is equal to the number of the out links
		netmanager->insertDestroyedNode(myAddress);	// put the node in the destroyed set
		netmanager->removeRedZoneNode(myAddress);	// remove it from red zone
		redZoneNode = false;
	}

	return redZoneNode;
}



void NodeInitializer::initialize() {
	myAddress = par("address");
	netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getParentModule()->getSubmodule("netmanager"));

	bool redZoneNode = disconnectChannelsAndCheckRedzone();

	if (redZoneNode && !netmanager->checkHospitalNode(myAddress)) {
		if (ev.isGUI())
			getParentModule()->getDisplayString().setTagArg("b", 3, "red");
	}
	if (netmanager->checkDestroyedNode(myAddress)) { // Look and feel
		if (ev.isGUI()) {
			getParentModule()->getDisplayString().setTagArg("b", 0, "0");
			getParentModule()->getDisplayString().setTagArg("b", 1, "0");
		}
	}

	//updates the topology in network manager
	netmanager->updateTopology(netmanager->getTopo(), 1);
	netmanager->updateTopology(netmanager->getTopoEmergency(), netmanager->getStartingChannelWeight()); //ACO
}

