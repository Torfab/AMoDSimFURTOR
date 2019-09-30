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
	simsignal_t skilledRequests;

	unsigned int emergencyIndex; //used to browse the vector
	int totalEmergenciesPerNode;
	int skilledRequestsCount;

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
}


/**
 * Schedule the next emergency as:
 * 90% normal
 * 10% red code
 */
void TripRequestSubmitter::scheduleEmergencyOrRedCode() {
//	scheduleAt(scheduledEmergencies[emergencyIndex++], redEmergencyPacket);
//	return;

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

	sendIATime = &par("sendIaTime");  // volatile parameter
	maxDelay = &par("maxDelay");

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

	skilledRequests = registerSignal("skilledRequests");
	skilledRequestsCount =0;
	// Check if the node is a coordination point
	if (netmanager->checkCollectionPointNode(myAddress)) {
		scheduleAt(sendIATime->doubleValue(), truckPacket);
	}

	if (netmanager->checkRedZoneNode(myAddress) && !netmanager->checkHospitalNode(myAddress)) {
		totalEmergenciesPerNode = par("numberOfEmergencies");
		emergencyIndex = 0;
		buildEmergencySchedule(totalEmergenciesPerNode);

		scheduleEmergencyOrRedCode();
	}

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

	StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, -1);
	pickupSP->setXcoord(x_coord);
	pickupSP->setYcoord(y_coord);

	StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), -1);

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

	int destAddress;

	StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
	pickupSP->setXcoord(x_coord);
	pickupSP->setYcoord(y_coord);
	pickupSP->setRedCode(true);

	if (intuniform(0, 10, 0) == 0) {
		//requires skilled hospital
		pickupSP->setNeedSkilledHospital(true);
		destAddress = netmanager->pickSkilledHospitalFromNode(myAddress);
		emit(skilledRequests, ++skilledRequestsCount);
	} else{
		// Generate emergency request to the closest hospital
		destAddress = netmanager->pickClosestHospitalFromNode(myAddress);
	}

	StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());
	dropoffSP->setRedCode(true);

	request->setPickupSP(pickupSP);
	request->setDropoffSP(dropoffSP);

	request->setIsSpecial(3); // 3 is red code hospital request

	return request;
}

