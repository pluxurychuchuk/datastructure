// ============================================================================
// ResourceManager.cpp
// Task 3 - Collection Counter and Delivery Resource Management Module
// ============================================================================
#include "ResourceManager.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

ResourceManager::ResourceManager()
    : resourceCount(0),
      availableCounters(MAX_RESOURCES),
      availableDelivery(MAX_RESOURCES),
      waitingCounter(MAX_WAITING),
      waitingDelivery(MAX_WAITING),
      updates(MAX_UPDATES) {}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

// Linear search in the registry array. O(n), n <= MAX_RESOURCES (small).
int ResourceManager::indexOf(const string& resourceId) const {
    for (int i = 0; i < resourceCount; i++)
        if (resources[i].id == resourceId) return i;
    return -1;
}

CircularQueue<int>& ResourceManager::availableQueue(ResourceType t) {
    return t == ResourceType::COUNTER ? availableCounters : availableDelivery;
}
const CircularQueue<int>& ResourceManager::availableQueue(ResourceType t) const {
    return t == ResourceType::COUNTER ? availableCounters : availableDelivery;
}
CircularQueue<AssignmentJob>& ResourceManager::waitingQueue(ResourceType t) {
    return t == ResourceType::COUNTER ? waitingCounter : waitingDelivery;
}
const CircularQueue<AssignmentJob>& ResourceManager::waitingQueue(ResourceType t) const {
    return t == ResourceType::COUNTER ? waitingCounter : waitingDelivery;
}

bool ResourceManager::validateJobIds(const string& requestId, const string& parcelId) const {
    if (!isValidRequestId(requestId)) {
        cout << "  [Error] Invalid Request ID '" << requestId << "'. Format: R001.\n";
        return false;
    }
    if (!isValidParcelId(parcelId)) {
        cout << "  [Error] Invalid Parcel ID '" << parcelId << "'. Format: P001.\n";
        return false;
    }
    return true;
}

// Status changes are buffered for Task 1/2/4. If the buffer is full the
// oldest change is dropped (circular buffer behaviour).
void ResourceManager::pushUpdate(const StatusUpdate& u) {
    if (updates.isFull()) {
        StatusUpdate oldest;
        updates.dequeue(oldest);
    }
    updates.enqueue(u);
}

// Put a request on a resource and mark the resource OCCUPIED.
void ResourceManager::occupy(int index, const AssignmentJob& job) {
    Resource& r = resources[index];
    r.status           = ResourceStatus::OCCUPIED;
    r.assignedRequestId = job.requestId;
    r.assignedParcelId  = job.parcelId;

    StatusUpdate u;
    u.requestId     = job.requestId;
    u.parcelId      = job.parcelId;
    u.resourceId    = r.id;
    u.requestStatus = RequestStatus::ASSIGNED;
    u.parcelChanged = true;
    u.parcelStatus  = (r.type == ResourceType::COUNTER) ? ParcelStatus::READY_FOR_COLLECTION
                                                         : ParcelStatus::OUT_FOR_DELIVERY;
    pushUpdate(u);

    cout << "  -> " << job.requestId << " (Parcel " << job.parcelId << ") assigned to "
         << r.id << " [" << r.name << "]\n";
}

// Make a resource AVAILABLE again, put it at the REAR of its rotation,
// then let it serve anyone who is waiting.
void ResourceManager::freeResource(int index) {
    Resource& r = resources[index];
    r.status = ResourceStatus::AVAILABLE;
    r.assignedRequestId.clear();
    r.assignedParcelId.clear();
    availableQueue(r.type).enqueue(index);
    serveWaitingJobs(r.type);
}

// While there are waiting requests AND free resources of this type, pair them.
void ResourceManager::serveWaitingJobs(ResourceType t) {
    CircularQueue<int>&           avail = availableQueue(t);
    CircularQueue<AssignmentJob>& wait  = waitingQueue(t);

    while (!wait.isEmpty() && !avail.isEmpty()) {
        AssignmentJob job;
        int index;
        wait.dequeue(job);
        avail.dequeue(index);
        cout << "  Waiting request served automatically:\n";
        occupy(index, job);
    }
}

// ---------------------------------------------------------------------------
// 1. Register and maintain resources
// ---------------------------------------------------------------------------
bool ResourceManager::registerResource(const string& id, const string& name) {
    ResourceType type;
    if (isValidCounterId(id))       type = ResourceType::COUNTER;
    else if (isValidDeliveryId(id)) type = ResourceType::DELIVERY;
    else {
        cout << "  [Error] Invalid Resource ID '" << id
             << "'. Use C + 2 digits for counters (C01) or D + 2 digits for delivery (D01).\n";
        return false;
    }
    if (name.empty()) {
        cout << "  [Error] Resource name cannot be empty.\n";
        return false;
    }
    if (indexOf(id) != -1) {
        cout << "  [Error] Resource ID " << id << " already exists.\n";
        return false;
    }
    if (resourceCount >= MAX_RESOURCES) {
        cout << "  [Error] Resource registry is full (max " << MAX_RESOURCES << ").\n";
        return false;
    }

    Resource& r = resources[resourceCount];
    r.id   = id;
    r.name = name;
    r.type = type;
    r.status = ResourceStatus::AVAILABLE;
    r.assignedRequestId.clear();
    r.assignedParcelId.clear();
    r.jobsCompleted = 0;

    availableQueue(type).enqueue(resourceCount);   // joins the rotation
    resourceCount++;

    cout << "  Registered " << typeToString(type) << " " << id << " (" << name << ").\n";
    serveWaitingJobs(type);   // a new resource may serve a waiting request at once
    return true;
}

// File format (one per line):  ID,Name     (lines starting with # are ignored)
int ResourceManager::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cout << "  [Warning] Could not open " << filename << ". No resources loaded.\n";
        return 0;
    }

    int loaded = 0, lineNo = 0;
    string line;
    while (getline(file, line)) {
        lineNo++;
        if (!line.empty() && line.back() == '\r') line.pop_back();   // Windows line endings
        if (line.empty() || line[0] == '#') continue;

        size_t comma = line.find(',');
        if (comma == string::npos) {
            cout << "  [Warning] Line " << lineNo << " skipped (expected ID,Name).\n";
            continue;
        }
        if (registerResource(line.substr(0, comma), line.substr(comma + 1))) loaded++;
    }
    return loaded;
}

// ---------------------------------------------------------------------------
// 2. Display current resource availability
// ---------------------------------------------------------------------------
void ResourceManager::displayAllResources() const {
    if (resourceCount == 0) {
        cout << "  No resources registered.\n";
        return;
    }
    cout << "\n  " << left << setw(6) << "ID" << setw(26) << "Name" << setw(10) << "Type"
         << setw(13) << "Status" << setw(9) << "Request" << setw(8) << "Parcel" << "Done\n";
    cout << "  " << string(76, '-') << "\n";
    for (int i = 0; i < resourceCount; i++) {
        const Resource& r = resources[i];
        cout << "  " << left << setw(6) << r.id << setw(26) << r.name.substr(0, 25)
             << setw(10) << typeToString(r.type) << setw(13) << statusToString(r.status)
             << setw(9) << (r.assignedRequestId.empty() ? "-" : r.assignedRequestId)
             << setw(8) << (r.assignedParcelId.empty() ? "-" : r.assignedParcelId)
             << r.jobsCompleted << "\n";
    }
}

// Round-robin order of free resources + waiting line for one type.
void ResourceManager::printQueues(ResourceType t) const {
    const CircularQueue<int>&           avail = availableQueue(t);
    const CircularQueue<AssignmentJob>& wait  = waitingQueue(t);

    cout << "  " << typeToString(t) << " rotation (next first): ";
    if (avail.isEmpty()) cout << "(none free)";
    for (int i = 0; i < avail.size(); i++)
        cout << resources[avail.at(i)].id << (i < avail.size() - 1 ? " -> " : "");
    cout << "\n  " << typeToString(t) << " waiting line: ";
    if (wait.isEmpty()) cout << "(empty)";
    for (int i = 0; i < wait.size(); i++)
        cout << wait.at(i).requestId << (i < wait.size() - 1 ? " -> " : "");
    cout << "\n";
}

void ResourceManager::displayAvailability() const {
    if (resourceCount == 0) {
        cout << "  No resources registered.\n";
        return;
    }
    const ResourceStatus groups[3] = { ResourceStatus::AVAILABLE, ResourceStatus::OCCUPIED,
                                       ResourceStatus::UNAVAILABLE };
    for (ResourceStatus s : groups) {
        int shown = 0;
        cout << "\n  == " << statusToString(s) << " ==\n";
        for (int i = 0; i < resourceCount; i++) {
            const Resource& r = resources[i];
            if (r.status != s) continue;
            cout << "  " << left << setw(6) << r.id << setw(10) << typeToString(r.type) << r.name;
            if (s == ResourceStatus::OCCUPIED)
                cout << "  | " << r.assignedRequestId << ", Parcel " << r.assignedParcelId;
            cout << "\n";
            shown++;
        }
        if (shown == 0) cout << "  (none)\n";
    }
    cout << "\n";
    printQueues(ResourceType::COUNTER);
    printQueues(ResourceType::DELIVERY);
}

// ---------------------------------------------------------------------------
// 3. Assign a service request
// ---------------------------------------------------------------------------
AssignOutcome ResourceManager::assignNext(const string& requestId, const string& parcelId,
                                          ResourceType type) {
    if (!validateJobIds(requestId, parcelId)) return AssignOutcome::REJECTED;

    if (!findResourceOfRequest(requestId).empty() || isRequestWaiting(requestId)) {
        cout << "  [Error] Duplicate assignment: " << requestId
             << " is already assigned or waiting.\n";
        return AssignOutcome::REJECTED;
    }

    // Is there any active resource of this type at all?
    bool typeExists = false;
    for (int i = 0; i < resourceCount; i++)
        if (resources[i].type == type && resources[i].status != ResourceStatus::UNAVAILABLE) {
            typeExists = true;
            break;
        }
    if (!typeExists) {
        cout << "  [Error] No suitable resource: no active " << typeToString(type)
             << " resources exist.\n";
        return AssignOutcome::REJECTED;
    }

    AssignmentJob job{requestId, parcelId, type};
    CircularQueue<int>& avail = availableQueue(type);

    // All suitable resources occupied -> wait in FIFO line (request stays PENDING)
    if (avail.isEmpty()) {
        if (!waitingQueue(type).enqueue(job)) {
            cout << "  [Error] All " << typeToString(type)
                 << " resources are occupied and the waiting line is full.\n";
            return AssignOutcome::REJECTED;
        }
        cout << "  All " << typeToString(type) << " resources are occupied. " << requestId
             << " stays PENDING in the waiting line (position "
             << waitingQueue(type).size() << ").\n";
        return AssignOutcome::QUEUED;
    }

    // Round-robin: the resource at the front gets the request
    int index;
    avail.dequeue(index);
    occupy(index, job);
    return AssignOutcome::ASSIGNED;
}

bool ResourceManager::assignToResource(const string& resourceId, const string& requestId,
                                       const string& parcelId) {
    int index = indexOf(resourceId);
    if (index == -1) {
        cout << "  [Error] Invalid Resource ID: " << resourceId << " not found.\n";
        return false;
    }
    if (!validateJobIds(requestId, parcelId)) return false;

    Resource& r = resources[index];
    if (r.status == ResourceStatus::UNAVAILABLE) {
        cout << "  [Error] " << resourceId << " is unavailable and cannot be selected.\n";
        return false;
    }
    if (r.status == ResourceStatus::OCCUPIED) {
        cout << "  [Error] " << resourceId << " is occupied with " << r.assignedRequestId << ".\n";
        return false;
    }
    if (!findResourceOfRequest(requestId).empty() || isRequestWaiting(requestId)) {
        cout << "  [Error] Duplicate assignment: " << requestId
             << " is already assigned or waiting.\n";
        return false;
    }

    availableQueue(r.type).remove(index);   // take it out of the rotation (O(n))
    occupy(index, AssignmentJob{requestId, parcelId, r.type});
    return true;
}

// ---------------------------------------------------------------------------
// 4. Update resource status
// ---------------------------------------------------------------------------
bool ResourceManager::releaseResource(const string& resourceId) {
    int index = indexOf(resourceId);
    if (index == -1) {
        cout << "  [Error] Invalid Resource ID: " << resourceId << " not found.\n";
        return false;
    }
    Resource& r = resources[index];
    if (r.status != ResourceStatus::OCCUPIED) {
        cout << "  [Error] Invalid release: " << resourceId << " is "
             << statusToString(r.status) << ", not occupied.\n";
        return false;
    }

    StatusUpdate u;
    u.requestId     = r.assignedRequestId;
    u.parcelId      = r.assignedParcelId;
    u.resourceId    = r.id;
    u.requestStatus = RequestStatus::COMPLETED;
    u.parcelChanged = true;
    u.parcelStatus  = (r.type == ResourceType::COUNTER) ? ParcelStatus::COLLECTED
                                                         : ParcelStatus::DELIVERED;
    pushUpdate(u);

    cout << "  " << r.id << " completed " << u.requestId << " (Parcel " << u.parcelId << ").\n";
    r.jobsCompleted++;
    freeResource(index);   // back to the REAR of the rotation + serve waiting
    return true;
}

// Request cancelled (e.g. by Task 2): free its resource or remove it from the waiting line.
bool ResourceManager::cancelAssignment(const string& requestId) {
    if (!isValidRequestId(requestId)) {
        cout << "  [Error] Invalid Request ID '" << requestId << "'. Format: R001.\n";
        return false;
    }

    StatusUpdate u;
    u.requestId     = requestId;
    u.requestStatus = RequestStatus::CANCELLED;

    string resId = findResourceOfRequest(requestId);
    if (!resId.empty()) {
        int index = indexOf(resId);
        u.parcelId   = resources[index].assignedParcelId;
        u.resourceId = resId;
        pushUpdate(u);
        cout << "  " << requestId << " cancelled. " << resId << " released (not counted as completed).\n";
        freeResource(index);
        return true;
    }

    // Not on a resource -> maybe waiting. remove() rotates the queue (O(n)).
    for (int t = 0; t < 2; t++) {
        CircularQueue<AssignmentJob>& wait =
            waitingQueue(t == 0 ? ResourceType::COUNTER : ResourceType::DELIVERY);
        for (int i = 0; i < wait.size(); i++) {
            if (wait.at(i).requestId == requestId) {
                AssignmentJob target = wait.at(i);   // copy: remove() moves elements
                u.parcelId = target.parcelId;
                wait.remove(target);
                pushUpdate(u);
                cout << "  " << requestId << " cancelled and removed from the waiting line.\n";
                return true;
            }
        }
    }

    cout << "  [Error] " << requestId << " has no active or waiting assignment.\n";
    return false;
}

bool ResourceManager::markUnavailable(const string& resourceId) {
    int index = indexOf(resourceId);
    if (index == -1) {
        cout << "  [Error] Invalid Resource ID: " << resourceId << " not found.\n";
        return false;
    }
    Resource& r = resources[index];

    if (r.status == ResourceStatus::UNAVAILABLE) {
        cout << "  [Info] " << resourceId << " is already unavailable.\n";
        return false;
    }

    if (r.status == ResourceStatus::AVAILABLE) {
        availableQueue(r.type).remove(index);   // leave the rotation
        r.status = ResourceStatus::UNAVAILABLE;
        cout << "  " << resourceId << " marked unavailable.\n";
        return true;
    }

    // OCCUPIED: resource becomes unavailable DURING processing.
    // The request must not be lost -> give it to another free resource, or put
    // it at the FRONT of the waiting line (it was already being served).
    AssignmentJob interrupted{r.assignedRequestId, r.assignedParcelId, r.type};
    r.status = ResourceStatus::UNAVAILABLE;
    r.assignedRequestId.clear();
    r.assignedParcelId.clear();
    cout << "  " << resourceId << " became unavailable while handling "
         << interrupted.requestId << ". Reassigning...\n";

    CircularQueue<int>& avail = availableQueue(interrupted.neededType);
    if (!avail.isEmpty()) {
        int newIndex;
        avail.dequeue(newIndex);
        occupy(newIndex, interrupted);           // new ASSIGNED update with new resource
        return true;
    }

    StatusUpdate u;                              // back to PENDING
    u.requestId     = interrupted.requestId;
    u.parcelId      = interrupted.parcelId;
    u.requestStatus = RequestStatus::PENDING;
    pushUpdate(u);

    if (waitingQueue(interrupted.neededType).enqueueFront(interrupted))
        cout << "  No free " << typeToString(interrupted.neededType) << " resource. "
             << interrupted.requestId << " is PENDING at the FRONT of the waiting line.\n";
    else
        cout << "  [Warning] Waiting line full. " << interrupted.requestId
             << " is PENDING and must be reassigned manually.\n";
    return true;
}

bool ResourceManager::markAvailable(const string& resourceId) {
    int index = indexOf(resourceId);
    if (index == -1) {
        cout << "  [Error] Invalid Resource ID: " << resourceId << " not found.\n";
        return false;
    }
    Resource& r = resources[index];
    if (r.status == ResourceStatus::AVAILABLE) {
        cout << "  [Info] " << resourceId << " is already available.\n";
        return false;
    }
    if (r.status == ResourceStatus::OCCUPIED) {
        cout << "  [Error] " << resourceId << " is occupied. Complete the job to release it.\n";
        return false;
    }

    cout << "  " << resourceId << " is available again.\n";
    freeResource(index);
    return true;
}

// ---------------------------------------------------------------------------
// Lookup
// ---------------------------------------------------------------------------
const Resource* ResourceManager::findResource(const string& resourceId) const {
    int index = indexOf(resourceId);
    return index == -1 ? nullptr : &resources[index];
}

string ResourceManager::findResourceOfRequest(const string& requestId) const {
    for (int i = 0; i < resourceCount; i++)
        if (resources[i].status == ResourceStatus::OCCUPIED &&
            resources[i].assignedRequestId == requestId)
            return resources[i].id;
    return "";
}

bool ResourceManager::isRequestWaiting(const string& requestId) const {
    AssignmentJob probe{requestId, "", ResourceType::COUNTER};   // compared by Request ID only
    return waitingCounter.contains(probe) || waitingDelivery.contains(probe);
}
