#include "expensee.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

using namespace std;

static int nextExpenseId = 1;

void addExpense(vector<Expense>& expenses) {
    Expense e;
    e.id = nextExpenseId++;
    
    cout << "Enter amount: Rs.";
    cin >> e.amount;
    
    cout << "Enter category: ";
    cin.ignore();
    getline(cin, e.category);
    
    expenses.push_back(e);
    cout << "Expense added with ID: " << e.id << "\n";
}

void displayExpenses(const vector<Expense>& expenses) {
    if (expenses.empty()) {
        cout << "No expenses recorded.\n";
        return;
    }
    
    double total = 0;
    
    cout << "\n" << string(50, '-') << "\n";
    cout << left << setw(8) << "ID" 
         << setw(15) << "Amount"
         << setw(25) << "Category" << "\n";
    cout << string(50, '-') << "\n";
    
    for (const auto& e : expenses) {
        cout << left << setw(8) << e.id 
             << "$" << setw(14) << fixed << setprecision(2) << e.amount
             << setw(25) << e.category << "\n";
        total += e.amount;
    }
    
    cout << string(50, '-') << "\n";
    cout << "Total: $" << fixed << setprecision(2) << total << "\n";
}

// Merge Sort implementation
static void merge(vector<Expense>& expenses, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    vector<Expense> L(n1), R(n2);
    
    for (int i = 0; i < n1; i++)
        L[i] = expenses[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = expenses[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    
    while (i < n1 && j < n2) {
        if (L[i].amount <= R[j].amount) {
            expenses[k] = L[i];
            i++;
        } else {
            expenses[k] = R[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        expenses[k] = L[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        expenses[k] = R[j];
        j++;
        k++;
    }
}

static void mergeSort(vector<Expense>& expenses, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(expenses, left, mid);
        mergeSort(expenses, mid + 1, right);
        merge(expenses, left, mid, right);
    }
}

void sortExpenses(vector<Expense>& expenses) {
    if (!expenses.empty()) {
        mergeSort(expenses, 0, expenses.size() - 1);
        cout << "Expenses sorted by amount.\n";
    }
}

// Binary Search (requires sorted array)
int searchExpense(const vector<Expense>& expenses, double amount) {
    int left = 0, right = expenses.size() - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (abs(expenses[mid].amount - amount) < 0.01) {
            return mid;
        }
        if (expenses[mid].amount < amount) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

void loadExpensesFromFile(vector<Expense>& expenses, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "No existing expense file found.\n";
        return;
    }
    
    expenses.clear();
    Expense e;
    while (file >> e.id >> e.amount >> ws && getline(file, e.category)) {
        expenses.push_back(e);
        if (e.id >= nextExpenseId) nextExpenseId = e.id + 1;
    }
    
    file.close();
    cout << "Loaded " << expenses.size() << " expenses.\n";
}

void saveExpensesToFile(const vector<Expense>& expenses, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error saving expenses.\n";
        return;
    }
    
    for (const auto& e : expenses) {
        file << e.id << " " << e.amount << "\n" << e.category << "\n";
    }
    
    file.close();
    cout << "Expenses saved.\n";
}

void expenseTrackerMenu(vector<Expense>& expenses) {
    int choice;
    
    do {
        cout << "\n=== Expense Tracker ===\n";
        cout << "1. Add Expense\n";
        cout << "2. Display All Expenses\n";
        cout << "3. Sort by Amount\n";
        cout << "4. Search by Amount\n";
        cout << "5. Save Expenses\n";
        cout << "6. Load Expenses\n";
        cout << "0. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: addExpense(expenses); break;
            case 2: displayExpenses(expenses); break;
            case 3: sortExpenses(expenses); displayExpenses(expenses); break;
            case 4: {
                double amt;
                cout << "Enter amount to search: $";
                cin >> amt;
                sortExpenses(expenses);  // must be sorted for binary search
                int idx = searchExpense(expenses, amt);
                if (idx != -1) {
                    cout << "Found: ID " << expenses[idx].id 
                         << ", $" << expenses[idx].amount 
                         << ", " << expenses[idx].category << "\n";
                } else {
                    cout << "Expense not found.\n";
                }
                break;
            }
            case 5: saveExpensesToFile(expenses, "expenses.dat"); break;
            case 6: loadExpensesFromFile(expenses, "expenses.dat"); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
