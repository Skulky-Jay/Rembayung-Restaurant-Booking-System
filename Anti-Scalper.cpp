
#include <iostream>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <string>
using namespace std;

// Anti-Scalper settings
const int MAX_ATTEMPTS = 5;
const int TIME_WINDOW_SECONDS = 60;

// Store booking attempt timestamps for each Customer ID
unordered_map<string, deque<chrono::steady_clock::time_point>> attemptHistory;

// Anti-Scalper function
bool antiScalperCheck(const string& customerId) {

    auto currentTime = chrono::steady_clock::now();

    // Get this customer's booking attempt history
    auto& attempts = attemptHistory[customerId];

    // Remove attempts older than 60 seconds
    while (!attempts.empty()) {
        auto elapsed = chrono::duration_cast<chrono::seconds>(
            currentTime - attempts.front()
        ).count();

        if (elapsed > TIME_WINDOW_SECONDS) {
            attempts.pop_front();
        } else {
            break;
        }
    }

    // Record the current booking attempt
    attempts.push_back(currentTime);

    cout << "\n--- Anti-Scalper Security Check ---\n";
    cout << "Customer ID: " << customerId << endl;
    cout << "Booking attempts in the last 60 seconds: "
         << attempts.size() << endl;

    // Check whether the customer exceeds the attempt limit
    if (attempts.size() > MAX_ATTEMPTS) {

        cout << "\nWARNING: Suspicious booking activity detected!\n";
        cout << "More than 5 booking attempts within 60 seconds.\n";

        char verification;

        cout << "Manual verification required.\n";
        cout << "Has the customer passed verification? (Y/N): ";
        cin >> verification;

        if (verification == 'Y' || verification == 'y') {

            cout << "Verification successful. Booking may proceed.\n";

            // Reset attempt history after successful verification
            attempts.clear();

            return true;
        } 
        else {

            cout << "Verification failed. Booking request rejected.\n";
            return false;
        }
    }

    cout << "No suspicious activity detected.\n";
    return true;
}