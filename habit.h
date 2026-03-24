#ifndef HABIT_H
#define HABIT_H

#include <string>
#include <vector>

struct Habit {
    int id;
    std::string name;
    int timeSpent;  // minutes per day
};

void addHabit(std::vector<Habit>& habits);
void displayHabits(const std::vector<Habit>& habits);
int calculateTotalHabitTime(const std::vector<Habit>& habits);
void habitTrackerMenu(std::vector<Habit>& habits);

#endif
