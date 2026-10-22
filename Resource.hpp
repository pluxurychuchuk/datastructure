// ============================================================================
// Resource.hpp
// Task 3 - Collection Counter and Delivery Resource Management Module
// Data types used by the resource manager (and by modules that call it).
// ============================================================================
#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <string>
#include "SharedTypes.hpp"

enum class ResourceType   { COUNTER, DELIVERY };       // C## or D##
enum class ResourceStatus { AVAILABLE, OCCUPIED, UNAVAILABLE };

// Result of an assignment attempt.
enum class AssignOutcome {
    ASSIGNED,   // a resource was given to the request immediately
    QUEUED,     // all suitable resources busy -> request waits (stays PENDING)
    REJECTED    // invalid input / duplicate / waiting line full
};

// One collection counter (C01) or one delivery staff member (D01).
struct Resource {
    std::string    id;
    std::string    name;
    ResourceType   type   = ResourceType::COUNTER;
    ResourceStatus status = ResourceStatus::AVAILABLE;
    std::string    assignedRequestId;   // e.g. R001 ("" if none)
    std::string    assignedParcelId;    // e.g. P001 ("" if none)
    int            jobsCompleted = 0;   // workload counter
};

// A service request (from Task 2) that needs a resource.
struct AssignmentJob {
    std::string  requestId;   // R001
    std::string  parcelId;    // P001
    ResourceType neededType = ResourceType::COUNTER;

    // Two jobs are "the same" if they have the same Request ID.
    bool operator==(const AssignmentJob& other) const { return requestId == other.requestId; }
};

// A status change produced by Task 3 that Task 1 (parcel), Task 2 (request)
// and Task 4 (activity history) must apply. See ResourceManager::pollUpdate().
struct StatusUpdate {
    std::string   requestId;
    std::string   parcelId;
    std::string   resourceId;                       // "" if no resource involved
    RequestStatus requestStatus = RequestStatus::PENDING;
    bool          parcelChanged = false;            // false -> leave parcel status as it is
    ParcelStatus  parcelStatus  = ParcelStatus::REGISTERED;
};

inline std::string typeToString(ResourceType t) {
    return t == ResourceType::COUNTER ? "Counter" : "Delivery";
}

inline std::string statusToString(ResourceStatus s) {
    switch (s) {
        case ResourceStatus::AVAILABLE:   return "Available";
        case ResourceStatus::OCCUPIED:    return "Occupied";
        case ResourceStatus::UNAVAILABLE: return "Unavailable";
    }
    return "Unknown";
}

#endif
