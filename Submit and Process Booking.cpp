// Rembayung Booking System - Steps 1-3
// Submit Booking Request, Process Booking (FCFS), Prevent Redundant Booking
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>
#include <limits>
using namespace std;

const int MAX_SEATS = 200;

struct Booking {
    string id;
    string name;
    int seats;
};

queue<Booking> waitingQueue;           // FCFS: arrival order preserved
unordered_set<string> activeCustomers; // IDs with a waiting OR confirmed booking
vector<Booking> confirmed;             // confirmed reservations
int confirmedSeats = 0;                // running total, never > MAX_SEATS

// 1. Submit Booking Request - O(1) average
void submitBooking() {
    Booking b;
    cout << "Customer ID: ";
    cin >> b.id;
    cout << "Customer name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, b.name);
    cout << "Seats requested: ";
    if (!(cin >> b.seats)) {                       // non-numeric input
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number.\n";
        return;
    }
    if (b.seats < 1 || b.seats > MAX_SEATS) {
        cout << "Rejected: seats must be 1-" << MAX_SEATS << ".\n";
        return;
    }

    waitingQueue.push(b);                          // goes to the back = FCFS
    cout << "Request received. Position in queue: " << waitingQueue.size() << "\n";
}

// 2. Process Next Booking (FCFS) - O(1)
void processNext() {
    if (waitingQueue.empty()) {
        cout << "No waiting requests.\n";
        return;
    }
    Booking b = waitingQueue.front();              // earliest valid request
    waitingQueue.pop();                            // always removed (strict FCFS)

    if (confirmedSeats + b.seats <= MAX_SEATS) {
        confirmedSeats += b.seats;
        confirmed.push_back(b);                    // ID stays in activeCustomers
        cout << "CONFIRMED: " << b.name << " (" << b.id << "), "
             << b.seats << " seats.\n";
    } else {
        activeCustomers.erase(b.id);               // no longer active, may retry
        cout << "UNSUCCESSFUL: " << b.name << " (" << b.id << ") needs "
             << b.seats << " seats, only " << MAX_SEATS - confirmedSeats
             << " left.\n";
    }
}

void showStatus() {
    cout << "Waiting: " << waitingQueue.size()
         << " | Confirmed bookings: " << confirmed.size()
         << " | Seats used: " << confirmedSeats << "/" << MAX_SEATS << "\n";
}

int main() {
    int choice;
    do {
        cout << "\n===== REMBAYUNG RESTAURANT =====\n"
             << "1. Submit Booking Request\n"
             << "2. Process Next Booking\n"
             << "0. Exit\n"
             << "Choice: ";
        if (!(cin >> choice)) break;
        switch (choice) {
            case 1: submitBooking(); break;
            case 2: processNext();   break;
            case 0: cout << "Goodbye.\n"; break;
            default: cout << "Invalid choice.\n";
        }
        if (choice == 1 || choice == 2) showStatus();
    } while (choice != 0);
    return 0;
}
