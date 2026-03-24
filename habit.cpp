#include "habitt.h"
#include <iostream>
#include <iomanip>

using namespace std;

static int nextHabitId = 1;

void addHabit(vector<Habit>& habits) {
    Habit h;
    h.id = nextHabitId++;
    
    cout << "Enter habit name: ";
    cin.ignore();
    getline(cin, h.name);
    
    cout << "Enter daily time spent (minutes): ";
    cin >> h.timeSpent;
    
    habits.push_back(h);
    cout << "Habit added with ID: " << h.id << "\n";
}

void displayHabits(const vector<Habit>& habits) {
    if (habits.empty()) {
        cout << "No habits tracked.\n";
        return;
    }
    
    cout << "\n" << string(45, '-') << "\n";
    cout << left << setw(8) << "ID" 
         << setw(25) << "Habit"
         << setw(12) << "Time/Day" << "\n";
    cout << string(45, '-') << "\n";
    
    for (const auto& h : habits) {
        cout << left << setw(8) << h.id 
             << setw(25) << h.name
             << h.timeSpent << " min\n";
    }
    cout << string(45, '-') << "\n";
}

int calculateTotalHabitTime(const vector<Habit>& habits) {
    int total = 0;
    for (const auto& h : habits) {
        total += h.timeSpent;
    }
    return total;
}

void habitTrackerMenu(vector<Habit>& habits) {
    int choice;
    
    do {
        cout << "\n=== Habit Tracker ===\n";
        cout << "1. Add Habit\n";
        cout << "2. Display Habits\n";
        cout << "3. Calculate Total Habit Time\n";
        cout << "0. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: addHabit(habits); break;
            case 2: displayHabits(habits); break;
            case 3: {
                int total = calculateTotalHabitTime(habits);
                cout << "Total daily habit time: " << total << " minutes ("
                     << total / 60 << "h " << total % 60 << "m)\n";
                break;
            }
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
