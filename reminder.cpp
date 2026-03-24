#include "reminderr.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

void addReminder(vector<Reminder>& reminders, const vector<Task>& tasks) {
    if (tasks.empty()) {
        cout << "No tasks available. Add tasks first.\n";
        return;
    }
    
    int taskId;
    cout << "Enter Task ID to create reminder: ";
    cin >> taskId;
    
    // Find the task
    for (const auto& t : tasks) {
        if (t.id == taskId) {
            Reminder r;
            r.taskId = t.id;
            r.priority = t.priority;
            r.deadline = t.deadline;
            r.subject = t.subject;
            reminders.push_back(r);
            cout << "Reminder created for: " << t.subject << "\n";
            return;
        }
    }
    
    cout << "Task not found.\n";
}

// Sort reminders by priority (descending) then deadline (ascending)
static void sortReminders(vector<Reminder>& reminders) {
    sort(reminders.begin(), reminders.end(), [](const Reminder& a, const Reminder& b) {
        if (a.priority != b.priority)
            return a.priority > b.priority;
        return a.deadline < b.deadline;
    });
}

void displayReminders(const vector<Reminder>& reminders) {
    if (reminders.empty()) {
        cout << "No reminders set.\n";
        return;
    }
    
    cout << "\n" << string(60, '=') << "\n";
    cout << "                    REMINDERS\n";
    cout << string(60, '=') << "\n";
    
    cout << left << setw(8) << "TaskID"
         << setw(25) << "Subject"
         << setw(12) << "Deadline"
         << setw(10) << "Priority" << "\n";
    cout << string(60, '-') << "\n";
    
    for (const auto& r : reminders) {
        string urgency = "";
        if (r.deadline <= 1) urgency = " [URGENT!]";
        else if (r.deadline <= 3) urgency = " [Soon]";
        
        cout << left << setw(8) << r.taskId
             << setw(25) << (r.subject + urgency)
             << setw(12) << (to_string(r.deadline) + " days")
             << setw(10) << r.priority << "\n";
    }
    cout << string(60, '=') << "\n";
}

void generatePriorityReminders(vector<Reminder>& reminders, const vector<Task>& tasks) {
    reminders.clear();
    
    for (const auto& t : tasks) {
        // Auto-create reminders for high priority or near deadline tasks
        if (t.priority >= 7 || t.deadline <= 3) {
            Reminder r;
            r.taskId = t.id;
            r.priority = t.priority;
            r.deadline = t.deadline;
            r.subject = t.subject;
            reminders.push_back(r);
        }
    }
    
    sortReminders(reminders);
    
    cout << "Generated " << reminders.size() << " priority reminders.\n";
    displayReminders(reminders);
}

void reminderMenu(vector<Reminder>& reminders, const vector<Task>& tasks) {
    int choice;
    
    do {
        cout << "\n=== Reminder System ===\n";
        cout << "1. Add Reminder for Task\n";
        cout << "2. View All Reminders\n";
        cout << "3. Auto-Generate Priority Reminders\n";
        cout << "0. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: addReminder(reminders, tasks); break;
            case 2: 
                sortReminders(reminders);
                displayReminders(reminders); 
                break;
            case 3: generatePriorityReminders(reminders, tasks); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
