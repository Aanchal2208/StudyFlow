#ifndef EXPENSE_H
#define EXPENSE_H

#include <string>
#include <vector>

struct Expense {
    int id;
    double amount;
    std::string category;
};

void addExpense(std::vector<Expense>& expenses);
void displayExpenses(const std::vector<Expense>& expenses);
void sortExpenses(std::vector<Expense>& expenses);
int searchExpense(const std::vector<Expense>& expenses, double amount);
void loadExpensesFromFile(std::vector<Expense>& expenses, const std::string& filename);
void saveExpensesToFile(const std::vector<Expense>& expenses, const std::string& filename);
void expenseTrackerMenu(std::vector<Expense>& expenses);

#endif
