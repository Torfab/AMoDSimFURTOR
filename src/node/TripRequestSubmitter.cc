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
#include "TripRequest.h"
#include "BaseCoord.h"
#include "AbstractNetworkManager.h"
#include <algorithm>
#include <vector>

class TripRequestSubmitter: public cSimpleModule {

private:
	// configuration
	int myAddress;
	int x_coord;
	int y_coord;

	double maxSubmissionTime;
	double minTripLength;
	int destAddresses;

	cPar *sendIATime;
	cPar *maxDelay;

	AbstractNetworkManager *netmanager;
	BaseCoord *tcoord;

	cMessage *generatePacket;
	cMessage *emergencyPacket;
	cMessage *redEmergencyPacket;
	cMessage *truckPacket;
	//Emergency schedule vector
	std::vector<double> scheduledEmergencies;

	// signals
	simsignal_t tripRequest;
	simsignal_t emergencyRequests;

	int emergencyIndex; //used to browse the vector
	int totalEmergenciesPerNode;

public:
	TripRequestSubmitter();
	virtual ~TripRequestSubmitter();

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual TripRequest* buildEmergencyRequest();
	virtual TripRequest* buildTruckRequest(); 		// Request builder
	virtual TripRequest* buildRedCodeRequest(); 	// Request builder

	void buildEmergencySchedule(int totalEmergencies); // Build the emergency schedule vector
	bool disconnectChannelsAndCheckRedzone();

	bool propagateDistance(int distance);	//Destroy link propagation function
	void scheduleEmergencyOrRedCode();//Schedule the next emergency as normal or red code
};

Define_Module(TripRequestSubmitter);

TripRequestSubmitter::TripRequestSubmitter() {
	generatePacket = NULL;
	emergencyPacket = NULL;
	truckPacket = NULL;
	redEmergencyPacket = NULL;
	netmanager = NULL;
	tcoord = NULL;
}

TripRequestSubmitter::~TripRequestSubmitter() {
	cancelAndDelete(generatePacket);
	cancelAndDelete(emergencyPacket);
	cancelAndDelete(truckPacket);
	cancelAndDelete(redEmergencyPacket);
}
/**
 * Builds the schedule for emergencies:
 * 50% in the first 2 mins
 * 30% between 2 mins and 10 mins
 * 20% between 10 mins and 1h
 */
void TripRequestSubmitter::buildEmergencySchedule(int totalEmergencies) {
	int fiftypercent = totalEmergencies * 0.5; //50% in the first 2 mins
	for (int i = 0; i < fiftypercent; i++) {

		scheduledEmergencies.push_back(uniform(0, 120));
	}

	int thirtypercent = totalEmergencies * 0.3; //30% between 2 mins and 10 mins
	for (int i = 0; i < thirtypercent; i++) {

		scheduledEmergencies.push_back(uniform(120, 600));
	}
	//20% between 10 mins and 1h
	int twentypercent = totalEmergencies * 0.2 + 1; //+1 avoid seg fault when low density
	for (int i = 0; i < twentypercent; i++) {

		scheduledEmergencies.push_back(uniform(600, 3600));
	}

	sort(scheduledEmergencies.begin(), scheduledEmergencies.end());

	// print
//	for (auto elem : v)
//		ev << elem << "   ";
//	ev << endl;
}

/**
 * Return true if the value between 0 and (4 * distance) - 4 is == 0
 * -4 because we always want that an epicenter destroys every links
 *
 */
bool TripRequestSubmitter::propagateDistance(int distance) {
	return intuniform(0, 4 * (distance) - 4) == 0;
}

/*
 * Disconnect channels
 * Check if the node is in redzone and return the bool
 *
 */
bool TripRequestSubmitter::disconnectChannelsAndCheckRedzone() {

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

/**
 * Schedule the next emergency as:
 * 90% normal
 * 10% red code
 */
void TripRequestSubmitter::scheduleEmergencyOrRedCode() {
	if (intuniform(0, 10) == 0) { // 10% chance that there's a red code emergency
		// red code request
		scheduleAt(scheduledEmergencies[emergencyIndex++], redEmergencyPacket);
	} else {

		//emergency request
		scheduleAt(scheduledEmergencies[emergencyIndex++], emergencyPacket);

	}
}

void TripRequestSubmitter::initialize() {
	myAddress = par("address");
	destAddresses = par("destAddresses");
	minTripLength = par("minTripLength");
	sendIATime = &par("sendIaTime");  // volatile parameter
	maxDelay = &par("maxDelay");
	maxSubmissionTime = par("maxSubmissionTime");

	x_coord = getParentModule()->par("x");
	y_coord = getParentModule()->par("y");
	netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getParentModule()->getSubmodule("netmanager"));
	tcoord = check_and_cast<BaseCoord *>(getParentModule()->getParentModule()->getSubmodule("tcoord"));

	generatePacket = new cMessage("nextPacket");
	emergencyPacket = new cMessage("nextPacket");
	truckPacket = new cMessage("nextPacket");
	redEmergencyPacket = new cMessage("nextPacket");

	emergencyRequests = registerSignal("emergencyRequests");
	tripRequest = registerSignal("tripRequest");

	bool disconnected = netmanager->checkDestroyedNode(myAddress);
	if (disconnected) //AVOID Trip request creations
		return;

	// Check if the node is a coordination point
	if (netmanager->checkCollectionPointNode(myAddress)) {
		scheduleAt(sendIATime->doubleValue(), truckPacket);
	}

	bool redZoneNode = disconnectChannelsAndCheckRedzone();
//	for (auto n : v)
//		ev << n << endl;

	if (redZoneNode && !netmanager->checkHospitalNode(myAddress)) {

		totalEmergenciesPerNode = par("numberOfEmergencies");
		emergencyIndex = 0;

		buildEmergencySchedule(totalEmergenciesPerNode);

		scheduleEmergencyOrRedCode();

		if (ev.isGUI())
			getParentModule()->getDisplayString().setTagArg("i", 1, "red");
	}
	netmanager->updateTopology(); //updates the topology in network manager
}

void TripRequestSubmitter::handleMessage(cMessage *msg) {
	//EMIT a RED CODE REQUEST
	if (msg == redEmergencyPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("RED CODE");
		tr = buildRedCodeRequest(); // Builds red code request

		EV << "Requiring a RED CODE REQUEST from: " << tr->getPickupSP()->getLocation() << " to " << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << endl;
		emit(tripRequest, tr); // Emit request
		//stats
		tcoord->emitRedCodeEmergencyRequest();

		//Schedule the next request
		if (emergencyIndex < scheduledEmergencies.size()) { //Check if the emergency counter fits
			scheduleEmergencyOrRedCode();
			EV << "Next request from node " << myAddress << "scheduled at: " << scheduledEmergencies[emergencyIndex] << endl;
		}
	}

	//EMIT an EMERGENCY REQUEST
	if (msg == emergencyPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("EMERGENCY REQUEST");
		tr = buildEmergencyRequest(); // Builds emergency request

		EV << "Requiring a EMERGENCY REQUEST from: " << tr->getPickupSP()->getLocation() << " to " << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << endl;

		emit(tripRequest, tr); // Emit request
		//stats
		tcoord->emitEmergencyRequest();
		//Schedule the next request
		if (emergencyIndex < scheduledEmergencies.size()) { //Check if the emergency counter fits
			scheduleEmergencyOrRedCode();
			EV << "Next request from node " << myAddress << "scheduled at: " << scheduledEmergencies[emergencyIndex] << endl;
		}
	}
	//EMIT a Truck REQUEST
	if (msg == truckPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("TRUCK REQUEST");
		tr = buildTruckRequest();

		EV << "Requiring a TRUCK REQUEST from: " << tr->getPickupSP()->getLocation() << " to " << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << endl;

		emit(tripRequest, tr);
		tcoord->emitTruckRequest();

		//Schedule the next request
		simtime_t nextTime = simTime() + sendIATime->doubleValue(); //slow request
		scheduleAt(nextTime, truckPacket);

	}
}

/**
 * Build a new Truck Request
 * From a random storage point to random a collection point
 * isSpecial = 2
 */
TripRequest* TripRequestSubmitter::buildTruckRequest() {
	TripRequest *request = new TripRequest();
	double simtime = simTime().dbl();

	// Get truck address for the request
	int destAddress = netmanager->pickRandomCollectionPointNode(); // Pick a random collection  point as destination
	if (destAddress == myAddress)
		destAddress = netmanager->pickRandomStoragePointNode(); //if the random number is myAddress, pick a storage point instead (go back to a storage point)

	StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
	pickupSP->setXcoord(x_coord);
	pickupSP->setYcoord(y_coord);
	;

	StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

	request->setPickupSP(pickupSP);
	request->setDropoffSP(dropoffSP);

	request->setIsSpecial(2); //Truck request

	return request;
}

/**
 * Build a new Emergency Request
 * with parameters:
 * destAddress = netmanager->pickClosestHospitalFromNode(myAddress)
 * isSpecial = 1
 * at current simTime
 */
TripRequest* TripRequestSubmitter::buildEmergencyRequest() {
	TripRequest *request = new TripRequest();
	double simtime = simTime().dbl();

	// Generate emergency request to the closest hospital
	int destAddress = netmanager->pickClosestHospitalFromNode(myAddress);

	StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
	pickupSP->setXcoord(x_coord);
	pickupSP->setYcoord(y_coord);

	StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

	request->setPickupSP(pickupSP);
	request->setDropoffSP(dropoffSP);

	request->setIsSpecial(1); //hospital request

	return request;
}

/**
 * Build a new red code Request
 * with parameters:
 * destAddress = netmanager->pickClosestHospitalFromNode(myAddress)
 * isSpecial = 3
 * at current simTime
 */
TripRequest* TripRequestSubmitter::buildRedCodeRequest() {
	TripRequest *request = new TripRequest();
	double simtime = simTime().dbl();

	// Generate emergency request to the closest hospital
	int destAddress = netmanager->pickClosestHospitalFromNode(myAddress);

	StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
	pickupSP->setXcoord(x_coord);
	pickupSP->setYcoord(y_coord);

	StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

	request->setPickupSP(pickupSP);
	request->setDropoffSP(dropoffSP);

	request->setIsSpecial(3); // 3 is red code hospital request

	return request;
}

