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
    state = -1; //IDLE

    specialVehicle=0; //veicolo di emergenza

//    chosenGate = 0;

}

Vehicle::~Vehicle() {
}


int Vehicle::getSpecialVehicle() const
{
    return specialVehicle;
}
void Vehicle::setSpecialVehicle(int specialVehicle)
{
    this->specialVehicle = specialVehicle;
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

int Vehicle::getState() const
{
    return state;
}

void Vehicle::setState(int state)
{
    this->state = state;
}

int Vehicle::getChosenGate() {
	return chosenGate;
}


void Vehicle::setChosenGate(int gate) {
	this->chosenGate = gate;
}
