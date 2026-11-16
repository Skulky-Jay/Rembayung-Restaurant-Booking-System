// Rembayung Booking System - Steps 1-3 + Anti-Scalper
// Submit Booking Request, Process Booking (FCFS), Prevent Redundant Booking,
// Anti-Scalper check (esther.cpp)
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>
#include <deque>
#include <chrono>
using namespace std;

const int MAX_SEATS = 200;

// Anti-Scalper settings
const int MAX_ATTEMPTS = 2;
const int TIME_WINDOW_SECONDS = 120;

struct Booking {
    int bookingId;       // unique per booking, auto-assigned
    string customerId;   // identifies the customer
    string name;
    string phone;
    string date;
    string time;
    int seats;
};

queue<Booking> waitingQueue;           // FCFS: arrival order preserved
unordered_set<string> activeCustomers; // IDs with a waiting OR confirmed booking
vector<Booking> confirmed;             // confirmed reservations
int confirmedSeats = 0;                // running total, never > MAX_SEATS
int nextBookingId = 1;                 // counter for booking IDs

// Anti-Scalper: booking attempt timestamps per Customer ID (sliding window)
unordered_map<string, deque<chrono::steady_clock::time_point>> attemptHistory;

// Anti-Scalper function: returns true if the booking attempt may proceed
bool antiScalperCheck(const string& customerId) {
    auto currentTime = chrono::steady_clock::now();

    // Get this customer's booking attempt history
    auto& attempts = attemptHistory[customerId];

    // Remove attempts older than the time window
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
    cout << "Booking attempts in the last " << TIME_WINDOW_SECONDS << " seconds: " << attempts.size() << endl;

    // Check whether the customer exceeds the attempt limit
    if (attempts.size() > MAX_ATTEMPTS) {
        cout << "\nWARNING: Suspicious booking activity detected!\n";
        cout << "More than " << MAX_ATTEMPTS << " booking attempts within " << TIME_WINDOW_SECONDS << " seconds.\n";

        char verification;

        cout << "Manual verification required.\n";
        cout << "Has the customer passed verification? (Y/N): ";
        cin >> verification;

        if (verification == 'Y' || verification == 'y') {

            cout << "Verification successful. Booking may proceed.\n";
            attempts.clear();   // reset history after successful verification

            return true;
        } else {

            cout << "Verification failed. Booking request rejected.\n";
            return false;

        }
    }

    cout << "No suspicious activity detected.\n";
    return true;
}

// Forward declarations (defined below)
bool is_duplicate(const vector<Booking>& bookings, const Booking& new_booking);
int search_booking(const vector<Booking>& bookings, const string& phone, const string& date, const string& time);
bool cancel_booking(vector<Booking>& bookings, int booking_id);

// 1. Submit Booking Request - O(1) average
void submitBooking() {
    Booking booking;
    cout << "Customer ID: ";
    cin >> booking.customerId;
    // Anti-Scalper check: every attempt counts, even ones rejected later
    if (!antiScalperCheck(booking.customerId)) return;
    cout << "Customer name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, booking.name);
    cout << "Phone: ";
    getline(cin, booking.phone);
    cout << "Date (DD/MM/YYYY): ";
    getline(cin, booking.date);
    cout << "Time (HH:MM): ";
    getline(cin, booking.time);
    cout << "Seats requested: ";
    if (!(cin >> booking.seats)) {                       // non-numeric input
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number.\n";
        return;
    }
    if (booking.seats < 1 || booking.seats > MAX_SEATS) {
        cout << "Rejected: seats must be 1-" << MAX_SEATS << ".\n";
        return;
    }
    // Same phone/date/time as an existing confirmed booking
    if (is_duplicate(confirmed, booking)) {
        cout << "Rejected: a booking with this phone, date and time already exists.\n";
        return;
    }
    // 3. Duplicate check: insert().second is false if ID already exists
    if (!activeCustomers.insert(booking.customerId).second) {
        cout << "Rejected: customer " << booking.customerId
             << " already has an active request or reservation.\n";
        return;
    }
    booking.bookingId = nextBookingId++;                 // assigned only once accepted
    waitingQueue.push(booking);                          // goes to the back = FCFS
    cout << "Request received. Booking ID: " << booking.bookingId
         << ". Position in queue: " << waitingQueue.size() << "\n";
}

// 2. Process Next Booking (FCFS) - O(1)
void processNext() {
    if (waitingQueue.empty()) {
        cout << "No waiting requests.\n";
        return;
    }
    Booking booking = waitingQueue.front();              // earliest valid request
    waitingQueue.pop();                            // always removed (strict FCFS)

    if (confirmedSeats + booking.seats <= MAX_SEATS) {
        confirmedSeats += booking.seats;
        confirmed.push_back(booking);                    // ID stays in activeCustomers
        cout << "CONFIRMED: " << booking.name << " (" << booking.customerId
             << ", booking #" << booking.bookingId << "), "
             << booking.seats << " seats.\n";
    } else {
        activeCustomers.erase(booking.customerId);       // no longer active, may retry
        cout << "UNSUCCESSFUL: " << booking.name << " (" << booking.customerId
             << ", booking #" << booking.bookingId << ") needs "
             << booking.seats << " seats, only " << MAX_SEATS - confirmedSeats
             << " left.\n";
    }
}

bool is_duplicate(const vector<Booking>& bookings, const Booking& new_booking) {
    for (Booking booking : bookings) {
        if (booking.phone == new_booking.phone && 
            booking.date == new_booking.date && 
            booking.time == new_booking.time) {
            return true; // Duplicate booking found
        }
    }
    return false; // No duplicate booking found
}

int search_booking(const vector<Booking>& bookings, const string& phone, const int bookingId) {
    for (int i = 0; i < bookings.size(); i++){
        if (bookings[i].phone == phone &&
            bookings[i].bookingId == bookingId) {
            return i;
        }
    }
    return -1; // Booking not found
}

bool cancel_booking(vector<Booking>& bookings, int booking_id) {
    for (int i = 0; i < bookings.size(); i++) {
        if (bookings[i].bookingId == booking_id) {
            confirmedSeats -= bookings[i].seats;
            activeCustomers.erase(bookings[i].customerId);
            bookings.erase(bookings.begin() + i);
            return true; // Booking canceled
        }
    }
    return false; // Booking not found
}

// 4. Display waiting booking requests (copy of queue, so the original is untouched)
void viewBookingRequests(queue<Booking> bookingQueue) {
    cout << "\n===== WAITING BOOKING REQUESTS =====\n";

    if (bookingQueue.empty()) {
        cout << "No booking requests available.\n";
        return;
    }

    while (!bookingQueue.empty()) {
        const Booking& currentBooking = bookingQueue.front();

        cout << "\nBooking ID: " << currentBooking.bookingId;
        cout << "\nCustomer ID: " << currentBooking.customerId;
        cout << "\nCustomer Name: " << currentBooking.name;
        cout << "\nSeats Requested: " << currentBooking.seats;
        cout << "\n----------------------------\n";

        bookingQueue.pop();
    }
}

// 5. Display confirmed reservations
void viewConfirmedReservations(const vector<Booking>& confirmedBookings) {
    cout << "\n===== CONFIRMED RESERVATIONS =====\n";

    if (confirmedBookings.empty()) {
        cout << "No confirmed reservations available.\n";
        return;
    }

    for (const Booking& currentBooking : confirmedBookings) {
        cout << "\nBooking ID: " << currentBooking.bookingId;
        cout << "\nCustomer ID: " << currentBooking.customerId;
        cout << "\nCustomer Name: " << currentBooking.name;
        cout << "\nSeats Reserved: " << currentBooking.seats;
        cout << "\n----------------------------\n";
    }

    cout << "\nTotal Confirmed Reservations: " << confirmedBookings.size() << endl;
}

// 7. Display remaining seat capacity
void viewAvailableSeats() {
    cout << "\n===== AVAILABLE SEATS =====\n"
         << "Seats used: " << confirmedSeats << "/" << MAX_SEATS << "\n"
         << "Seats available: " << MAX_SEATS - confirmedSeats << "\n";
}

int main() {
    int choice;

    do {
        cout << "\n===== REMBAYUNG RESTAURANT =====\n"
             << "1. Submit Booking Request\n"
             << "2. Process Next Booking\n"
             << "3. Search Booking\n"
             << "4. Cancel Booking\n"
             << "5. View Booking Requests\n"
             << "6. View Confirmed Reservations\n"
             << "7. View Available Seats\n"
             << "0. Exit\n"
             << "Choice: ";
        if (!(cin >> choice)) break;
        switch (choice) {
            case 1: submitBooking(); break;
            case 2: processNext();   break;
            case 3: {
                string phone;
                int bookingId;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin >> bookingId;
                cout << "Booking ID to search: " << bookingId << endl;
                cout << "Phone: ";
                getline(cin, phone);
                int idx = search_booking(confirmed, phone, bookingId);
                if (idx == -1) {
                    cout << "Booking not found.\n";
                } else {
                    const Booking& b = confirmed[idx];
                    cout << "Found: booking #" << b.bookingId << ", " << b.name
                         << " (" << b.customerId << "), " << b.seats << " seats.\n";
                }
                break;
            }
            case 4: {
                int id;
                cout << "Booking ID to cancel: ";
                if (!(cin >> id)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid number.\n";
                    break;
                }
                if (cancel_booking(confirmed, id)) cout << "Booking #" << id << " cancelled.\n";
                else cout << "Booking not found.\n";
                break;
            }
            case 5: viewBookingRequests(waitingQueue); break;
            case 6: viewConfirmedReservations(confirmed); break;
            case 7: viewAvailableSeats(); break;
            case 0: cout << "Goodbye.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
    return 0;
}
