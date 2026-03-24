#ifndef REMINDER_H
#define REMINDER_H

#include <vector>
#include "study.h"

struct Reminder {
    int taskId;
    int priority;
    int deadline;
    std::string subject;
};

void addReminder(std::vector<Reminder>& reminders, const std::vector<Task>& tasks);
void displayReminders(const std::vector<Reminder>& reminders);
void generatePriorityReminders(std::vector<Reminder>& reminders, const std::vector<Task>& tasks);
void reminderMenu(std::vector<Reminder>& reminders, const std::vector<Task>& tasks);

#endif
