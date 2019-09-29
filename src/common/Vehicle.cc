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
    weight = 1;
    specialVehicle=0;
    currentTraveledTime = 0.0;
    passengers=0;
    /**
     * default speed = 9.7 mps
     *  (35 km/h)
     */
    speed = 9.7;
    this->acceleration = 1.676; //[mpss]
}

/*  special vehicle
 * -1 civil
 *  0 taxi (unused)
 *  1 ambulance
 *  2 truck
 */
Vehicle::Vehicle(int specialVehicle, double speed, int Weight) {
    id = ++nextID;
    setName((std::to_string(id)).c_str());
    traveledDistance = 0.0;
    seats = 1;
    this->weight = Weight;
    this->specialVehicle=specialVehicle;
    passengers=0;
    currentTraveledTime = 0.0;
    /**
     * default speed = 9.7 [mps]
     *  (35 km/h)
     */
    this->speed = speed;
    this->acceleration = 1.676; //[mpss]
}
Vehicle::Vehicle(int specialVehicle, double speed, int Weight, int seats) {
		id = ++nextID;
	    setName((std::to_string(id)).c_str());
	    traveledDistance = 0.0;
	    this->seats = seats;
	    this->weight = Weight;
	    this->specialVehicle=specialVehicle;
	    passengers=0;
	    currentTraveledTime = 0.0;
	    /**
	     * default speed = 9.7 [mps]
	     *  (35 km/h)
	     */
	    this->speed = speed;
	    this->acceleration = 1.676; //[mpss]
}

Vehicle::~Vehicle() {
}


int Vehicle::getSpecialVehicle() const
{
    return specialVehicle;
}

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

double Vehicle::getAcceleration() const {
	return acceleration;
}

int Vehicle::getPassengers() const {
	return passengers;
}


void Vehicle::setPassengers(int passengers) {
	this->passengers = passengers;
}

void Vehicle::setSpecialVehicle(int specialVehicle) {
	this->specialVehicle = specialVehicle;
}

void Vehicle::setOptimalEstimatedTravelTime(double optimalEstimatedTravelTime) {
	this->optimalEstimatedTravelTime = optimalEstimatedTravelTime;
}

int Vehicle::getWeight() const {
	return weight;
}


void Vehicle::setSpeed(double speed) {
	this->speed = speed;
}
