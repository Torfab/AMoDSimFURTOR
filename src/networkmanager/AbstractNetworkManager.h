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

#ifndef ABSTRACTNETWORKMANAGER_H_
#define ABSTRACTNETWORKMANAGER_H_


class AbstractNetworkManager : public cSimpleModule{

protected:
    std::map<int,int> vehiclesPerNode;  //Number of vehicles per node at simulation start
    int numberOfVehicles;               //Number of vehicles in the network
    int numberOfNodes;                  //Number of crossroads(nodes) in the network
    double additionalTravelTime;        //Additional Travel Time due to acceleration and deceleration
    double ambulanceSpeed;				// Ambulance Speed
    double truckSpeed;					// Truck Speed
    int hospitalAddress;
    int collectionPointAddress;
    int epicenterAddress;
    int disasterRadius;

    virtual void initialize() = 0;
    virtual void handleMessage(cMessage *msg) = 0;
    virtual std::set<int> propagateEarthquakeBetweenNodes(int epicenterAddress, std::set<int> auxSet) = 0;

  public:
    virtual double getTimeDistance(int srcAddr, int dstAddr)=0;      //Get the time needed to go from srcAddr to dstAddr
    virtual double getSpaceDistance(int srcAddr, int dstAddr)=0;     //Get the space-distance from srcAddr to dstAddr
    virtual double getChannelLength(int nodeAddr, int gateIndex)=0;  //Get the length of the channel connected to the specified gate
    virtual int getOutputGate(int srcAddr, int destAddr)=0;          //Get the index of the gate where send the packet to reach the destAddr
    virtual int getVehiclesPerNode(int nodeAddr)=0;                  //Get the number of vehicles located in the node at simulation start
    virtual bool isValidAddress(int nodeAddr)=0;                     //Check if the specified address is valid
    inline int getNumberOfVehicles(){return numberOfVehicles;}       //Get the fleet size
    inline int getNumberOfNodes(){return numberOfNodes;}            // Get the nodes
    inline double getAdditionalTravelTime(){return additionalTravelTime;} //Get the additional travel time due to acceleration and deceleration
    inline virtual double getAmbulanceSpeed() {return ambulanceSpeed;}
    inline virtual double getTruckSpeed() {return truckSpeed;}
    inline virtual int getHospitalAddress(){return hospitalAddress;}
    virtual bool checkDisconnectedNode(int addr) =0;


    double setAdditionalTravelTime(double speed, double acceleration) //Evaluate Additional Travel Time due to acceleration and deceleration
    {
        if(acceleration<=0) {additionalTravelTime=0; return 0;}
        else{
//            double Ta=speed/acceleration;
//            double D = 0.5*acceleration*pow(Ta, 2);
//            double Ta_prime = D/speed;
//
//            additionalTravelTime = 2*(Ta - Ta_prime);
//            return additionalTravelTime;
        	return 10;
        }
    }

	int getDisasterRadius() const {
		return disasterRadius;
	}
};


#endif /* ABSTRACTNETWORKMANAGER_H_ */
