#include "healthh.h"
#include <iostream>
#include <iomanip>

using namespace std;

static int currentDay = 1;

void addHealthData(vector<Health>& healthLog) {
    Health h;
    h.day = currentDay++;
    
    cout << "Enter sleep hours (last night): ";
    cin >> h.sleepHours;
    
    cout << "Enter water intake (liters): ";
    cin >> h.waterIntake;
    
    healthLog.push_back(h);
    cout << "Health data logged for Day " << h.day << "\n";
}

void displayHealthData(const vector<Health>& healthLog) {
    if (healthLog.empty()) {
        cout << "No health data recorded.\n";
        return;
    }
    
    cout << "\n" << string(40, '-') << "\n";
    cout << left << setw(8) << "Day" 
         << setw(15) << "Sleep (hrs)"
         << setw(15) << "Water (L)" << "\n";
    cout << string(40, '-') << "\n";
    
    double totalSleep = 0, totalWater = 0;
    
    for (const auto& h : healthLog) {
        cout << left << setw(8) << h.day 
             << setw(15) << fixed << setprecision(1) << h.sleepHours
             << setw(15) << h.waterIntake << "\n";
        totalSleep += h.sleepHours;
        totalWater += h.waterIntake;
    }
    
    cout << string(40, '-') << "\n";
    cout << "Averages: " << fixed << setprecision(1)
         << totalSleep / healthLog.size() << " hrs sleep, "
         << totalWater / healthLog.size() << " L water\n";
}

int getAvailableStudyTime(const vector<Health>& healthLog) {
    if (healthLog.empty()) {
        return 8;  // default 8 hours if no data
    }
    
    // Get latest entry
    double lastSleep = healthLog.back().sleepHours;
    
    // Calculate available study hours based on sleep
    // Optimal: 7-8 hours sleep = 8 hours study
    // Less sleep = less productive study time
    int studyHours;
    
    if (lastSleep >= 7) {
        studyHours = 8;
    } else if (lastSleep >= 5) {
        studyHours = 6;
    } else {
        studyHours = 4;
    }
    
    return studyHours;
}

void healthTrackerMenu(vector<Health>& healthLog) {
    int choice;
    
    do {
        cout << "\n=== Health Tracker ===\n";
        cout << "1. Log Today's Health Data\n";
        cout << "2. View Health History\n";
        cout << "3. Get Recommended Study Time\n";
        cout << "0. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: addHealthData(healthLog); break;
            case 2: displayHealthData(healthLog); break;
            case 3: {
                int hours = getAvailableStudyTime(healthLog);
                cout << "Based on your sleep data, recommended study time today: "
                     << hours << " hours\n";
                break;
            }
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
