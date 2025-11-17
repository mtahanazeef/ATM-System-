#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <ctime>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

// ===============================
// Constants and Configuration
// ===============================
const string ADMIN_PASSWORD = "admin123";
const double MAX_DEPOSIT = 10000.0;
const double MAX_WITHDRAWAL = 5000.0;
const int MAX_LOGIN_ATTEMPTS = 3;

// ===============================
// Data Structures
// ===============================
struct Transaction {
    string type;
    double amount;
    double balance;
    string timestamp;
};

struct Account {
    string accountNumber;
    string password;
    string accountHolderName;
    string phoneNumber;
    double balance;
    vector<Transaction> transactionHistory;
};

// ===============================
// Utility Functions
// ===============================
void clearScreen() {
    system("cls");
}

void pauseScreen(int milliseconds = 2000) {
    Sleep(milliseconds);
}

string getCurrentTimestamp() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timestamp(dt);
    timestamp.erase(timestamp.find('\n'), 1); // Remove newline
    return timestamp;
}

string formatCurrency(double amount) {
    stringstream ss;
    ss << fixed << setprecision(2) << "$" << amount;
    return ss.str();
}

void printHeader(const string& title) {
    cout << "\n";
    cout << "\t\t" << string(50, '=') << endl;
    cout << "\t\t" << title << endl;
    cout << "\t\t" << string(50, '=') << endl;
    cout << "\n";
}

void printSuccess(const string& message) {
    cout << "\x1b[32m✓ " << message << "\x1b[37m" << endl;
}

void printError(const string& message) {
    cout << "\x1b[31m✗ " << message << "\x1b[37m" << endl;
}

void printInfo(const string& message) {
    cout << "\x1b[36mℹ " << message << "\x1b[37m" << endl;
}

// ===============================
// File Operations
// ===============================
bool accountExists(const string& accountNumber) {
    ifstream file(accountNumber + ".txt");
    return file.good();
}

void saveAccount(const Account& account) {
    ofstream file(account.accountNumber + ".txt");
    file << account.accountNumber << endl;
    file << account.password << endl;
    file << account.accountHolderName << endl;
    file << account.phoneNumber << endl;
    file << account.balance << endl;
    
    // Save transaction history
    file << account.transactionHistory.size() << endl;
    for (const auto& transaction : account.transactionHistory) {
        file << transaction.type << endl;
        file << transaction.amount << endl;
        file << transaction.balance << endl;
        file << transaction.timestamp << endl;
    }
    file.close();
}

Account loadAccount(const string& accountNumber) {
    Account account;
    ifstream file(accountNumber + ".txt");
    
    if (file) {
        getline(file, account.accountNumber);
        getline(file, account.password);
        getline(file, account.accountHolderName);
        getline(file, account.phoneNumber);
        file >> account.balance;
        
        // Load transaction history
        int historySize;
        file >> historySize;
        file.ignore(); // Clear newline
        
        account.transactionHistory.clear();
        for (int i = 0; i < historySize; i++) {
            Transaction transaction;
            getline(file, transaction.type);
            file >> transaction.amount;
            file >> transaction.balance;
            file.ignore(); // Clear newline
            getline(file, transaction.timestamp);
            account.transactionHistory.push_back(transaction);
        }
        file.close();
    }
    return account;
}

void addTransaction(Account& account, const string& type, double amount) {
    Transaction transaction;
    transaction.type = type;
    transaction.amount = amount;
    transaction.balance = account.balance;
    transaction.timestamp = getCurrentTimestamp();
    
    account.transactionHistory.push_back(transaction);
    
    // Keep only last 50 transactions
    if (account.transactionHistory.size() > 50) {
        account.transactionHistory.erase(account.transactionHistory.begin());
    }
}

// ===============================
// Core Functions
// ===============================
void createAccount() {
    clearScreen();
    printHeader("CREATE NEW ACCOUNT");
    
    Account newAccount;
    
    cout << "Enter Account Number: ";
    cin >> newAccount.accountNumber;
    
    if (accountExists(newAccount.accountNumber)) {
        printError("Account number already exists!");
        pauseScreen();
        return;
    }
    
    cout << "Enter Password: ";
    cin >> newAccount.password;
    
    cout << "Enter Account Holder Name: ";
    cin.ignore();
    getline(cin, newAccount.accountHolderName);
    
    cout << "Enter Phone Number: ";
    cin >> newAccount.phoneNumber;
    
    cout << "Enter Initial Deposit: $";
    cin >> newAccount.balance;
    
    if (newAccount.balance < 0) {
        printError("Initial deposit cannot be negative!");
        pauseScreen();
        return;
    }
    
    // Add initial deposit transaction
    if (newAccount.balance > 0) {
        addTransaction(newAccount, "INITIAL DEPOSIT", newAccount.balance);
    }
    
    saveAccount(newAccount);
    printSuccess("Account created successfully!");
    pauseScreen(3000);
}

bool login(string& accountNumber) {
    clearScreen();
    printHeader("ACCOUNT LOGIN");
    
    string inputPassword;
    int attempts = 0;
    
    while (attempts < MAX_LOGIN_ATTEMPTS) {
        cout << "Enter Account Number: ";
        cin >> accountNumber;
        cout << "Enter Password: ";
        cin >> inputPassword;
        
        if (!accountExists(accountNumber)) {
            printError("Account not found!");
            attempts++;
            continue;
        }
        
        Account account = loadAccount(accountNumber);
        
        if (account.password == inputPassword) {
            printSuccess("Login successful! Welcome, " + account.accountHolderName + "!");
            pauseScreen(2000);
            return true;
        } else {
            attempts++;
            printError("Invalid password! Attempts remaining: " + to_string(MAX_LOGIN_ATTEMPTS - attempts));
        }
    }
    
    printError("Maximum login attempts exceeded!");
    pauseScreen(3000);
    return false;
}

void checkBalance(const string& accountNumber) {
    clearScreen();
    printHeader("ACCOUNT BALANCE");
    
    Account account = loadAccount(accountNumber);
    cout << "Account Holder: " << account.accountHolderName << endl;
    cout << "Account Number: " << account.accountNumber << endl;
    cout << "Current Balance: " << formatCurrency(account.balance) << endl;
    
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

void viewTransactionHistory(const string& accountNumber) {
    clearScreen();
    printHeader("TRANSACTION HISTORY");
    
    Account account = loadAccount(accountNumber);
    
    if (account.transactionHistory.empty()) {
        printInfo("No transactions found.");
    } else {
        cout << setw(20) << left << "Date/Time" 
             << setw(20) << left << "Type" 
             << setw(15) << right << "Amount" 
             << setw(15) << right << "Balance" << endl;
        cout << string(70, '-') << endl;
        
        for (const auto& transaction : account.transactionHistory) {
            cout << setw(20) << left << transaction.timestamp.substr(0, 16)
                 << setw(20) << left << transaction.type
                 << setw(15) << right << formatCurrency(transaction.amount)
                 << setw(15) << right << formatCurrency(transaction.balance) << endl;
        }
    }
    
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

void depositMoney(string& accountNumber) {
    clearScreen();
    printHeader("DEPOSIT MONEY");
    
    Account account = loadAccount(accountNumber);
    double amount;
    
    cout << "Current Balance: " << formatCurrency(account.balance) << endl;
    cout << "Enter amount to deposit: $";
    cin >> amount;
    
    if (amount <= 0) {
        printError("Deposit amount must be positive!");
        pauseScreen(2000);
        return;
    }
    
    if (amount > MAX_DEPOSIT) {
        printError("Maximum deposit limit is " + formatCurrency(MAX_DEPOSIT));
        pauseScreen(2000);
        return;
    }
    
    account.balance += amount;
    addTransaction(account, "DEPOSIT", amount);
    saveAccount(account);
    
    printSuccess("Amount deposited successfully!");
    printInfo("New Balance: " + formatCurrency(account.balance));
    
    cout << "\nPress 1 to print receipt, any other key to continue: ";
    string choice;
    cin >> choice;
    
    if (choice == "1") {
        // Receipt printing would go here
        printSuccess("Receipt generated!");
    }
    pauseScreen(2000);
}

void withdrawMoney(string& accountNumber) {
    clearScreen();
    printHeader("WITHDRAW MONEY");
    
    Account account = loadAccount(accountNumber);
    double amount;
    
    cout << "Current Balance: " << formatCurrency(account.balance) << endl;
    cout << "Enter amount to withdraw: $";
    cin >> amount;
    
    if (amount <= 0) {
        printError("Withdrawal amount must be positive!");
        pauseScreen(2000);
        return;
    }
    
    if (amount > MAX_WITHDRAWAL) {
        printError("Maximum withdrawal limit is " + formatCurrency(MAX_WITHDRAWAL));
        pauseScreen(2000);
        return;
    }
    
    if (amount > account.balance) {
        printError("Insufficient balance!");
        pauseScreen(2000);
        return;
    }
    
    account.balance -= amount;
    addTransaction(account, "WITHDRAWAL", amount);
    saveAccount(account);
    
    printSuccess("Amount withdrawn successfully!");
    printInfo("New Balance: " + formatCurrency(account.balance));
    
    cout << "\nPress 1 to print receipt, any other key to continue: ";
    string choice;
    cin >> choice;
    
    if (choice == "1") {
        printSuccess("Receipt generated!");
    }
    pauseScreen(2000);
}

void changePassword(string& accountNumber) {
    clearScreen();
    printHeader("CHANGE PASSWORD");
    
    Account account = loadAccount(accountNumber);
    string currentPassword, newPassword, confirmPassword;
    
    cout << "Enter current password: ";
    cin >> currentPassword;
    
    if (currentPassword != account.password) {
        printError("Current password is incorrect!");
        pauseScreen(2000);
        return;
    }
    
    cout << "Enter new password: ";
    cin >> newPassword;
    cout << "Confirm new password: ";
    cin >> confirmPassword;
    
    if (newPassword != confirmPassword) {
        printError("Passwords do not match!");
        pauseScreen(2000);
        return;
    }
    
    if (newPassword.length() < 4) {
        printError("Password must be at least 4 characters long!");
        pauseScreen(2000);
        return;
    }
    
    account.password = newPassword;
    saveAccount(account);
    printSuccess("Password changed successfully!");
    pauseScreen(2000);
}

void adminPanel() {
    clearScreen();
    printHeader("ADMIN PANEL");
    
    string password;
    cout << "Enter admin password: ";
    cin >> password;
    
    if (password != ADMIN_PASSWORD) {
        printError("Invalid admin password!");
        pauseScreen(2000);
        return;
    }
    
    while (true) {
        clearScreen();
        printHeader("ADMIN PANEL");
        
        cout << "1. View All Accounts\n";
        cout << "2. View Account Details\n";
        cout << "3. Back to Main Menu\n";
        cout << "\nEnter your choice: ";
        
        int choice;
        cin >> choice;
        
        if (choice == 3) break;
        
        switch (choice) {
            case 1:
                // Implementation for viewing all accounts
                printInfo("Feature under development");
                pauseScreen(2000);
                break;
            case 2:
                // Implementation for viewing account details
                printInfo("Feature under development");
                pauseScreen(2000);
                break;
            default:
                printError("Invalid choice!");
                pauseScreen(2000);
        }
    }
}

// ===============================
// Menu Functions
// ===============================
void printWelcomeScreen() {
    clearScreen();
    cout << "\x1b[33m";
    cout << "\n\n";
    cout << "\t\t" << string(60, '*') << endl;
    cout << "\t\t*" << setw(58) << "*" << endl;
    cout << "\t\t*" << setw(25) << "WELCOME TO" << setw(33) << "*" << endl;
    cout << "\t\t*" << setw(28) << "ADVANCED ATM SYSTEM" << setw(30) << "*" << endl;
    cout << "\t\t*" << setw(58) << "*" << endl;
    cout << "\t\t" << string(60, '*') << endl;
    cout << "\x1b[37m";
    cout << "\n\n";
}

void printMainMenu() {
    printWelcomeScreen();
    cout << "\x1b[32m" << "MAIN MENU\n";
    cout << string(40, '-') << endl;
    cout << "1. Create Account\n";
    cout << "2. Login\n";
    cout << "3. Admin Panel\n";
    cout << "4. Exit\n";
    cout << string(40, '-') << endl;
    cout << "\x1b[37m";
    cout << "Enter your choice: ";
}

void printUserMenu(const string& accountHolderName) {
    clearScreen();
    printWelcomeScreen();
    cout << "\x1b[36m" << "Welcome, " << accountHolderName << "!\n";
    cout << string(40, '-') << endl;
    cout << "1. Check Balance\n";
    cout << "2. Deposit Money\n";
    cout << "3. Withdraw Money\n";
    cout << "4. Transaction History\n";
    cout << "5. Change Password\n";
    cout << "6. Logout\n";
    cout << string(40, '-') << endl;
    cout << "\x1b[37m";
    cout << "Enter your choice: ";
}

// ===============================
// Main Function
// ===============================
int main() {
    int choice;
    string accountNumber;
    
    while (true) {
        printMainMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                createAccount();
                break;
                
            case 2:
                if (login(accountNumber)) {
                    Account account = loadAccount(accountNumber);
                    
                    while (true) {
                        printUserMenu(account.accountHolderName);
                        cin >> choice;
                        
                        if (choice == 1) checkBalance(accountNumber);
                        else if (choice == 2) depositMoney(accountNumber);
                        else if (choice == 3) withdrawMoney(accountNumber);
                        else if (choice == 4) viewTransactionHistory(accountNumber);
                        else if (choice == 5) changePassword(accountNumber);
                        else if (choice == 6) break;
                        else {
                            printError("Invalid choice!");
                            pauseScreen(2000);
                        }
                    }
                }
                break;
                
            case 3:
                adminPanel();
                break;
                
            case 4:
                clearScreen();
                cout << "\x1b[33m";
                cout << "\n\n";
                cout << "\t\t" << string(60, '=') << endl;
                cout << "\t\t" << "Thank you for using our ATM System!" << endl;
                cout << "\t\t" << "Your trust means everything to us." << endl;
                cout << "\t\t" << string(60, '=') << endl;
                cout << "\x1b[37m";
                cout << "\n\n";
                return 0;
                
            default:
                printError("Invalid choice!");
                pauseScreen(2000);
        }
    }
    
    return 0;
}