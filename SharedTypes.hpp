// ============================================================================
// SharedTypes.hpp
// Shared data conventions for the whole Smart Campus Parcel system.
// (Keep ONE copy of this file in the integrated project.)
//
// IDs:   P001 = parcel, R001 = service request, C01 = counter, D01 = delivery
// ============================================================================
#ifndef SHARED_TYPES_HPP
#define SHARED_TYPES_HPP

#include <string>
#include <cctype>

enum class ParcelStatus {
    REGISTERED, READY_FOR_COLLECTION, OUT_FOR_DELIVERY, SPECIAL_HANDLING, ON_HOLD,
    MISSING, COLLECTED, DELIVERED, RETURNED, CANCELLED
};

enum class RequestStatus { PENDING, ASSIGNED, COMPLETED, CANCELLED };

inline std::string parcelStatusToString(ParcelStatus s) {
    switch (s) {
        case ParcelStatus::REGISTERED:           return "REGISTERED";
        case ParcelStatus::READY_FOR_COLLECTION: return "READY_FOR_COLLECTION";
        case ParcelStatus::OUT_FOR_DELIVERY:     return "OUT_FOR_DELIVERY";
        case ParcelStatus::SPECIAL_HANDLING:     return "SPECIAL_HANDLING";
        case ParcelStatus::ON_HOLD:              return "ON_HOLD";
        case ParcelStatus::MISSING:              return "MISSING";
        case ParcelStatus::COLLECTED:            return "COLLECTED";
        case ParcelStatus::DELIVERED:            return "DELIVERED";
        case ParcelStatus::RETURNED:             return "RETURNED";
        case ParcelStatus::CANCELLED:            return "CANCELLED";
    }
    return "UNKNOWN";
}

inline std::string requestStatusToString(RequestStatus s) {
    switch (s) {
        case RequestStatus::PENDING:   return "PENDING";
        case RequestStatus::ASSIGNED:  return "ASSIGNED";
        case RequestStatus::COMPLETED: return "COMPLETED";
        case RequestStatus::CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}

// True if id = prefix letter followed by exactly `digits` digits (e.g. P001).
inline bool hasIdFormat(const std::string& id, char prefix, int digits) {
    if ((int)id.length() != digits + 1 || id[0] != prefix) return false;
    for (size_t i = 1; i < id.length(); i++)
        if (!isdigit(static_cast<unsigned char>(id[i]))) return false;
    return true;
}

inline bool isValidParcelId(const std::string& id)   { return hasIdFormat(id, 'P', 3); }
inline bool isValidRequestId(const std::string& id)  { return hasIdFormat(id, 'R', 3); }
inline bool isValidCounterId(const std::string& id)  { return hasIdFormat(id, 'C', 2); }
inline bool isValidDeliveryId(const std::string& id) { return hasIdFormat(id, 'D', 2); }

#endif
