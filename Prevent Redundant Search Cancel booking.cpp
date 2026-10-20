#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Booking {
    int id;
    string phone;
    string date;
    string time;
};


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

int search_booking(const vector<Booking>& bookings, const string& phone, const string& date, const string& time) {
    for (int i = 0; i < bookings.size(); i++){
        if (bookings[i].id == booking_id) {
            return i;
        }
    }
    return -1; // Booking not found
}

bool cancel_booking(vector<Booking>& bookings, int booking_id) {
    for (int i = 0; i < bookings.size(); i++) {
        if (bookings[i].id == booking_id) {
            bookings.erase(bookings.begin() + i);
            return true; // Booking canceled
        }
    }
    return false; // Booking not found
}