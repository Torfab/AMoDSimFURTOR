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
    int rows;

	simtime_t civilEscapeInterval;

	BaseCoord *tcoord;
	AbstractNetworkManager *netmanager;

	// signals
	simsignal_t newTripAssigned;

	simsignal_t decayPheromoneValue;
	// Travel time related signals
	simsignal_t signal_ambulanceDelayTravelTime;
	simsignal_t signal_truckDelayTravelTime;
	simsignal_t signal_civilDelayTravelTime;
	simsignal_t signal_civilTravelTime;
	simsignal_t signal_ambulanceTravelTime;
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
		tcoord->evacuateCivil(myAddress);
			return;
		}
	Vehicle* civile;// = new Vehicle(-1, 9.7, 1);
	int destAddress;
	bool cp;



	if (intuniform(0, 1) == 0) {  // 50% chances: border node - collection point
		civile=new Vehicle(-1, 9.7, 1);
	    destAddress = tcoord->getClosestExitNode(myAddress); // look for a border node
		cp = false;
	} else {

		destAddress = netmanager->pickClosestCollectionPointFromNode(myAddress); // look for a collection point
		if (netmanager->getHopDistance(myAddress,destAddress)<=3){
		    civile=new Vehicle(-1, (float) uniform(1,2.5), 0); //il civile appiedato 1m/s 0 traffico
//		    EV<<"civile appiedato"<<endl;
		}else{
		    civile=new Vehicle(-1, 9.7, 1);
		}
		cp = true;
	}

	civile->setSrcAddr(myAddress);
	civile->setDestAddr(destAddress);

//	if (cp)
//		EV << "New (PANIC) civil vehicle " << civile->getID() << " running away to collection point: " << civile->getDestAddr() << " scheduled at " << interval << endl;
//	else
//		EV << "New (PANIC) civil vehicle " << civile->getID() << " running away to border node: " << civile->getDestAddr()<< " scheduled at " << interval  << endl;


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
	signal_civilTravelTime =registerSignal("signal_civilTravelTime");
	signal_ambulanceTravelTime =registerSignal("signal_ambulanceTravelTime");

	signal_ambulancesIdle = registerSignal("signal_ambulancesIdle");

	currentVehiclesInNode = numberOfVehicles;
	int numberOfCivils;

	newTripAssigned = registerSignal("newTripAssigned");
	decayPheromoneValue = registerSignal("decayPheromoneValue");


	CivilDestinations = netmanager->getNumberOfNodes();
	// Subscription to civil traffic
	simulation.getSystemModule()->subscribe("newCivilVehicle", this);

	EV << "I am node " << myAddress << endl;

	bool hospital = netmanager->checkHospitalNode(myAddress);
	bool storagePoint = netmanager->checkStoragePointNode(myAddress);
	bool collectionPoint = netmanager->checkCollectionPointNode(myAddress);

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
				getParentModule()->getDisplayString().setTagArg("i", 1, "white");


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
		ev << "nodo: " << myAddress << " in red zone" << endl;
		civilEscapeInterval = par("civilEscapeInterval");
		for (int i = 0; i < numberOfCivils; i++) {
			generateCivilTraffic(exponential(civilEscapeInterval));
		}
	}


	simulation.getSystemModule()->subscribe("decayPheromoneValue", this);
	if (!simulation.getSystemModule()->isSubscribed("decayPheromoneValue", this))
		ev << "non sottoscritto" << endl;
	else
		ev << "sottoscritto" << endl;

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

		emit(signal_civilTravelTime,vehicle->getCurrentTraveledTime()); //curr travel time

		delete vehicle;
		tcoord->evacuateCivil(myAddress);
		return;


	case 1:	//ambulance
		if (netmanager->checkHospitalNode(myAddress)) {
			emit(signal_ambulanceDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops);

			emit(signal_ambulanceTravelTime,vehicle->getCurrentTraveledTime()); //curr travel time

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


	StopPoint *currentStopPoint = tcoord->getCurrentStopPoint(vehicle->getID());

	if (currentStopPoint != NULL && currentStopPoint->getLocation() != -1 && currentStopPoint->getIsPickup()) {
		//This is a PICK-UP stop-point
		double waitTimeMinutes = (simTime().dbl() - currentStopPoint->getTime()) / 60;
		EV << "The vehicle is here! Pickup time: " << simTime() << "; Request time: " << currentStopPoint->getTime() << "; Waiting time: " << waitTimeMinutes << "minutes." << endl;

	}
	if (vehicle->getSpecialVehicle() == 1 && currentStopPoint->getIsPickup()){
		//take simtime di currentstoppoint
		//take simtime simulazione
		double difference = abs(simTime().dbl() - currentStopPoint->getTime());
		//emit differenza
		tcoord->emitDifferenceFromRequestToPickup(difference);

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
	if (signalID == decayPheromoneValue) {
//		pheromone->decayPheromone();
		ev << "Segnale di decay ricevuto" << endl;
	}
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

//				if (ev.isGUI())
//					getParentModule()->getDisplayString().setTagArg("i", 1, "gold");
				//if (simulation.getSystemModule()->isSubscribed("tripRequestCoord",this))
				//  simulation.getSystemModule()->unsubscribe("tripRequestCoord",this);
			}
		}
	}

}
