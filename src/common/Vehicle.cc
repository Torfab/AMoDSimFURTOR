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

#include <Vehicle.h>

int Vehicle::nextID = 0;

Vehicle::Vehicle() {
    id = ++nextID;
    setName((std::to_string(id)).c_str());
    traveledDistance = 0.0;
    seats = 1;
//    busyState = false; //IDLE
    trafficWeight = 1;

    /*  veicolo di emergenza
     * -1 veicolo civile
     *  0 taxi
     *  1 veicolo di emergenza
     *  2 truck
     */
    specialVehicle=0;
    currentTraveledTime = 0.0;
    /**
     * default speed = 9.7 mps
     *  (35 km/h)
     */
    speed = 9.7;
}

/*  veicolo di emergenza
 * -1 veicolo civile
 *  0 taxi
 *  1 veicolo di emergenza
 */
Vehicle::Vehicle(int specialVehicle, double speed, int trafficWeight) {
    id = ++nextID;
    setName((std::to_string(id)).c_str());
    traveledDistance = 0.0;
    seats = 1;
//    busyState = false; //IDLE
    this->trafficWeight = trafficWeight;

    this->specialVehicle=specialVehicle;

    currentTraveledTime = 0.0;
    /**
     * default speed = 9.7 mps
     *  (35 km/h)
     */
    this->speed = speed;
}

Vehicle::~Vehicle() {
}


int Vehicle::getSpecialVehicle() const
{
    return specialVehicle;
}
//void Vehicle::setSpecialVehicle(int specialVehicle)
//{
//    this->specialVehicle = specialVehicle;
//}

int Vehicle::getID() const
{
    return id;
}

int Vehicle::getSeats() const
{
    return seats;
}

void Vehicle::setSeats(int seats)
{
    this->seats = seats;
}

double Vehicle::getTraveledDistance() const
{
    return traveledDistance;
}

void Vehicle::setTraveledDistance(double distance)
{
    this->traveledDistance = distance;
}


int Vehicle::getChosenGate() {
	return chosenGate;
}


void Vehicle::setChosenGate(int gate) {
	this->chosenGate = gate;
}

double Vehicle::getSpeed() const {
	return speed;
}


double Vehicle::getCurrentTraveledTime() const {
	return currentTraveledTime;
}

void Vehicle::setCurrentTraveledTime(double currentTraveledTime) {
	this->currentTraveledTime = currentTraveledTime;
}

double Vehicle::getOptimalEstimatedTravelTime() const {
	return optimalEstimatedTravelTime;
}

void Vehicle::setOptimalEstimatedTravelTime(double optimalEstimatedTravelTime) {
	this->optimalEstimatedTravelTime = optimalEstimatedTravelTime;
}

int Vehicle::getTrafficWeight() const {
	return trafficWeight;
}


void Vehicle::setSpeed(double speed) {
	this->speed = speed;
}
