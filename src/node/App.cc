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
class App : public cSimpleModule,cListener
{
  private:
    // configuration
    int myAddress;
    int numberOfVehicles;
    int seatsPerVehicle;
    int boardingTime;
    int alightingTime;
    double additionalTravelTime;
    double ambulanceSpeed;
    int CivilDestinations;
//    int civilDest;
    int CivilTrafficN;

    BaseCoord *tcoord;
    AbstractNetworkManager *netmanager;

    // signals
    simsignal_t newTripAssigned;

  public:
    App();
    virtual ~App();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double vehicleID);
	void generateCivilTraffic();
};

Define_Module(App);


App::App()
{
    tcoord = NULL;
}

App::~App()
{
}

void App::generateCivilTraffic() {
	Vehicle* civile = new Vehicle(-1,9.7, 1);

	int destAddress = intuniform(0, CivilDestinations - 1, 3);
	while (destAddress == myAddress)
		destAddress = intuniform(0, CivilDestinations - 1, 3);
	civile->setSrcAddr(myAddress);
	civile->setDestAddr(destAddress);

	 EV << "New civil vehicle " << civile->getID() << " with dest: " << civile->getDestAddr() << endl;
	 double delay=uniform(0,3);
	sendDelayed(civile, delay, "out");
}

void App::initialize()
{
    myAddress = par("address");
    seatsPerVehicle = par("seatsPerVehicle");
    alightingTime = getParentModule()->getParentModule()->par("alightingTime");
    boardingTime = getParentModule()->getParentModule()->par("boardingTime");
    tcoord = check_and_cast<BaseCoord *>(getParentModule()->getParentModule()->getSubmodule("tcoord"));
    netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getParentModule()->getSubmodule("netmanager"));
    numberOfVehicles = netmanager->getVehiclesPerNode(myAddress);
    additionalTravelTime = netmanager->getAdditionalTravelTime();

    ambulanceSpeed = netmanager->getAmbulanceSpeed();

    CivilTrafficN = par("CivilTrafficN");

    cTopology* topo = new cTopology("topo");
    std::vector<std::string> nedTypes;
    nedTypes.push_back("src.node.Node");
    topo->extractByNedTypeName(nedTypes);
    cTopology::Node *node = topo->getNode(myAddress);

    if (netmanager->checkDisconnectedNode(myAddress)) {
        //KABOOM
        // disconnects channels

        for (int j = 0; j < node->getNumOutLinks(); j++) {
            cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();
            cGate *gate = node->getLinkOut(j)->getLocalGate();
            gate->disconnect();

        }
        return;
    } else {
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();

            if (netmanager->checkDisconnectedNode(neighbour->getModule()->getIndex())){
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

    //If the vehicle is in this node (at startup) subscribe it to "tripRequestSignal"
    if (numberOfVehicles > 0)
    {
        for(int i=0; i<numberOfVehicles; i++)
        {
            Vehicle *v = new Vehicle(1, ambulanceSpeed , 1);
            int hospitalAddress = netmanager->getHospitalAddress();
            if (myAddress == hospitalAddress){
//                v->setSpecialVehicle(1);  //ambulanza
                v->setSeats(1);
            } else
                v->setSeats(seatsPerVehicle);

            EV << "I am node " << myAddress << ". I HAVE THE VEHICLE " << v->getID() << "of type "<< v->getSpecialVehicle() << ". It has " << v->getSeats() << " seats." << endl;
            tcoord->registerVehicle(v, myAddress);
        }

        if (ev.isGUI())
            getParentModule()->getDisplayString().setTagArg("i",1,"green");

        //When the coordinator assign a new request to a vehicle, local node will be notified
        simulation.getSystemModule()->subscribe("newTripAssigned",this);
	}

	for (int i=0;i<CivilTrafficN;i++)
	generateCivilTraffic();

}


void App::handleMessage(cMessage *msg)
{
    Vehicle *vehicle = NULL;

    double sendDelayTime = additionalTravelTime;// * trafficFactor;

    try{
        //A vehicle is here
        vehicle = check_and_cast<Vehicle *>(msg);
    }catch (cRuntimeError e) {
        EV << "Can not handle received message! Ignoring..." << endl;
        return ;
    }

    EV << "Destination completed: VEHICLE " << vehicle->getID() << " after " << vehicle->getHopCount() << " hops. The type of vehicle is " <<  vehicle->getSpecialVehicle() <<endl;
    vehicle->setBusyState(false);


    // Civil vehicle
    if (vehicle->getSpecialVehicle() == -1){
    	if (vehicle->getDestAddr() == myAddress) {
    		EV << "Veicolo civile a destinazione " << vehicle->getDestAddr()<< " partito da "<< vehicle->getSrcAddr() <<endl;
    		generateCivilTraffic();
    		return;
    	}
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
	}


    StopPoint *currentStopPoint = tcoord->getCurrentStopPoint(vehicle->getID());

    if (currentStopPoint != NULL && currentStopPoint->getLocation() != -1 && currentStopPoint->getIsPickup())
    {
        //This is a PICK-UP stop-point
        double waitTimeMinutes = (simTime().dbl() - currentStopPoint->getTime()) / 60;
        EV << "The vehicle is here! Pickup time: " << simTime() << "; Request time: " << currentStopPoint->getTime() << "; Waiting time: " << waitTimeMinutes << "minutes." << endl;
    }

    //Ask to coordinator for next stop point
    StopPoint *nextStopPoint = tcoord->getNextStopPoint(vehicle->getID());
    if(nextStopPoint != NULL)
    {
        //There is another stop point for the vehicle!
        EV << "The next stop point for the vehicle " << vehicle->getID() << " is: " << nextStopPoint->getLocation() << endl;
        vehicle->setSrcAddr(myAddress);
        vehicle->setDestAddr(nextStopPoint->getLocation());

        //Time for boarding or drop-off passengers
        double delays = (nextStopPoint->getActualTime() - simTime().dbl()) - netmanager->getTimeDistance(myAddress, nextStopPoint->getLocation());
        if(delays <0)
            delays=0;

        if(nextStopPoint->getLocation() == myAddress)
            sendDelayed(vehicle,delays,"out");
        else
            sendDelayed(vehicle,sendDelayTime+delays,"out");
    }

    //No other stop point for the vehicle. The vehicle stay here
    else
    {
        EV << "Vehicle " << vehicle->getID() << " is in node " << myAddress << endl;
        tcoord->registerVehicle(vehicle, myAddress);

        if (ev.isGUI())
            getParentModule()->getDisplayString().setTagArg("i",1,"green");

        if (!simulation.getSystemModule()->isSubscribed("newTripAssigned",this))
            simulation.getSystemModule()->subscribe("newTripAssigned",this);
    }


}

/**
 * Handle an Omnet signal.
 * 
 * @param source
 * @param signalID
 * @param obj
 */
void App::receiveSignal(cComponent *source, simsignal_t signalID,
        double vehicleID) {

    /**
     * The coordinator has accepted a trip proposal
     */
    if (signalID == newTripAssigned) {
           if (tcoord->getLastVehicleLocation(vehicleID) == myAddress) {
            //The vehicle that should serve the request is in this node
            Vehicle *veic = tcoord->getVehicleByID(vehicleID);


            if (veic != NULL) {

            	veic->setBusyState(true);

                double sendDelayTime = additionalTravelTime;

                StopPoint* sp = tcoord->getNewAssignedStopPoint(veic->getID());
                EV << "The proposal of vehicle: " << veic->getID()
                          << " has been accepted for requestID:  "
                          << sp->getRequestID() << endl;
                veic->setSrcAddr(myAddress);
                veic->setDestAddr(sp->getLocation());

                //Time for boarding or dropoff
                double delays = (sp->getActualTime() - simTime().dbl())
                        - netmanager->getTimeDistance(myAddress,
                                sp->getLocation());
                if (delays < 0)
                    delays = 0;

                if (sp->getLocation() == myAddress)
                    sendDelayTime = delays;
                else
                    sendDelayTime = sendDelayTime + delays;

                EV << "Sending Vehicle from: " << veic->getSrcAddr() << " to "
                          << veic->getDestAddr() << endl;
                Enter_Method
                ("sendDelayed", veic, sendDelayTime, "out");
                sendDelayed(veic, sendDelayTime, "out");

                if (ev.isGUI())
                    getParentModule()->getDisplayString().setTagArg("i", 1,
                            "gold");
                //if (simulation.getSystemModule()->isSubscribed("tripRequestCoord",this))
                //  simulation.getSystemModule()->unsubscribe("tripRequestCoord",this);
            }
        }
    }

}
