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

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <list>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Vehicle.h"
#include "TripRequest.h"
#include "BaseCoord.h"
#include "AbstractNetworkManager.h"
#include <sstream>

/**
 * Node's Application level.
 */
class App: public cSimpleModule, cListener {
private:
	// configuration
	int myAddress;
	int numberOfVehicles;
	int seatsPerVehicle;
	int boardingTime;
	int alightingTime;
	double additionalTravelTime;
	double ambulanceSpeed;
	double truckSpeed;
	int CivilDestinations;
	int numberOfTrucks;
	int currentVehiclesInNode;
//	bool enableCivilTraffic;
	int numberOfCivils;

	simtime_t civilEscapeInterval;

	BaseCoord *tcoord;
	AbstractNetworkManager *netmanager;

	// signals
	simsignal_t newTripAssigned;
	// Travel time related signals
	simsignal_t signal_ambulanceDelayTravelTime;
	simsignal_t signal_truckDelayTravelTime;
	simsignal_t signal_civilDelayTravelTime;
	// Idle signal
	simsignal_t signal_ambulancesIdle;

public:
	App();
	virtual ~App();

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void receiveSignal(cComponent *source, simsignal_t signalID, double vehicleID);
	void generateCivilTraffic(simtime_t interval);
};

Define_Module(App);

App::App() {
	tcoord = NULL;
}

App::~App() {
}

void App::generateCivilTraffic(simtime_t interval) {


	if (netmanager->checkBorderNode(myAddress)) {
	//		civile->setDestAddr(myAddress);
	//		send(civile, "out");
	//		EV << "Vehicle already on border, running away" << endl;
			return;
		}
	Vehicle* civile = new Vehicle(-1, 9.7, 1);
	int destAddress;
	bool cp;

	civile->setSrcAddr(myAddress);

	if (intuniform(0, 1) == 0) {  // 50% chances: border node - collection point
		destAddress = tcoord->getClosestExitNode(myAddress); // look for a border node
		cp = false;
	} else {
		destAddress = netmanager->pickClosestCollectionPointFromNode(myAddress); // look for a collection point
		cp = true;
	}

	civile->setDestAddr(destAddress);

	if (cp)
		EV << "New (PANIC) civil vehicle " << civile->getID() << " running away to collection point: " << civile->getDestAddr() << " scheduled at " << interval << endl;
	else
		EV << "New (PANIC) civil vehicle " << civile->getID() << " running away to border node: " << civile->getDestAddr()<< " scheduled at " << interval  << endl;


	sendDelayed(civile, interval, "out");

}

void App::initialize() {
	myAddress = par("address");
	seatsPerVehicle = par("seatsPerVehicle");
	alightingTime = getParentModule()->getParentModule()->par("alightingTime");
	boardingTime = getParentModule()->getParentModule()->par("boardingTime");
	tcoord = check_and_cast<BaseCoord *>(getParentModule()->getParentModule()->getSubmodule("tcoord"));
	netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getParentModule()->getSubmodule("netmanager"));
	numberOfVehicles = netmanager->getVehiclesPerNode(myAddress);
	additionalTravelTime = netmanager->getAdditionalTravelTime();
	numberOfTrucks = netmanager->getNumberOfTrucks();
	ambulanceSpeed = netmanager->getAmbulanceSpeed();
	truckSpeed = netmanager->getTruckSpeed();

	signal_truckDelayTravelTime = registerSignal("signal_truckDelayTravelTime");
	signal_ambulanceDelayTravelTime = registerSignal("signal_ambulanceDelayTravelTime");
	signal_civilDelayTravelTime = registerSignal("signal_civilDelayTravelTime");

	signal_ambulancesIdle = registerSignal("signal_ambulancesIdle");

	currentVehiclesInNode = numberOfVehicles;
	int numberOfCivils;


	// Destroying nodes part
	cTopology* topo = new cTopology("topo");
	std::vector<std::string> nedTypes;
	nedTypes.push_back("src.node.Node");
	topo->extractByNedTypeName(nedTypes);


	//topo = tcoord->getTopo();
	cTopology::Node *node = topo->getNode(myAddress);

	if (netmanager->checkDisconnectedNode(myAddress)) {
		// disconnects channels
		for (int j = 0; j < node->getNumOutLinks(); j++) {
//            cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();
			cGate *gate = node->getLinkOut(j)->getLocalGate();
			gate->disconnect();

		}
		return;
	} else {
		for (int j = 0; j < node->getNumOutLinks(); j++) {
			cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();

			if (netmanager->checkDisconnectedNode(neighbour->getModule()->getIndex())) {
				cGate *gate = node->getLinkOut(j)->getLocalGate();
				gate->disconnect();
			}
		}

	}

	delete topo;
	newTripAssigned = registerSignal("newTripAssigned");

	CivilDestinations = netmanager->getNumberOfNodes();
	// Subscription to civil traffic
	simulation.getSystemModule()->subscribe("newCivilVehicle", this);

	EV << "I am node " << myAddress << endl;

	bool hospital = netmanager->checkHospitalNode(myAddress);
	bool storagePoint=netmanager->checkStoragePointNode(myAddress);
//	int truckStart = netmanager->getTruckStartNode();

	if (numberOfVehicles > 0) {
		for (int i = 0; i < numberOfVehicles; i++) {
			Vehicle *v;

			if (hospital) {
				v = new Vehicle(1, ambulanceSpeed, 1);
				v->setSeats(1);
			} else if (storagePoint && numberOfTrucks > 0) {
				v = new Vehicle(2, truckSpeed, 20);
				v->setSeats(0);
				numberOfTrucks--;
			}

			else {
				v = new Vehicle(-1, 9.7, 1);
				v->setSeats(seatsPerVehicle);
			}

			EV << "I am node " << myAddress << ". I HAVE THE VEHICLE " << v->getID() << " of type " << v->getSpecialVehicle() << ". It has " << v->getSeats() << " seats." << endl;
			tcoord->registerVehicle(v, myAddress);
		}

		if (netmanager->checkHospitalNode(myAddress)){
			emit(signal_ambulancesIdle,currentVehiclesInNode);

			if (ev.isGUI())
				getParentModule()->getDisplayString().setTagArg("i", 3, "white");


		}
//			else if (ev.isGUI())
//				getParentModule()->getDisplayString().setTagArg("i", 1, "green");

		//When the coordinator assign a new request to a vehicle, local node will be notified
		simulation.getSystemModule()->subscribe("newTripAssigned", this);
	}

//	enableCivilTraffic = par("enableCivilTraffic");
	numberOfCivils = par("numberOfCivils");
	// Sono un nodo in red zone?
	// si -> genero traffico
	// no -> nice
	if (netmanager->checkRedZoneNode(myAddress)) {
		civilEscapeInterval = par("civilEscapeInterval");
		for (int i = 0; i < numberOfCivils; i++) {
			generateCivilTraffic(exponential(civilEscapeInterval));
		}
	}
}

void App::handleMessage(cMessage *msg) {
	Vehicle *vehicle = NULL;

	double sendDelayTime = additionalTravelTime;    // * trafficFactor;

	try {
		//A vehicle is here
		vehicle = check_and_cast<Vehicle *>(msg);
	} catch (cRuntimeError e) {
		EV << "Can not handle received message! Ignoring..." << endl;
		return;
	}

	EV << "Destination completed: VEHICLE " << vehicle->getID() << " after " << vehicle->getHopCount() << " hops. The type of vehicle is " << vehicle->getSpecialVehicle() << endl;
//    vehicle->setBusyState(false);

	int numHops = netmanager->getHopDistance(vehicle->getSrcAddr(), vehicle->getDestAddr());

	switch (vehicle->getSpecialVehicle()) {
	case -1: //civil
		EV << "Veicolo civile a destinazione " << vehicle->getDestAddr() << " partito da " << vehicle->getSrcAddr() << endl;
		emit(signal_civilDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops);
		delete vehicle;
		tcoord->evacuateCivil(myAddress);
		return;

	case 1:	//ambulance
		if (netmanager->checkHospitalNode(myAddress)) {
			emit(signal_ambulanceDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops);
			EV << "Ambulanza Tempo reale: " << vehicle->getCurrentTraveledTime() << " stimato: " << vehicle->getOptimalEstimatedTravelTime() << " hops " << numHops << endl;

		}
		break;
	case 2:
		emit(signal_truckDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops);
		EV << "Truck Tempo reale: " << vehicle->getCurrentTraveledTime() << " stimato: " << vehicle->getOptimalEstimatedTravelTime() << " hops " << numHops << endl;
		break;
	default:
		break;
	}
//
//    // Civil vehicle
//    if (vehicle->getSpecialVehicle() == -1){
//    	if (vehicle->getDestAddr() == myAddress) {
//    		EV << "Veicolo civile a destinazione " << vehicle->getDestAddr()<< " partito da "<< vehicle->getSrcAddr() <<endl;
////    		generateCivilTraffic();
//
//    		emit(signal_civilDelayTravelTime,(vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime())/numHops);
//
//    		delete vehicle;
//    		return;
//    	}
//    }

//        int destAddress = intuniform(0, CivilDestinations, 3);
//                   while (destAddress == myAddress)
//                       destAddress = intuniform(0, CivilDestinations - 1, 3);
//		vehicle->setDestAddr(destAddress);
//
//		double delays = simTime().dbl()	- (netmanager->getTimeDistance(myAddress,vehicle->getDestAddr()));
//		if (delays < 0)
//			delays = 0;
//
//		if (vehicle->getDestAddr() == myAddress)
//			sendDelayed(vehicle, delays, "out");
//		else
//			sendDelayed(vehicle, sendDelayTime + delays, "out");
//		return;

	StopPoint *currentStopPoint = tcoord->getCurrentStopPoint(vehicle->getID());

	if (currentStopPoint != NULL && currentStopPoint->getLocation() != -1 && currentStopPoint->getIsPickup()) {
		//This is a PICK-UP stop-point
		double waitTimeMinutes = (simTime().dbl() - currentStopPoint->getTime()) / 60;
		EV << "The vehicle is here! Pickup time: " << simTime() << "; Request time: " << currentStopPoint->getTime() << "; Waiting time: " << waitTimeMinutes << "minutes." << endl;

	}

	//Ask to coordinator for next stop point
	StopPoint *nextStopPoint = tcoord->getNextStopPoint(vehicle->getID());
	if (nextStopPoint != NULL) {

		//There is another stop point for the vehicle!
		EV << "The next stop point for the vehicle " << vehicle->getID() << " is: " << nextStopPoint->getLocation() << endl;
		vehicle->setSrcAddr(myAddress);
		vehicle->setDestAddr(nextStopPoint->getLocation());

		// reset times
		vehicle->setOptimalEstimatedTravelTime(netmanager->getHopDistance(myAddress, nextStopPoint->getLocation()) * (netmanager->getXChannelLength() / vehicle->getSpeed()));// * (netmanager->getXChannelLength() / vehicle->getSpeed())));
		vehicle->setCurrentTraveledTime(0);
		vehicle->setHopCount(0);

		//Time for boarding or drop-off passengers
		double delays = (nextStopPoint->getActualTime() - simTime().dbl()) - netmanager->getTimeDistance(myAddress, nextStopPoint->getLocation());
		if (delays < 0)
			delays = 0;

		if (nextStopPoint->getLocation() == myAddress)
			sendDelayed(vehicle, delays, "out");
		else
			sendDelayed(vehicle, sendDelayTime + delays, "out");
	}

	//No other stop point for the vehicle. The vehicle stay here
	else {
		EV << "Vehicle " << vehicle->getID() << " is in node " << myAddress << endl;
		tcoord->registerVehicle(vehicle, myAddress);


		if (netmanager->checkHospitalNode(myAddress)){
						emit(signal_ambulancesIdle,++currentVehiclesInNode);
					}
//        if (ev.isGUI())
//            getParentModule()->getDisplayString().setTagArg("i",1,"green");

		if (!simulation.getSystemModule()->isSubscribed("newTripAssigned", this))
			simulation.getSystemModule()->subscribe("newTripAssigned", this);
	}

}

/**
 * Handle an Omnet signal.
 * 
 * @param source
 * @param signalID
 * @param obj
 */
void App::receiveSignal(cComponent *source, simsignal_t signalID, double vehicleID) {

	/**
	 * The coordinator has accepted a trip proposal
	 */
	if (signalID == newTripAssigned) {
		if (tcoord->getLastVehicleLocation(vehicleID) == myAddress) {
			//The vehicle that should serve the request is in this node
			Vehicle *veic = tcoord->getVehicleByID(vehicleID);

			if (veic != NULL) {

//				veic->setBusyState(true);


				double sendDelayTime = additionalTravelTime;

				StopPoint* sp = tcoord->getNewAssignedStopPoint(veic->getID());
				EV << "The proposal of vehicle: " << veic->getID() << " has been accepted for requestID:  " << sp->getRequestID() << endl;
				veic->setSrcAddr(myAddress);
				veic->setDestAddr(sp->getLocation());

				// reset times
				veic->setOptimalEstimatedTravelTime(netmanager->getHopDistance(myAddress, sp->getLocation()) * (netmanager->getXChannelLength() / veic->getSpeed()));// * (netmanager->getXChannelLength() / vehicle->getSpeed())));
				veic->setCurrentTraveledTime(0);
				veic->setHopCount(0);

				//Time for boarding or dropoff
				double delays = (sp->getActualTime() - simTime().dbl()) - netmanager->getTimeDistance(myAddress, sp->getLocation());
				if (delays < 0)
					delays = 0;

				if (sp->getLocation() == myAddress)
					sendDelayTime = delays;
				else
					sendDelayTime = sendDelayTime + delays;

				if (netmanager->checkHospitalNode(myAddress)){
					emit(signal_ambulancesIdle,--currentVehiclesInNode);
				}

				EV << "Sending Vehicle from: " << veic->getSrcAddr() << " to " << veic->getDestAddr() << endl;
				Enter_Method
				("sendDelayed", veic, sendDelayTime, "out");
				sendDelayed(veic, sendDelayTime, "out");

				if (ev.isGUI())
					getParentModule()->getDisplayString().setTagArg("i", 1, "gold");
				//if (simulation.getSystemModule()->isSubscribed("tripRequestCoord",this))
				//  simulation.getSystemModule()->unsubscribe("tripRequestCoord",this);
			}
		}
	}

}
