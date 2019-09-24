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
	double ambulanceSpeed;
	double truckSpeed;
	int CivilDestinations;
	int numberOfTrucks;
	int currentVehiclesInNode;
	int numberOfCivils;


	simtime_t civilEscapeInterval;

	BaseCoord *tcoord;
	AbstractNetworkManager *netmanager;

	// signals
	simsignal_t newTripAssigned;

	// Travel time related signals
	simsignal_t signal_truckTravelTime;
	simsignal_t signal_civilDelayTravelTime;
	simsignal_t signal_civilTravelTime;

	simsignal_t signal_ambulanceTravelTime;
	simsignal_t signal_ambulanceDelayTravelTime;


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
	virtual double computeAccelererationTime(double speed, double acceleration);
};

Define_Module(App);

App::App() {
	tcoord = NULL;
}

App::~App() {
}

/*
 * generate a civil packet and send it on simtime()+interval
 */
void App::generateCivilTraffic(simtime_t interval) {


	if (netmanager->checkBorderNode(myAddress)) {
		tcoord->evacuateCivil(myAddress);
			return;
		}
	Vehicle* civile;// = new Vehicle(-1, 9.7, 1);
	int destAddress;



	if (intuniform(0, 1) == 0) {  // 50% chances: border node - collection point
		civile=new Vehicle(-1, 9.7, 1); // vehicle(type,speed,traffic weight)
	    destAddress = tcoord->getClosestExitNode(myAddress); // look for a border node

	} else {

		destAddress = netmanager->pickClosestCollectionPointFromNode(myAddress); // look for a collection point
		if (netmanager->getHopDistance(myAddress,destAddress)<=3){               //if the collection point is too close
		    civile=new Vehicle(-1, (float) uniform(1,2.5), 0);                   // civil don't use a car but walk to the collectionPOint, vehicle(type,speed,traffic weight)
		}else{
		    civile=new Vehicle(-1, 9.7, 1);
		}
	}

	civile->setSrcAddr(myAddress);
	civile->setDestAddr(destAddress);


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
	numberOfTrucks = netmanager->getNumberOfTrucks();
	ambulanceSpeed = netmanager->getAmbulanceSpeed();
	truckSpeed = netmanager->getTruckSpeed();

	signal_truckTravelTime = registerSignal("signal_truckTravelTime");
	signal_ambulanceDelayTravelTime = registerSignal("signal_ambulanceDelayTravelTime");
	signal_civilDelayTravelTime = registerSignal("signal_civilDelayTravelTime");
	signal_civilTravelTime =registerSignal("signal_civilTravelTime");
	signal_ambulanceTravelTime =registerSignal("signal_ambulanceTravelTime");

	signal_ambulancesIdle = registerSignal("signal_ambulancesIdle");

	currentVehiclesInNode = numberOfVehicles;

	newTripAssigned = registerSignal("newTripAssigned");



	CivilDestinations = netmanager->getNumberOfNodes();
	// Subscription to civil traffic
	simulation.getSystemModule()->subscribe("newCivilVehicle", this);

	EV << "I am node " << myAddress << endl;

	bool hospital = netmanager->checkHospitalNode(myAddress);
	bool storagePoint = netmanager->checkStoragePointNode(myAddress);
	bool collectionPoint = netmanager->checkCollectionPointNode(myAddress);
	//this portion of code let us initialize the vehicles in the interesting nodes of our grid
	if (numberOfVehicles > 0) {
		for (int i = 0; i < numberOfVehicles; i++) {
			Vehicle *v;

			if (hospital) {
				v = new Vehicle(1, ambulanceSpeed, 1);
				v->setSeats(1);
			} else if (storagePoint){
				v = new Vehicle(2, truckSpeed, 20);
				v->setSeats(0);
			}

            EV << "I am node " << myAddress << ". I HAVE THE VEHICLE "<< v->getID() << " of type " << v->getSpecialVehicle()<< ". It has " << v->getSeats() << " seats." << endl;
            tcoord->registerVehicle(v, myAddress);
        }


		simulation.getSystemModule()->subscribe("newTripAssigned", this);

	}
	// fancy colors
	if (hospital) {
		emit(signal_ambulancesIdle, currentVehiclesInNode);
		if (ev.isGUI())
			getParentModule()->getDisplayString().setTagArg("b", 3, "white");
	} else if (storagePoint) {
		if (ev.isGUI())
			getParentModule()->getDisplayString().setTagArg("b", 3, "orange");
	} else if (collectionPoint) {
		if (ev.isGUI())
			getParentModule()->getDisplayString().setTagArg("b", 3, "blue");
	}


    numberOfCivils = par("numberOfCivils");

    //Only the node in red zone will generate civil traffic of people escaping, all the traffic is generated in initialize and will be sent delayed in the simulation
    if (netmanager->checkRedZoneNode(myAddress)) {
        ev << "nodo: " << myAddress << " in red zone" << endl;
        civilEscapeInterval = par("civilEscapeInterval");
        for (int i = 0; i < numberOfCivils; i++) {
            generateCivilTraffic(exponential(civilEscapeInterval));
        }
    }

}



void App::handleMessage(cMessage *msg) {
	Vehicle *vehicle = nullptr;

	try {
		//A vehicle is here
		vehicle = check_and_cast<Vehicle *>(msg);
	} catch (cRuntimeError e) {
		EV << "Can not handle received message! Ignoring..." << endl;
		return;
	}
	double sendDelayTime = computeAccelererationTime(vehicle->getSpeed(),vehicle->getAcceleration());    // time necessary for deceleration
	vehicle->setCurrentTraveledTime(vehicle->getCurrentTraveledTime() + sendDelayTime);

	EV << "Destination completed: VEHICLE " << vehicle->getID() << " after " << vehicle->getHopCount() << " hops. The type of vehicle is " << vehicle->getSpecialVehicle() << endl;

	int numHops = netmanager->getHopDistance(vehicle->getSrcAddr(), vehicle->getDestAddr());

	switch (vehicle->getSpecialVehicle()) {
	case -1: //civil
		EV << "civil Vehicle reached its destination " << vehicle->getDestAddr() << " he departed from " << vehicle->getSrcAddr() << endl;
		emit(signal_civilDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops); //number of average delay for each node compared to no traffic roads
		emit(signal_civilTravelTime,vehicle->getCurrentTraveledTime()); //emit of the actual traveled time to reach the request

		delete vehicle;         //civil vehicles are deleted and cannot be use after they reach their destination
		tcoord->evacuateCivil(myAddress);
		return;


	case 1:	//ambulance
		if (netmanager->checkHospitalNode(myAddress)) {
//			emit(signal_ambulanceDelayTravelTime, (vehicle->getCurrentTraveledTime() - vehicle->getOptimalEstimatedTravelTime()) / numHops);
			emit(signal_ambulanceTravelTime,vehicle->getCurrentTraveledTime()); //curr travel time

			EV << "Ambulance actual time from last stop point to current: " << vehicle->getCurrentTraveledTime() << " the estimated one: " << vehicle->getOptimalEstimatedTravelTime() << " hops: " << numHops << endl;

		}

		break;
	case 2: //truck
		emit(signal_truckTravelTime, vehicle->getCurrentTraveledTime());    //number of average delay for each node compared to no traffic roads
		EV << "Truck actual time from last stop point to current: " << vehicle->getCurrentTraveledTime() << " estimated: " << vehicle->getOptimalEstimatedTravelTime() << " hops: " << numHops << endl;
		break;
	default:
		break;
	}

	StopPoint *currentStopPoint = tcoord->getCurrentStopPoint(vehicle->getID());
	if (currentStopPoint != NULL && currentStopPoint->getLocation() != -1 && currentStopPoint->getIsPickup()) {
		//This is a PICK-UP stop-point
		double waitTimeMinutes = (simTime().dbl() - currentStopPoint->getTime()) / 60;
		EV << "The vehicle is here! Pickup time: " << simTime() << "; Request time: " << currentStopPoint->getTime() << "; Waiting time: " << waitTimeMinutes << "minutes." << endl;
		sendDelayTime = 180;  //180s lumped for boarding up an emergency
		vehicle->setCurrentTraveledTime(vehicle->getCurrentTraveledTime() + sendDelayTime);
		if (vehicle->getSpecialVehicle() == 1) {
			double difference = abs(simTime().dbl() - currentStopPoint->getTime());
			//emit actual time from request to pickup
			tcoord->emitDifferenceFromRequestToPickup(difference, currentStopPoint->isRedCode()); //emit in two different signals if the request was a red code or not
			tcoord->emitPickupEmergencies();
		}
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

		if(nextStopPoint->getIsPickup()){
		vehicle->setCurrentTraveledTime(0);
		vehicle->setHopCount(0);
		}

		//Time for boarding or drop-off passengers


		sendDelayed(vehicle, sendDelayTime, "out");     //if there is another stop point send after sendDelayTime
	}

	//No other stop point for the vehicle. The vehicle stay here and it is registered in the node
	else {
		EV << "Vehicle " << vehicle->getID() << " is in node " << myAddress << endl;
		tcoord->registerVehicle(vehicle, myAddress);


		if (netmanager->checkHospitalNode(myAddress)){
						emit(signal_ambulancesIdle,++currentVehiclesInNode);
					}

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
			Vehicle *vehicle = tcoord->getVehicleByID(vehicleID);

			if (vehicle != nullptr) {

				double sendDelayTime = computeAccelererationTime(vehicle->getSpeed(),vehicle->getAcceleration());

				StopPoint* sp = tcoord->getNewAssignedStopPoint(vehicle->getID());
				EV << "The proposal of vehicle: " << vehicle->getID() << " has been accepted for requestID:  " << sp->getRequestID() << endl;
				vehicle->setSrcAddr(myAddress);
				vehicle->setDestAddr(sp->getLocation());

				// reset times
				vehicle->setOptimalEstimatedTravelTime(netmanager->getHopDistance(myAddress, sp->getLocation()) * (netmanager->getXChannelLength() / vehicle->getSpeed()));// * (netmanager->getXChannelLength() / vehicle->getSpeed())));
				vehicle->setCurrentTraveledTime(0);
				vehicle->setHopCount(0);

				if (netmanager->checkHospitalNode(myAddress)){
					emit(signal_ambulancesIdle,--currentVehiclesInNode);
				}

				EV << "Sending Vehicle from: " << vehicle->getSrcAddr() << " to " << vehicle->getDestAddr() << endl;
				sendDelayed(vehicle, sendDelayTime, "out");

			}
		}
	}

}

double App::computeAccelererationTime(double speed, double acceleration) //Evaluate Additional Travel Time due to acceleration and deceleration
    {
	double additionalTravelTime;
        if(acceleration<=0) {additionalTravelTime=0; return 0;}
        else{
            double Ta=speed/acceleration;
            double D = 0.5*acceleration*pow(Ta, 2);
            double Ta_prime = D/speed;

            additionalTravelTime = 2*(Ta - Ta_prime);
            return additionalTravelTime;
        }
    }
