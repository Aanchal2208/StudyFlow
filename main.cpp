#include <iostream>
#include <vector>
#include "studyy.h"
#include "expensee.h"
#include "habitt.h"
#include "healthh.h"
#include "reminderr.h"

using namespace std;

void displayMainMenu() {
    cout << "\n";
    cout << "╔════════════════════════════════════════╗\n";
    cout << "║     STUDENT LIFE MANAGEMENT SYSTEM     ║\n";
    cout << "╠════════════════════════════════════════╣\n";
    cout << "║  1. Study Planner                      ║\n";
    cout << "║  2. Expense Tracker                    ║\n";
    cout << "║  3. Habit Tracker                      ║\n";
    cout << "║  4. Health Tracker                     ║\n";
    cout << "║  5. Reminders                          ║\n";
    cout << "║  6. Quick Overview                     ║\n";
    cout << "║  0. Exit                               ║\n";
    cout << "╚════════════════════════════════════════╝\n";
    cout << "Enter choice: ";
}

void showQuickOverview(const vector<Task>& tasks, 
                       const vector<Expense>& expenses,
                       const vector<Habit>& habits,
                       const vector<Health>& healthLog) {
    cout << "\n=== Quick Overview ===\n";
    cout << "Tasks pending: " << tasks.size() << "\n";
    
    // Count urgent tasks
    int urgent = 0;
    for (const auto& t : tasks) {
        if (t.deadline <= 2 || t.priority >= 8) urgent++;
    }
    cout << "Urgent tasks: " << urgent << "\n";
    
    // Total expenses
    double totalExpense = 0;
    for (const auto& e : expenses) {
        totalExpense += e.amount;
    }
    cout << "Total expenses: $" << fixed << setprecision(2) << totalExpense << "\n";
    
    // Habit time
    int habitTime = calculateTotalHabitTime(habits);
    cout << "Daily habit time: " << habitTime << " minutes\n";
    
    // Study time recommendation
    int studyTime = getAvailableStudyTime(healthLog);
    cout << "Recommended study time today: " << studyTime << " hours\n";
}

int main() {
    vector<Task> tasks;
    vector<Expense> expenses;
    vector<Habit> habits;
    vector<Health> healthLog;
    vector<Reminder> reminders;
    
    // Load saved data on startup
    loadTasksFromFile(tasks, "tasks.dat");
    loadExpensesFromFile(expenses, "expenses.dat");
    
    int choice;
    
    cout << "Welcome to Student Life Management System!\n";
    
    do {
        displayMainMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                studyPlannerMenu(tasks);
                break;
            case 2:
                expenseTrackerMenu(expenses);
                break;
            case 3:
                habitTrackerMenu(habits);
                break;
            case 4:
                healthTrackerMenu(healthLog);
                break;
            case 5:
                reminderMenu(reminders, tasks);
                break;
            case 6:
                showQuickOverview(tasks, expenses, habits, healthLog);
                break;
            case 0:
                cout << "Saving data...\n";
                saveTasksToFile(tasks, "tasks.dat");
                saveExpensesToFile(expenses, "expenses.dat");
                cout << "Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);
    
    return 0;
}
