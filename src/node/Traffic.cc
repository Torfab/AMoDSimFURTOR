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

void Traffic::increaseTraffic(int i) {
	traffic[i]++;
}

const double Traffic::getTraffic(int i) const {
	return traffic[i];
}

int Traffic::getNumberOfGates() const {
	return numberOfGates;
}

void Traffic::decay(int i) {
		traffic[i]--;

}
/*
 * La formula è stata inventata "a mente"
 */
double Traffic::trafficInfluence(int i) {
	return traffic[i] * 0.05; // y = 0.05x + 1
}
