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

#include <omnetpp.h>
#include "TripRequest.h"
#include "BaseCoord.h"
#include "AbstractNetworkManager.h"
#include <algorithm>
#include <vector>

class TripRequestSubmitter : public cSimpleModule
{
    private:
        // configuration
        int myAddress;
        int x_coord;
        int y_coord;
        int rows; //of the grid

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
        long pkCounter;

        std::vector<double> v;

        // signals
        simsignal_t tripRequest;
        simsignal_t emergencyRequests;
        int emergencyIndex;
        int totalEmergenciesPerNode;
      public:
        TripRequestSubmitter();
        virtual ~TripRequestSubmitter();

      protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual TripRequest* buildEmergencyRequest();
        virtual TripRequest* buildTripRequest();
        virtual TripRequest* buildTruckRequest();
        virtual TripRequest* buildRedCodeRequest();
	void buildEmergencySchedule(int totalEmergencies);
	bool disconnectChannelsAndCheckRedzone();
	bool propagateDistance(int distance);
	void scheduleEmergencyOrRedCode();
};

Define_Module(TripRequestSubmitter);

TripRequestSubmitter::TripRequestSubmitter()
{
    generatePacket = NULL;
    netmanager = NULL;
}

TripRequestSubmitter::~TripRequestSubmitter()
{
    cancelAndDelete(generatePacket);
    cancelAndDelete(emergencyPacket);
    cancelAndDelete(truckPacket);
    cancelAndDelete(redEmergencyPacket);
}

void TripRequestSubmitter::buildEmergencySchedule(int totalEmergencies) {
	int fiftypercent = totalEmergencies * 0.5;
	for (int i = 0; i < fiftypercent; i++) {
		v.push_back(uniform(0, 120));
	}
	int thirtypercent = totalEmergencies * 0.3;
	for (int i = 0; i < thirtypercent; i++) {
		v.push_back(uniform(120, 600));
	}
	int twentypercent = totalEmergencies * 0.2 + 1;  //+1 avoid seg fault when low density
	for (int i = 0; i < twentypercent; i++) {
		v.push_back(uniform(600, 3600));
	}
	sort(v.begin(), v.end());
//	for (auto elem : v)  	// print
//		ev << elem << "   ";
//	ev << endl;
}

bool TripRequestSubmitter::propagateDistance(int distance) {
	return intuniform(0, 4*(distance)-4) == 0;
}

bool TripRequestSubmitter::disconnectChannelsAndCheckRedzone() {
	rows = getParentModule()->getParentModule()->par("width");
	bool hospital = netmanager->checkHospitalNode(myAddress);
	bool storagePoint = netmanager->checkStoragePointNode(myAddress);
	bool collectionPoint = netmanager->checkCollectionPointNode(myAddress);
	bool redZoneNode = false;


	// Destroying nodes part
	cTopology *topo = netmanager->getTopo();
	//topo = tcoord->getTopo();
	cTopology::Node* node = topo->getNode(myAddress);

	std::set<int> s = netmanager->getSetOfEpicenters();

	int myX = myAddress % rows;
	int myY = myAddress / rows;
	for (auto elem : s) {

		int epicX = elem % rows;
		int epicY = elem / rows;

		if (!hospital && !collectionPoint && !storagePoint) {
			//rompi gate a destra
			//rompi gate in basso
			int distance;

			for (int j = 0; j < node->getNumOutLinks(); j++) {

				switch (node->getLinkOut(j)->getLocalGate()->getIndex()) {
				case 1:  	// EAST
					distance = netmanager->getManhattanDistance(myAddress, elem);
					if (myX >= epicX)
						distance++;

					if (propagateDistance(distance)) {

						cGate *gate = node->getLinkOut(j)->getLocalGate();
						gate->disconnect();
						netmanager->insertRedZoneNode(myAddress);
						redZoneNode = true;

					}
					break;
				case 2: 	// SOUTH
					distance = netmanager->getManhattanDistance(myAddress, elem);
					if (myY >= epicY)
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

		int guardiaW = -1;
		int guardiaN = -1;

		for (int j = 0; j < node->getNumInLinks(); j++) {
			ev << "index del nodo" << node->getModule()->getIndex() << " : in: " << node->getLinkIn(j)->getLocalGate()->getIndex() << endl;

			if (node->getLinkIn(j)->getLocalGate()->getIndex() == 3 &&  node->getLinkIn(j)->getLocalGate()->isConnected()) {
				// il canale esiste
				// non succede nulla
				guardiaW = 1;
			}
			if (node->getLinkIn(j)->getLocalGate()->getIndex() == 0  &&  node->getLinkIn(j)->getLocalGate()->isConnected()) {
				// il canale esiste
				// non succede nulla
				guardiaN = 1;
			}

		}
		if (guardiaW == -1) {
			// controlla se ha un canale in uscita verso quel nodo
			for (int k = 0; k < node->getNumOutLinks(); k++) {
				if (node->getLinkOut(k)->getLocalGate()->getIndex() == 3) {
					cGate *gate = node->getLinkOut(k)->getLocalGate();
					gate->disconnect();
					netmanager->insertRedZoneNode(myAddress);
					redZoneNode = true;

				}
			}
		}
		if (guardiaN == -1) {
			// controlla se ha un canale in uscita verso quel nodo
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

	int disconnected = 0;
	for (int k = 0; k < node->getNumOutLinks(); k++) {

		cGate *gate = node->getLinkOut(k)->getLocalGate();
		if (!gate->isConnected()) {
			disconnected++;
		}
	}
	if (disconnected == node->getNumOutLinks()) {
		netmanager->insertDestroyedNode(myAddress);
		netmanager->removeRedZoneNode(myAddress);
		redZoneNode = false;
	}

	return redZoneNode;
}

void TripRequestSubmitter::scheduleEmergencyOrRedCode() {
	if (intuniform(0, 10) == 0) {
		// con probabilita' 50% genera un generatepacket o un emergencypacket e lo schedula
		//richiesta red code
		scheduleAt(v[emergencyIndex++], redEmergencyPacket);
	} else {
		//richiesta emergenza normale
		scheduleAt(v[emergencyIndex++], emergencyPacket);

	}
}

void TripRequestSubmitter::initialize()
{
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
    if (netmanager->checkCollectionPointNode(myAddress)){
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
	}

	netmanager->updateTopology();
}


void TripRequestSubmitter::handleMessage(cMessage *msg)
{

	/*
    //EMIT a TRIP REQUEST (unused)
    if (msg == generatePacket)
    {
        TripRequest *tr = nullptr;

        if (ev.isGUI()) getParentModule()->bubble("TRIP REQUEST");
        tr = buildTripRequest();

        EV << "Requiring a bus trip request from/to: " << tr->getPickupSP()->getLocation() << "/" << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
        EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << ". DropOFF required time: " << tr->getDropoffSP()->getTime() << ". Passengers: " << tr->getPickupSP()->getNumberOfPassengers() << endl;

        emit(tripRequest, tr);

        //Schedule the next request
        simtime_t nextTime = simTime() + sendIATime->doubleValue();

        if (maxSubmissionTime < 0 || nextTime.dbl() < maxSubmissionTime) {
                   EV << "Next request from node " << myAddress << "scheduled at: " << nextTime.dbl() << endl;
            if (intuniform(0, 1, 3) == 0) { // con probabilita' 50% genera un generatepacket o un emergencypacket e lo schedula
                //richiesta normale
                scheduleAt(nextTime, generatePacket);
            } else {
                //richiesta emergenza
                scheduleAt(nextTime, emergencyPacket);
            }
        }
    }
	*/
	if (msg == redEmergencyPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("RED CODE");
		tr = buildRedCodeRequest(); // Builds red code request

		EV << "Requiring a RED CODE REQUEST from/to: " << tr->getPickupSP()->getLocation() << "/" << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << ". DropOFF required time: " << tr->getDropoffSP()->getTime() << ". Passengers: " << tr->getPickupSP()->getNumberOfPassengers() << endl;

		emit(tripRequest, tr); // Emit request
		//stats
		tcoord->emitRedCodeEmergencyRequest();

		//schedulazione nuova
		if (emergencyIndex < v.size()) { //Check if the emergency counter fits
			scheduleEmergencyOrRedCode();
			EV << "Next request from node " << myAddress << "scheduled at: " << v[emergencyIndex] << endl;
		}
	}


	//EMIT an EMERGENCY REQUEST
	if (msg == emergencyPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("EMERGENCY REQUEST");
		tr = buildEmergencyRequest(); // Builds emergency request

		EV << "Requiring a EMERGENCY REQUEST from/to: " << tr->getPickupSP()->getLocation() << "/" << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << ". DropOFF required time: " << tr->getDropoffSP()->getTime() << ". Passengers: " << tr->getPickupSP()->getNumberOfPassengers() << endl;

		emit(tripRequest, tr); // Emit request
		//stats
		tcoord->emitEmergencyRequest();
		//schedulazione nuova
		if (emergencyIndex < v.size()) { //Check if the emergency counter fits
			scheduleEmergencyOrRedCode();
			EV << "Next request from node " << myAddress << "scheduled at: " << v[emergencyIndex] << endl;
		}
	}
	//EMIT an Truck REQUEST
	if (msg == truckPacket) {
		TripRequest *tr = nullptr;

		if (ev.isGUI())
			getParentModule()->bubble("TRUCK REQUEST");
		tr = buildTruckRequest();

		EV << "Requiring a TRUCK REQUEST from/to: " << tr->getPickupSP()->getLocation() << "/" << tr->getDropoffSP()->getLocation() << ". I am node: " << myAddress << endl;
		EV << "Requested pickupTime: " << tr->getPickupSP()->getTime() << ". DropOFF required time: " << tr->getDropoffSP()->getTime() << ". Passengers: " << tr->getPickupSP()->getNumberOfPassengers() << endl;

		emit(tripRequest, tr);

		//Schedule the next request
		simtime_t nextTime = simTime() + sendIATime->doubleValue(); //slow request
		if (maxSubmissionTime < 0 || nextTime.dbl() < maxSubmissionTime) {
			EV << "Next truck request from node " << myAddress << "scheduled at: " << nextTime.dbl() << endl;
			////truck request
			scheduleAt(simTime() + sendIATime->doubleValue(), truckPacket);

		}
	}
}

/**
 * Build a new Truck Request
 * From a random storage point to random a collection point
 * isSpecial = 2
 */
TripRequest* TripRequestSubmitter::buildTruckRequest()
{
    TripRequest *request = new TripRequest();
    double simtime = simTime().dbl();

    // Get truck address for the request
//    int destAddress = netmanager->getTruckStartNode();

    int destAddress = netmanager->pickRandomCollectionPointNode(); //pickClosestCollectionPointFromNode(myAddress);
    if (destAddress == myAddress) destAddress = netmanager->pickRandomStoragePointNode();

    StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
    pickupSP->setXcoord(x_coord);
    pickupSP->setYcoord(y_coord);
    pickupSP->setNumberOfPassengers(0);

    StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

    request->setPickupSP(pickupSP);
    request->setDropoffSP(dropoffSP);

    request->setIsSpecial(2); //truck request

    return request;
}


/**
 * Build a new Emergency Request
 * with parameters:
 * destAddress = netmanager->pickClosestHospitalFromNode(myAddress)
 * isSpecial = 1
 * at current simTime
 */
TripRequest* TripRequestSubmitter::buildEmergencyRequest()
{
    TripRequest *request = new TripRequest();
    double simtime = simTime().dbl();

    // Generate emergency request to the closest hospital
    int destAddress = netmanager->pickClosestHospitalFromNode(myAddress);

    StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
    pickupSP->setXcoord(x_coord);
    pickupSP->setYcoord(y_coord);
    pickupSP->setNumberOfPassengers(par("passengersPerRequest"));

    StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

    request->setPickupSP(pickupSP);
    request->setDropoffSP(dropoffSP);

    request->setIsSpecial(1); //hospital request

    EV << "emergency from " << myAddress << " to the hospital in node: " << destAddress << endl;
    return request;
}

/**
 * Build a new red code Request
 * with parameters:
 * destAddress = netmanager->pickClosestHospitalFromNode(myAddress)
 * isSpecial = 1
 * at current simTime
 */
TripRequest* TripRequestSubmitter::buildRedCodeRequest()
{
    TripRequest *request = new TripRequest();
    double simtime = simTime().dbl();

    // Generate emergency request to the closest hospital
    int destAddress = netmanager->pickClosestHospitalFromNode(myAddress);

    StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
    pickupSP->setXcoord(x_coord);
    pickupSP->setYcoord(y_coord);
    pickupSP->setNumberOfPassengers(par("passengersPerRequest"));

    StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

    request->setPickupSP(pickupSP);
    request->setDropoffSP(dropoffSP);

    request->setIsSpecial(3); // 3 is red code hospital request

    EV << "red code emergency from " << myAddress << " to the hospital in node: " << destAddress << endl;
    return request;
}

//(unused)
TripRequest* TripRequestSubmitter::buildTripRequest()
{
    TripRequest *request = new TripRequest();
    double simtime = simTime().dbl();

    // Generate a random destination address for the request
    int destAddress = netmanager->pickRandomNodeInRedZone();

    StopPoint *pickupSP = new StopPoint(request->getID(), myAddress, true, simtime, maxDelay->doubleValue());
    pickupSP->setXcoord(x_coord);
    pickupSP->setYcoord(y_coord);
    pickupSP->setNumberOfPassengers(intuniform(1,10,3));

    StopPoint *dropoffSP = new StopPoint(request->getID(), destAddress, false, simtime + netmanager->getTimeDistance(myAddress, destAddress), maxDelay->doubleValue());

    request->setPickupSP(pickupSP);
    request->setDropoffSP(dropoffSP);

    request->setIsSpecial(0); // bus request

    return request;
}

