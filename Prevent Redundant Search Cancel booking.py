def is_duplicate(bookings, new_booking):
    for booking in bookings:
        if (
            booking['phone'] == new_booking['phone'] and
            booking['date'] == new_booking['date'] and
            booking['time'] == new_booking['time']
        ):
            return True
    return False

def search_booking(bookings, booking_id):
    for booking in bookings:
        if booking['id'] == booking_id:
            return booking
    return None

def cancel_booking(bookings, booking_id):
    for i, booking in enumerate(bookings):
        if booking['id'] == booking_id:
            del bookings[i]
            return True
    return False

