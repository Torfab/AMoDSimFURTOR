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

#ifndef __AMOD_SIMULATOR_MANHATTANNETWORKMANAGER_H_
#define __AMOD_SIMULATOR_MANHATTANNETWORKMANAGER_H_

#include <omnetpp.h>
#include <AbstractNetworkManager.h>
#include <set>

class ManhattanNetworkManager : public AbstractNetworkManager
{
private:
    int rows;
    int columns;
    int numberOfEmergencyVehicles;

    double xChannelLength;
    double yChannelLength;
    double xTravelTime;
    double yTravelTime;

    simsignal_t newCivilVehicle;
    simsignal_t signal_ambulanceTravelTime;


    std::set<int> setOfDestroyedNodes;
    std::set<int> setOfNodesInRedZone;      //good neighbours of destroyed nodes
    std::set<int> setOfAvailableNodes;      //nodes not destroyed

private:
   ~ManhattanNetworkManager();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
//    virtual void propagateEarthquakeBetweenNodes(int epicenterAddress);
//	void buildSetOfNodesInRedZone();
	void buildsetOfBorderNodes();
    void buildsetOfAvailableNodes();
	void buildSetOfDestroyedNodes();
//	void buildSetOfNodesInRedZone(std::set<int> auxSet);
	void buildSetOfBorderNodes();
	void buildTruckStartNode();
	int pickRandomElemFromSet(std::set<int> s);
	void buildHospitalNodes();
	void buildSkilledHospitalNodes();
	void buildCollectionPointNodes();
	void buildStoragePointNodes();
	int pickRandomStoragePointNode();
	void initTopo(cTopology* topology); //initialize topology

    virtual int pickSkilledHospitalFromNode(int addr);
    virtual int getClosestExitNode(int address) override;



  public:
	virtual void insertDestroyedNode(int addr) override;
	virtual void removeRedZoneNode(int addr) override;
	virtual void updateTopology(cTopology* topology, int channelWeight) override;
	 virtual void insertRedZoneNode(int addr) override;
	 virtual double getManhattanDistanceX(int srcAddr, int dstAddr) override;  //Get the manhattan distance from srcAddr to dstAddr
	 virtual double getManhattanDistanceY(int srcAddr, int dstAddr) override;  //Get the manhattan distance from srcAddr to dstAddr
	 virtual double getManhattanDistance(int srcAddr, int dstAddr) override;
    virtual double getTimeDistance(int srcAddr, int dstAddr) override;
    virtual double getSpaceDistance(int srcAddr, int dstAddr) override;
    virtual double getHopDistance(int srcAddr, int dstAddr) override;
    virtual double getChannelLength(int nodeAddr, int gateIndex) override;
    virtual double getXChannelLength() override;
    virtual int getOutputGate(int srcAddr, int destAddr) override;
    virtual int getVehiclesPerNode(int nodeAddr) override;
    virtual bool isValidAddress(int nodeAddr) override;
    virtual bool checkDestroyedNode(int addr) override;
    virtual bool checkBorderNode(int addr) override;
    virtual bool checkRedZoneNode(int addr) override;
    virtual bool checkHospitalNode(int addr) override;
    virtual int checkStoragePointNode(int addr) override;
    virtual int pickRandomNodeInRedZone()override;
    virtual int pickClosestHospitalFromNode(int addr) override;
    virtual bool checkCollectionPointNode(int addr) override;
    virtual int pickClosestCollectionPointFromNode(int addr) override;
    virtual int pickRandomCollectionPointNode() override;
    virtual void emit_signal_ambulanceTravelTime(int signal_value) override;
};

#endif
