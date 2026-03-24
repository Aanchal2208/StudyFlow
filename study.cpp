#include "studyy.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;

static int nextTaskId = 1;

void addTask(vector<Task>& tasks) {
    Task t;
    t.id = nextTaskId++;
    
    cout << "Enter subject: ";
    cin.ignore();
    getline(cin, t.subject);
    
    cout << "Enter deadline (days from now): ";
    cin >> t.deadline;
    
    cout << "Enter duration (hours needed): ";
    cin >> t.duration;
    
    cout << "Enter priority (1-10): ";
    cin >> t.priority;
    
    tasks.push_back(t);
    cout << "Task added successfully with ID: " << t.id << "\n";
}

void displayTasks(const vector<Task>& tasks) {
    if (tasks.empty()) {
        cout << "No tasks available.\n";
        return;
    }
    
    cout << "\n" << string(70, '-') << "\n";
    cout << left << setw(5) << "ID" 
         << setw(25) << "Subject" 
         << setw(12) << "Deadline"
         << setw(12) << "Duration"
         << setw(10) << "Priority" << "\n";
    cout << string(70, '-') << "\n";
    
    for (const auto& t : tasks) {
        cout << left << setw(5) << t.id 
             << setw(25) << t.subject 
             << setw(12) << (to_string(t.deadline) + " days")
             << setw(12) << (to_string(t.duration) + " hrs")
             << setw(10) << t.priority << "\n";
    }
    cout << string(70, '-') << "\n";
}

// Quick Sort implementation for sorting
static int partitionByDeadline(vector<Task>& tasks, int low, int high) {
    int pivot = tasks[high].deadline;
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (tasks[j].deadline <= pivot) {
            i++;
            swap(tasks[i], tasks[j]);
        }
    }
    swap(tasks[i + 1], tasks[high]);
    return i + 1;
}

static void quickSortByDeadline(vector<Task>& tasks, int low, int high) {
    if (low < high) {
        int pi = partitionByDeadline(tasks, low, high);
        quickSortByDeadline(tasks, low, pi - 1);
        quickSortByDeadline(tasks, pi + 1, high);
    }
}

void sortTasksByDeadline(vector<Task>& tasks) {
    if (!tasks.empty()) {
        quickSortByDeadline(tasks, 0, tasks.size() - 1);
        cout << "Tasks sorted by deadline.\n";
    }
}

static int partitionByPriority(vector<Task>& tasks, int low, int high) {
    int pivot = tasks[high].priority;
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (tasks[j].priority >= pivot) {  // descending order
            i++;
            swap(tasks[i], tasks[j]);
        }
    }
    swap(tasks[i + 1], tasks[high]);
    return i + 1;
}

static void quickSortByPriority(vector<Task>& tasks, int low, int high) {
    if (low < high) {
        int pi = partitionByPriority(tasks, low, high);
        quickSortByPriority(tasks, low, pi - 1);
        quickSortByPriority(tasks, pi + 1, high);
    }
}

void sortTasksByPriority(vector<Task>& tasks) {
    if (!tasks.empty()) {
        quickSortByPriority(tasks, 0, tasks.size() - 1);
        cout << "Tasks sorted by priority (highest first).\n";
    }
}

// Greedy algorithm: select maximum tasks that can be completed before deadlines
vector<Task> scheduleTasksGreedy(vector<Task>& tasks) {
    vector<Task> scheduled;
    
    if (tasks.empty()) {
        cout << "No tasks to schedule.\n";
        return scheduled;
    }
    
    // Sort by deadline
    vector<Task> sortedTasks = tasks;
    quickSortByDeadline(sortedTasks, 0, sortedTasks.size() - 1);
    
    int currentTime = 0;
    
    for (const auto& t : sortedTasks) {
        if (currentTime + t.duration <= t.deadline * 8) {  // assuming 8 study hours per day
            scheduled.push_back(t);
            currentTime += t.duration;
        }
    }
    
    cout << "\n=== Greedy Schedule ===\n";
    cout << "Tasks that can be completed before deadlines:\n";
    displayTasks(scheduled);
    cout << "Total scheduled tasks: " << scheduled.size() << "\n";
    cout << "Total time required: " << currentTime << " hours\n";
    
    return scheduled;
}

// 0/1 Knapsack: maximize priority within available time
vector<Task> knapsackOptimization(const vector<Task>& tasks, int totalTime) {
    int n = tasks.size();
    
    if (n == 0 || totalTime <= 0) {
        cout << "No tasks or no time available.\n";
        return {};
    }
    
    // DP table
    vector<vector<int>> dp(n + 1, vector<int>(totalTime + 1, 0));
    
    // Build table
    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= totalTime; w++) {
            if (tasks[i-1].duration <= w) {
                dp[i][w] = max(dp[i-1][w], 
                              dp[i-1][w - tasks[i-1].duration] + tasks[i-1].priority);
            } else {
                dp[i][w] = dp[i-1][w];
            }
        }
    }
    
    // Backtrack to find selected tasks
    vector<Task> selected;
    int w = totalTime;
    for (int i = n; i > 0 && w > 0; i--) {
        if (dp[i][w] != dp[i-1][w]) {
            selected.push_back(tasks[i-1]);
            w -= tasks[i-1].duration;
        }
    }
    
    cout << "\n=== Knapsack Optimization ===\n";
    cout << "Available time: " << totalTime << " hours\n";
    cout << "Maximum priority achievable: " << dp[n][totalTime] << "\n";
    cout << "Selected tasks:\n";
    displayTasks(selected);
    
    return selected;
}

void loadTasksFromFile(vector<Task>& tasks, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "No existing task file found. Starting fresh.\n";
        return;
    }
    
    tasks.clear();
    Task t;
    while (file >> t.id >> ws && getline(file, t.subject)) {
        file >> t.deadline >> t.duration >> t.priority;
        tasks.push_back(t);
        if (t.id >= nextTaskId) nextTaskId = t.id + 1;
    }
    
    file.close();
    cout << "Loaded " << tasks.size() << " tasks from file.\n";
}

void saveTasksToFile(const vector<Task>& tasks, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error saving tasks.\n";
        return;
    }
    
    for (const auto& t : tasks) {
        file << t.id << "\n" << t.subject << "\n" 
             << t.deadline << " " << t.duration << " " << t.priority << "\n";
    }
    
    file.close();
    cout << "Tasks saved successfully.\n";
}

void studyPlannerMenu(vector<Task>& tasks) {
    int choice;
    
    do {
        cout << "\n=== Study Planner ===\n";
        cout << "1. Add Task\n";
        cout << "2. Display All Tasks\n";
        cout << "3. Sort by Deadline\n";
        cout << "4. Sort by Priority\n";
        cout << "5. Greedy Schedule\n";
        cout << "6. Knapsack Optimization\n";
        cout << "7. Save Tasks\n";
        cout << "8. Load Tasks\n";
        cout << "0. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: addTask(tasks); break;
            case 2: displayTasks(tasks); break;
            case 3: sortTasksByDeadline(tasks); displayTasks(tasks); break;
            case 4: sortTasksByPriority(tasks); displayTasks(tasks); break;
            case 5: scheduleTasksGreedy(tasks); break;
            case 6: {
                int time;
                cout << "Enter available study time (hours): ";
                cin >> time;
                knapsackOptimization(tasks, time);
                break;
            }
            case 7: saveTasksToFile(tasks, "tasks.dat"); break;
            case 8: loadTasksFromFile(tasks, "tasks.dat"); break;
            case 0: cout << "Returning to main menu.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
