#ifndef STUDY_H
#define STUDY_H

#include <string>
#include <vector>

struct Task {
    int id;
    std::string subject;
    int deadline;      // days from now
    int duration;      // hours needed
    int priority;      // 1-10, higher = more important
};

void addTask(std::vector<Task>& tasks);
void displayTasks(const std::vector<Task>& tasks);
void sortTasksByDeadline(std::vector<Task>& tasks);
void sortTasksByPriority(std::vector<Task>& tasks);
std::vector<Task> scheduleTasksGreedy(std::vector<Task>& tasks);
std::vector<Task> knapsackOptimization(const std::vector<Task>& tasks, int totalTime);
void loadTasksFromFile(std::vector<Task>& tasks, const std::string& filename);
void saveTasksToFile(const std::vector<Task>& tasks, const std::string& filename);
void studyPlannerMenu(std::vector<Task>& tasks);

#endif

