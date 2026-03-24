#ifndef HEALTH_H
#define HEALTH_H

#include <vector>

struct Health {
    int day;
    double sleepHours;
    double waterIntake;  // liters
};

void addHealthData(std::vector<Health>& healthLog);
void displayHealthData(const std::vector<Health>& healthLog);
int getAvailableStudyTime(const std::vector<Health>& healthLog);
void healthTrackerMenu(std::vector<Health>& healthLog);

#endif
