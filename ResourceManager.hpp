// ============================================================================
// ResourceManager.hpp
// Task 3 - Collection Counter and Delivery Resource Management Module
//
// DATA STRUCTURES
//   resources[]        - fixed array: registry of every counter / courier
//   availableCounters  - circular queue of indexes of AVAILABLE counters
//   availableDelivery  - circular queue of indexes of AVAILABLE couriers
//   waitingCounter     - circular queue of requests waiting for a counter
//   waitingDelivery    - circular queue of requests waiting for a courier
//   updates            - circular queue (buffer) of status changes for other modules
//
// ALLOCATION RULE (round-robin)
//   1. A request needs a COUNTER (recipient collection) or DELIVERY (campus delivery).
//   2. The resource at the FRONT of that type's available queue gets it.
//   3. When a resource finishes, it re-joins at the REAR of the queue.
//      -> work is rotated fairly; no counter/courier is overloaded.
//   4. If none is free, the request waits (FIFO) and is served automatically
//      as soon as a resource of that type becomes free.
//
// STATUS RULES (shared data)
//   assigned to counter   -> Request ASSIGNED,  Parcel READY_FOR_COLLECTION
//   assigned to delivery  -> Request ASSIGNED,  Parcel OUT_FOR_DELIVERY
//   waiting for resource  -> Request PENDING
//   completed at counter  -> Request COMPLETED, Parcel COLLECTED
//   completed by delivery -> Request COMPLETED, Parcel DELIVERED
//   cancelled             -> Request CANCELLED (parcel decided by Task 1/2)
// ============================================================================
#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <string>
#include "Resource.hpp"
#include "CircularQueue.hpp"

class ResourceManager {
public:
    static const int MAX_RESOURCES = 20;
    static const int MAX_WAITING   = 50;
    static const int MAX_UPDATES   = 100;

    ResourceManager();

    // ---- 1. Register and maintain resources (type comes from the ID prefix) ----
    bool registerResource(const std::string& id, const std::string& name);
    int  loadFromFile(const std::string& filename);   // returns number loaded

    // ---- 2. Display availability ----
    void displayAllResources() const;
    void displayAvailability() const;

    // ---- 3. Assign a service request ----
    AssignOutcome assignNext(const std::string& requestId, const std::string& parcelId,
                             ResourceType type);                       // round-robin
    bool assignToResource(const std::string& resourceId, const std::string& requestId,
                          const std::string& parcelId);                // manual choice

    // ---- 4. Update resource status ----
    bool releaseResource(const std::string& resourceId);    // job done -> AVAILABLE
    bool cancelAssignment(const std::string& requestId);    // request cancelled
    bool markUnavailable(const std::string& resourceId);
    bool markAvailable(const std::string& resourceId);

    // ---- Lookup ----
    const Resource* findResource(const std::string& resourceId) const;
    std::string     findResourceOfRequest(const std::string& requestId) const;  // "" if none
    bool            isRequestWaiting(const std::string& requestId) const;
    int             getResourceCount() const { return resourceCount; }

    // ---- Integration: take the next status change (oldest first) ----
    // Usage in the integrated main, after any Task 3 call:
    //   StatusUpdate u;
    //   while (resourceManager.pollUpdate(u)) { update request, parcel, history }
    bool pollUpdate(StatusUpdate& out) { return updates.dequeue(out); }

private:
    Resource resources[MAX_RESOURCES];
    int      resourceCount;

    CircularQueue<int>           availableCounters;
    CircularQueue<int>           availableDelivery;
    CircularQueue<AssignmentJob> waitingCounter;
    CircularQueue<AssignmentJob> waitingDelivery;
    CircularQueue<StatusUpdate>  updates;

    int  indexOf(const std::string& resourceId) const;

    CircularQueue<int>&                 availableQueue(ResourceType t);
    const CircularQueue<int>&           availableQueue(ResourceType t) const;
    CircularQueue<AssignmentJob>&       waitingQueue(ResourceType t);
    const CircularQueue<AssignmentJob>& waitingQueue(ResourceType t) const;

    bool validateJobIds(const std::string& requestId, const std::string& parcelId) const;
    void occupy(int index, const AssignmentJob& job);
    void freeResource(int index);
    void serveWaitingJobs(ResourceType t);
    void pushUpdate(const StatusUpdate& u);
    void printQueues(ResourceType t) const;
};

#endif
