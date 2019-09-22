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

#include "Pheromone.h"

Pheromone::Pheromone(double DecayTime, double DecayFactor) :
		pheromoneDecayTime(DecayTime), pheromoneDecayFactor(DecayFactor) {
	this->numberOfGates = 4;
	pheromone = new double[numberOfGates];
	for (int i = 0; i < numberOfGates; i++) {
		pheromone[i] = 0;
	}
}

Pheromone::~Pheromone() {
	delete [] pheromone;
}

double Pheromone::getPheromoneDecayFactor() const {
	return pheromoneDecayFactor;

}

double Pheromone::getPheromoneDecayTime() const {
	return pheromoneDecayTime;
}
/**
 * Increase pheromone value by weight value.
 */
void Pheromone::increasePheromone(int index, int weight) {
	pheromone[index] = pheromone[index] + weight;

}

const double Pheromone::getPheromone(int i) const {
	return pheromone[i];
}

int Pheromone::getNumberOfGates() const {
	return numberOfGates;
}

/**
 * Decay the pheromone of the current value * pheromoneDecayFactor
 */
void Pheromone::decayPheromone() {
	for (int i = 0; i < numberOfGates; i++) {
		pheromone[i] -= pheromone[i] * pheromoneDecayFactor;
	}
}

void Pheromone::setPheromone(int gate, int pheromone) {
	this->pheromone[gate] = pheromone;
}
