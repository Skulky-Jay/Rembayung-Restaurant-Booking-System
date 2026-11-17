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

// Forward declarations
bool isDuplicateInQueue(queue<Booking> temp, const Booking& newBooking);
bool is_duplicate(const vector<Booking>& bookings, const Booking& new_booking);
int search_booking(const vector<Booking>& bookings, int bookingId);
bool cancel_booking(vector<Booking>& bookings, int booking_id);

// Returns true if every character in the text is a digit (0-9)
bool onlyDigits(const string& text) {
    for (int i = 0; i < text.size(); i++) {
        if (text[i] < '0' || text[i] > '9') {
            return false;
        }
    }
    return true;
}

// Date validation DD/MM/YYYY
bool isValidDate(const string& date) {
    // Step 1: the text must be exactly 10 characters long
    if (date.size() != 10) {
        return false;
    }

    // Step 2: the 3rd and 6th characters must be '/'
    if (date[2] != '/' || date[5] != '/') {
        return false;
    }

    // Step 3: the day, month and year parts must be numbers
    string dayText = date.substr(0, 2);     // first 2 characters
    string monthText = date.substr(3, 2);   // characters 4-5
    string yearText = date.substr(6, 4);    // last 4 characters
    if (!onlyDigits(dayText) || !onlyDigits(monthText) || !onlyDigits(yearText)) {
        return false;
    }

    // Step 4: the numbers must be in a sensible range
    int day = stoi(dayText);
    int month = stoi(monthText);
    int year = stoi(yearText);
    if (day < 1 || day > 31) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    if (year < 2000) {
        return false;
    }

    return true;   // passed every check
}

// Time validation HH:MM 
bool isValidTime(const string& time) {
    // Step 1: the text must be exactly 5 characters long
    if (time.size() != 5) {
        return false;
    }

    // Step 2: the 3rd character must be ':'
    if (time[2] != ':') {
        return false;
    }

    // Step 3: the hour and minute parts must be numbers
    string hourText = time.substr(0, 2);     // first 2 characters
    string minuteText = time.substr(3, 2);   // last 2 characters
    if (!onlyDigits(hourText) || !onlyDigits(minuteText)) {
        return false;
    }

    // Step 4: the numbers must be in a sensible range
    int hour = stoi(hourText);
    int minute = stoi(minuteText);
    if (hour < 0 || hour > 23) {
        return false;
    }
    if (minute < 0 || minute > 59) {
        return false;
    }

    return true;   // passed every check
}

// 1. Submit Booking Request - O(1) average
void submitBooking() {
    Booking booking;
    cout << "Customer ID: ";
    cin >> booking.customerId;

    // Check whether the customer already has an active booking
    if (activeCustomers.count(booking.customerId)) {
        cout << "You already have an active booking!" << endl;
        return;
    }
    if (!antiScalperCheck(booking.customerId)) {
        return;
    }
    cout << "Customer name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, booking.name);
    cout << "Phone: ";
    getline(cin, booking.phone);
    cout << "Date (DD/MM/YYYY): ";
    getline(cin, booking.date);
    if (!isValidDate(booking.date)) {
        cout << "Rejected: invalid date, use DD/MM/YYYY.\n";
        return;
    }
    cout << "Time (HH:MM): ";
    getline(cin, booking.time);
    if (!isValidTime(booking.time)) {
        cout << "Rejected: invalid time, use 24-hour HH:MM.\n";
        return;
    }
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
    // Check duplicate booking in Confirmed and Waiting Queue
    if (is_duplicate(confirmed, booking) || isDuplicateInQueue(waitingQueue, booking)) {
        cout << "Rejected: duplicate booking detected!\n";
        return;
    }

    booking.bookingId = nextBookingId++;                 // assigned only once accepted
    activeCustomers.insert(booking.customerId);
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

// Check duplicate booking in Waiting Queue
bool isDuplicateInQueue(queue<Booking> temp, const Booking& newBooking) {
    while (!temp.empty())
    {
        Booking current = temp.front();
        temp.pop();

        if (current.phone == newBooking.phone &&
            current.date == newBooking.date &&
            current.time == newBooking.time)
        {
            return true;
        }
    }

    return false;
}

int search_booking(const vector<Booking>& bookings, int bookingId) {
    for (int i = 0; i < bookings.size(); i++){
        if (bookings[i].bookingId == bookingId) {
            return i;
        }
    }
    return -1; // Booking not found
}

bool cancel_booking(vector<Booking>& bookings, int bookingId) {
    for (int i = 0; i < bookings.size(); i++) {
        if (bookings[i].bookingId == bookingId) {
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
        cout << "\nPhone: " << currentBooking.phone;
        cout << "\nDate: " << currentBooking.date;
        cout << "\nTime: " << currentBooking.time;
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

//Test Data
void pushTestBookings() {
    Booking tests[] = {
        {0, "C001", "Ahmad Faiz",    "012-3456789", "20/12/2026", "19:00", 4},
        {0, "C002", "Siti Aminah",   "013-9876543", "21/12/2026", "20:30", 10},
        {0, "C003", "Daniel Tan",    "011-2223334", "25/12/2026", "18:00", 2},
    };

    for (Booking& booking : tests) {
        booking.bookingId = nextBookingId++;
        confirmed.push_back(booking);
        confirmedSeats += booking.seats;
        activeCustomers.insert(booking.customerId);
    }

}

int main() {
    int choice;
    pushTestBookings(); //call test data, comment this to not allow test data and be fresh
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
                int bookingId;
                cout << "Booking ID to search: ";
                if (!(cin >> bookingId)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid number.\n";
                    break;
                }
                int idx = search_booking(confirmed, bookingId);
                if (idx == -1) {
                    cout << "Booking not found.\n";
                } else {
                    const Booking& booking = confirmed[idx];
                    cout << "Found booking #" << booking.bookingId << "\n"
                         << "  Customer ID: " << booking.customerId << "\n"
                         << "  Name: " << booking.name << "\n"
                         << "  Phone: " << booking.phone << "\n"
                         << "  Date: " << booking.date << "\n"
                         << "  Time: " << booking.time << "\n"
                         << "  Seats: " << booking.seats << "\n";
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
