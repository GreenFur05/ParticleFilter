/*
For milestone 2, my approach was to first determine if the robot had moved or
just changed direction. This would then determine whether or not the particle
list would update based on the previous list. If the robot moved, the new list
would be generated by comparing the possible particles generated from the
observation and the previus list minus the direction travelled. This allowed
for easy traversal when writing code.
Issues faced included working out where the robot had previously travelled, and
incorporating this into a new list.
A software design choice I made was trying to aim for high cohesion of classes  
and functions by containing repetitive code within a common method.
The basis of this code is highly procedural, containing a lot of if-statements
and for-loops. Generally, this is not the most ideal nor optimised solution
however it allowed for maintainable code that allowed objects and pointers to 
be passed around with ease.

For milestone 3, I used many of the similar design patterns as with milestone 2,
including adding a method for rotating the observation pattern to recognise
different orientations.

For milestone 4, the amount of rows and columns are initialised to only the 
amount of memory required for them. When creating new particle lists, they are generated in a separate array of pointers, and rather than performing a deep copy, only the pointer location is copied across. Also, when rotating the
observation pattern, it is rotated using a swap procedure to achieve a memory
complexity of O(n) rather than O(n^2) had it created another array.

*/
#include "ParticleFilter.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#define FIRST        0
#define MIDDLE       1
#define EDGES        3
#define ROTATIONS    4

// Initialise a new particle filter with a given maze of size (x,y)
ParticleFilter::ParticleFilter(char** maze, int rows, int cols) {

   // Initialise private attributes
   this->maze = maze;
   this->rows = rows;
   this->cols = cols;

   firstObservation = true;
   milestoneThree = false;

   // Creates an instance of ParticleList for use with this maze
   particleList = new ParticleList();

   // Instantiates the ParticleList with all possible particles robot
   // could be in without any obeservations
   initialiseParticleList();
}

// Clean-up the Particle Filter
ParticleFilter::~ParticleFilter() {
   particleList->clear();
   delete particleList;
}

// A new observation of the robot, of size 3x3
void ParticleFilter::newObservation(Grid observation) {

   Orientation initialOrientation = particleList->get(FIRST)->getOrientation();
   Orientation finalOrientation = ORIEN_UP;

   // Check for Milestone 3
   if (observation[MIDDLE][MIDDLE] == '*') {
      milestoneThree = true;
   }
   else {
      finalOrientation = checkOrientation(observation); 
   }

   // Create new particles from first observations of any rotation
   if (milestoneThree && firstObservation) {
      ParticleList* tempList = new ParticleList();

      char pos[4] = {'^','>','v','<'};

      for (int h = 0; h < ROTATIONS; h++) {
         // Selects direction based on rotation of observation
         observation[MIDDLE][MIDDLE] = pos[h];
         
         for (int i = 1; i < cols - MIDDLE; i++) {
            for (int j = 1; j < rows - MIDDLE; j++) {
               checkMaze(i,j,tempList,observation);
            }
         }

         // Rotate observation by 90 degrees
         observation = rotate(observation);
      }

      particleList->clear();
      particleList = tempList;

      milestoneThree = false;
      firstObservation = false;
   }
   // Robot has moved in a direction of unknown orientation
   else if (milestoneThree) {
      ParticleList* tempList = new ParticleList();
      ParticleList* newList = new ParticleList();

      char pos[4] = {'^','>','v','<'};

      for (int h = 0; h < ROTATIONS; h++) {
         // Selects direction based on rotation of observation
         observation[MIDDLE][MIDDLE] = pos[h];
         
         for (int i = 1; i < cols - MIDDLE; i++) {
            for (int j = 1; j < rows - MIDDLE; j++) {
               checkMaze(i,j,tempList,observation);
            }
         }

         // Rotate observation by 90 degrees
         observation = rotate(observation);
      }

      // Checks if robot is just changing direction and not moving
      bool changeDirection = true;
      if(tempList->getNumberParticles() == particleList->getNumberParticles()) {
         for (int i = 0; i < tempList->getNumberParticles(); i++) {
            Particle tempParticle = *(tempList->get(i));
            Particle particle = *(particleList->get(i));
            if (!(tempParticle.getX() == particle.getX() &&
            tempParticle.getY() == particle.getY() &&
            tempParticle.getOrientation() == particle.getOrientation())) {
               changeDirection = false;
            }
         }
      }
      else {
         changeDirection = false;
      }  
      

      // Robot has moved
      if (!(changeDirection)) {
         for (int i = 0; i < tempList->getNumberParticles(); i++) {

            for (int j = 0; j < particleList->getNumberParticles(); j++) {
               
               Particle tempParticle = *(tempList->get(i));
               Particle particle = *(particleList->get(j));

               Orientation direction = tempParticle.getOrientation();
               int horizontal = 0, vertical = 0;
               if (direction == ORIEN_UP) {
                  vertical = 1;
               }
               else if (direction == ORIEN_LEFT) {
                  horizontal = 1;
               }
               else if (direction == ORIEN_DOWN) {
                  vertical = -1;
               }
               else if (direction == ORIEN_RIGHT) {
                  horizontal = -1;
               }


               if (particle.getX() == tempParticle.getX() + horizontal &&
               particle.getY() == tempParticle.getY() + vertical) {
                  newList->add_back(new Particle(tempParticle.getX(),
                  tempParticle.getY(), direction));
               }
            }
         } 
      }

      particleList->clear();
      particleList = newList;

      tempList->clear();
      delete tempList;

      milestoneThree = false;
   }
   // Create new particles from first observation
   else if (firstObservation) {
      ParticleList* tempList = new ParticleList();

      for (int i = 1; i < cols - MIDDLE; i++) {
         for (int j = 1; j < rows - MIDDLE; j++) {
            checkMaze(i,j,tempList,observation);
         }
      }

      particleList->clear();
      particleList = tempList;

      firstObservation = false;

   }
   // Moved in a direction
   else if (initialOrientation == finalOrientation) {

      ParticleList* tempList = new ParticleList();
      ParticleList* newList = new ParticleList();

      for (int i = 1; i < cols - MIDDLE; i++) {
         for (int j = 1; j < rows - MIDDLE; j++) {
            checkMaze(i,j,tempList,observation);
         }
      }

      char direction = observation[MIDDLE][MIDDLE];
      int horizontal = 0, vertical = 0;
      if (direction == '^') {
         vertical = 1;
      }
      else if (direction == '<') {
         horizontal = 1;
      }
      else if (direction == 'v') {
         vertical = -1;
      }
      else if (direction == '>') {
         horizontal = -1;
      }

      // For each particle matched against current observation, check to see if
      // the particle from which the robot traversed from is in the previous
      // particle list.
      for (int i = 0; i < tempList->getNumberParticles(); i++) {
         for (int j = 0; j < particleList->getNumberParticles(); j++) {
            
            Particle tempParticle = *(tempList->get(i));
            Particle particle = *(particleList->get(j));

            if (particle.getX() == tempParticle.getX() + horizontal &&
            particle.getY() == tempParticle.getY() + vertical) {
               Orientation newOrientation = checkOrientation(observation);
               newList->add_back(new Particle(tempParticle.getX(),
               tempParticle.getY(), newOrientation));
            }
         }
      }


      particleList->clear();
      particleList = newList;

      tempList->clear();
      delete tempList;
   }
   // If neither condition met, robot has not moved, only changed orientation.
   else {
      ParticleList* tempList = new ParticleList();

      for (int i = 0; i < particleList->getNumberParticles(); i++) {
         tempList->add_back(new Particle(particleList->get(i)->getX(),
         particleList->get(i)->getY(), finalOrientation));
      }

      // Give addresses of tempList to particleList
      particleList->clear();
      particleList = tempList;
   }

}


// Return a DEEP COPY of the ParticleList of all particles representing
//    the current possible locations of the robot
ParticleList* ParticleFilter::getParticles() {
   ParticleList* list = new ParticleList();

   for (int i = 0; i < particleList->getNumberParticles(); i++) {
      list->add_back(new Particle(particleList->get(i)->getX(),
      particleList->get(i)->getY(), particleList->get(i)->getOrientation()));
   }

   return list;
}


// Check the maze against the current observation and add particle if a match.
// Using multiple returns rather than nested-if statements for code clarity
void ParticleFilter::checkMaze(int i, int j, ParticleList* list, Grid obs) {
   // (x,y) = (0,0)
   if (!(maze[j-1][i-1] == obs[0][0])) {
      return;
   }

   // (x,y) = (1,0)
   if (!(maze[j-1][i] == obs[0][1])) {
      return;
   }

   // (x,y) = (2,0)
   if (!(maze[j-1][i+1] == obs[0][2])) {
      return;
   }

   // (x,y) = (0,1)
   if (!(maze[j][i-1] == obs[1][0])) {
      return;
   }

   // (x,y) = (1,1)
   // Not required, not checking orientation

   // (x,y) = (2,1)
   if (!(maze[j][i+1] == obs[1][2])) {
      return;
   }

   // (x,y) = (0,2)
   if (!(maze[j+1][i-1] == obs[2][0])) {
      return;
   }

   // (x,y) = (1,2)
   if (!(maze[j+1][i] == obs[2][1])) {
      return;
   }

   // (x,y) = (2,2)
   if (!(maze[j+1][i+1] == obs[2][2])) {
      return;
   }

   Orientation newOrientation = checkOrientation(obs);

   // Add particle to supplied list
   list->add_back(new Particle(i,j,newOrientation));
}


Orientation ParticleFilter::checkOrientation(Grid obs) {
   char direction = obs[MIDDLE][MIDDLE];
   Orientation newOrientation = ORIEN_UP;
   if (direction == '^') {
      newOrientation = ORIEN_UP;
   }
   else if (direction == '<') {
      newOrientation = ORIEN_LEFT;
   }
   else if (direction == 'v') {
      newOrientation = ORIEN_DOWN;
   }
   else if (direction == '>') {
      newOrientation = ORIEN_RIGHT;
   }

   return newOrientation;
}

void ParticleFilter::initialiseParticleList() {

   for (int x = 0; x < cols; x++) {
      for (int y = 0; y < rows; y++) {
         if (maze[y][x] == '.') {
            particleList->add_back(new Particle(x,y,ORIEN_UP));
         }
      }
   }
}

Grid ParticleFilter::rotate(Grid obs) {

   for (int i = 0; i < EDGES; i++) {
      for (int j = i + 1; j < EDGES; j++) {
         char temp = obs[i][j];
         obs[i][j] = obs[j][i];
         obs[j][i] = temp;
      }
   }

   for (int i = 0; i < EDGES; i++) {
      for (int j = 0; j < EDGES / 2; j++) {
         char temp = obs[i][j];
         obs[i][j] = obs[i][EDGES - 1 - j];
         obs[i][EDGES - 1 - j] = temp;
      }
   }

   return obs;   
}


