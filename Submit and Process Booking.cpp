// Rembayung Booking System
// Submit Booking (with duplicate check), Process Booking (FCFS),
// Search Booking, Cancel Booking
#include <iostream>
#include <deque>
#include <unordered_set>
#include <vector>
#include <string>
#include <limits>
using namespace std;

const int MAX_SEATS = 200;

struct Booking {
    int bookingId;       // system-generated, used for cancellation
    string customerId;   // customer's own ID
    string name;
    string phone;
    string date;
    string time;
    int seats;
};

deque<Booking> waitingQueue;            // FCFS order (deque so we can search/cancel)
unordered_set<string> activeCustomers;  // customer IDs with a waiting OR confirmed booking
vector<Booking> confirmed;              // confirmed reservations
int confirmedSeats = 0;                 // running total, never > MAX_SEATS
int nextBookingId = 1;

// ---------- helpers (work on both vector and deque) ----------
template <typename Container>
bool is_duplicate(const Container& bookings, const Booking& nb) {
    for (const Booking& b : bookings)
        if (b.phone == nb.phone && b.date == nb.date && b.time == nb.time)
            return true;
    return false;
}

template <typename Container>
int search_booking(const Container& bookings, const string& phone,
                   const string& date, const string& time) {
    for (int i = 0; i < (int)bookings.size(); i++)
        if (bookings[i].phone == phone && bookings[i].date == date &&
            bookings[i].time == time)
            return i;
    return -1;
}

template <typename Container>
bool cancel_booking(Container& bookings, int bookingId, Booking& removed) {
    for (int i = 0; i < (int)bookings.size(); i++) {
        if (bookings[i].bookingId == bookingId) {
            removed = bookings[i];
            bookings.erase(bookings.begin() + i);
            return true;
        }
    }
    return false;
}

void printBooking(const Booking& b, const string& status) {
    cout << "  [" << status << "] Booking #" << b.bookingId
         << " | " << b.name << " (" << b.customerId << ")"
         << " | " << b.phone << " | " << b.date << " " << b.time
         << " | " << b.seats << " seats\n";
}

// ---------- 1. Submit Booking Request ----------
void submitBooking() {
    Booking b;
    cout << "Customer ID: ";
    cin >> b.customerId;
    cout << "Customer name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, b.name);
    cout << "Phone: ";
    cin >> b.phone;
    cout << "Date (DD/MM/YYYY): ";
    cin >> b.date;
    cout << "Time (HH:MM): ";
    cin >> b.time;
    cout << "Seats requested: ";
    if (!(cin >> b.seats)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number.\n";
        return;
    }
    if (b.seats < 1 || b.seats > MAX_SEATS) {
        cout << "Rejected: seats must be 1-" << MAX_SEATS << ".\n";
        return;
    }

    // 3. Prevent redundant booking
    if (activeCustomers.count(b.customerId)) {
        cout << "Rejected: customer " << b.customerId
             << " already has a waiting or confirmed booking.\n";
        return;
    }
    if (is_duplicate(waitingQueue, b) || is_duplicate(confirmed, b)) {
        cout << "Rejected: duplicate booking (same phone, date and time).\n";
        return;
    }

    b.bookingId = nextBookingId++;
    waitingQueue.push_back(b);            // back of the queue = FCFS
    activeCustomers.insert(b.customerId);
    cout << "Request received. Booking ID: " << b.bookingId
         << ". Position in queue: " << waitingQueue.size() << "\n";
}

// ---------- 2. Process Next Booking (FCFS) ----------
void processNext() {
    if (waitingQueue.empty()) {
        cout << "No waiting requests.\n";
        return;
    }
    Booking b = waitingQueue.front();
    waitingQueue.pop_front();             // always removed (strict FCFS)

    if (confirmedSeats + b.seats <= MAX_SEATS) {
        confirmedSeats += b.seats;
        confirmed.push_back(b);           // ID stays in activeCustomers
        cout << "CONFIRMED: " << b.name << " (" << b.customerId << "), "
             << b.seats << " seats.\n";
    } else {
        activeCustomers.erase(b.customerId);   // may retry later
        cout << "UNSUCCESSFUL: " << b.name << " (" << b.customerId << ") needs "
             << b.seats << " seats, only " << MAX_SEATS - confirmedSeats
             << " left.\n";
    }
}

// ---------- 4. Search Booking ----------
void searchBooking() {
    string phone, date, time;
    cout << "Phone: ";
    cin >> phone;
    cout << "Date (DD/MM/YYYY): ";
    cin >> date;
    cout << "Time (HH:MM): ";
    cin >> time;

    int i = search_booking(confirmed, phone, date, time);
    if (i != -1) { printBooking(confirmed[i], "CONFIRMED"); return; }
    i = search_booking(waitingQueue, phone, date, time);
    if (i != -1) { printBooking(waitingQueue[i], "WAITING"); return; }
    cout << "Booking not found.\n";
}

// ---------- 5. Cancel Booking ----------
void cancelBooking() {
    int id;
    cout << "Booking ID to cancel: ";
    if (!(cin >> id)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number.\n";
        return;
    }
    Booking removed;
    if (cancel_booking(confirmed, id, removed)) {
        confirmedSeats -= removed.seats;          // free the seats
        activeCustomers.erase(removed.customerId);
        cout << "Cancelled confirmed booking #" << id << ". Seats freed: "
             << removed.seats << "\n";
    } else if (cancel_booking(waitingQueue, id, removed)) {
        activeCustomers.erase(removed.customerId);
        cout << "Cancelled waiting booking #" << id << ".\n";
    } else {
        cout << "Booking not found.\n";
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
             << "3. Search Booking\n"
             << "4. Cancel Booking\n"
             << "0. Exit\n"
             << "Choice: ";
        if (!(cin >> choice)) break;
        switch (choice) {
            case 1: submitBooking(); break;
            case 2: processNext();   break;
            case 3: searchBooking(); break;
            case 4: cancelBooking(); break;
            case 0: cout << "Goodbye.\n"; break;
            default: cout << "Invalid choice.\n";
        }
        if (choice >= 1 && choice <= 4) showStatus();
    } while (choice != 0);
    return 0;
}
