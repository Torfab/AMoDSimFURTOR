//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <Traffic.h>

Traffic::~Traffic() {
	delete [] traffic;
}

Traffic::Traffic() {
	this->numberOfGates = 4;
	traffic = new double[numberOfGates];
	for (int i = 0; i < numberOfGates; i++) {
		traffic[i] = 0;
	}
}

/*
 * increases the traffic of weight
 * i index of vector
 * w traffic weight of the vehicle to increase
 */
void Traffic::increaseTraffic(int index, int weight) {
	traffic[index]+=weight;
}
const double Traffic::getTraffic(int i) const {
	return traffic[i];
}

int Traffic::getNumberOfGates() const {
	return numberOfGates;
}

/*
 * decreases the traffic of weight
 * i index of vector
 * w traffic weight of the vehicle to increase
 */
void Traffic::decay(int index,int weight) {
		traffic[index]-=weight;
}
/*
 * return the traffic influence corresponding to the current value
 * y = 0.05x
 */
double Traffic::trafficInfluence(int index) {
	return traffic[index] * 0.05;
}
