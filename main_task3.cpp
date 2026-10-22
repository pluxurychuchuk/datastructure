// ============================================================================
// main_task3.cpp
// Standalone test menu for Task 3 - Collection Counter and Delivery Resource
// Management Module. In the integrated system, the team's main menu calls the
// same ResourceManager functions instead (exclude this file there).
//
// Compile:  g++ -std=c++17 -Wall main_task3.cpp ResourceManager.cpp -o task3
// ============================================================================
#include <iostream>
#include <string>
#include <cctype>
#include "ResourceManager.hpp"

using namespace std;

// Read a full line and trim surrounding spaces.
static string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) return "";
    size_t start = s.find_first_not_of(" \t\r");
    size_t end   = s.find_last_not_of(" \t\r");
    return start == string::npos ? "" : s.substr(start, end - start + 1);
}

// Read an integer safely (no cin fail state). Returns -1 if not a number.
static int readInt(const string& prompt) {
    string s = readLine(prompt);
    if (s.empty() || s.length() > 3) return -1;
    for (char c : s)
        if (!isdigit(static_cast<unsigned char>(c))) return -1;
    return stoi(s);
}

static string readUpper(const string& prompt) {
    string s = readLine(prompt);
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

static bool readType(ResourceType& type) {
    int t = readInt("  Service needs (1 = Collection Counter, 2 = Delivery Personnel): ");
    if (t == 1) { type = ResourceType::COUNTER;  return true; }
    if (t == 2) { type = ResourceType::DELIVERY; return true; }
    cout << "  [Error] Invalid choice.\n";
    return false;
}

// Shows the status changes Task 3 produced. In the integrated system, these
// lines are replaced by calls to Task 1 (parcel), Task 2 (request), Task 4 (history).
static void syncUpdates(ResourceManager& manager) {
    StatusUpdate u;
    while (manager.pollUpdate(u)) {
        cout << "  [Sync] " << u.requestId << " -> " << requestStatusToString(u.requestStatus);
        if (u.parcelChanged)
            cout << " | " << u.parcelId << " -> " << parcelStatusToString(u.parcelStatus);
        if (!u.resourceId.empty()) cout << " | " << u.resourceId;
        cout << "\n";
    }
}

int main() {
    ResourceManager manager;
    cout << "Loading resources from resources.txt...\n";
    int loaded = manager.loadFromFile("resources.txt");
    cout << loaded << " resource(s) loaded.\n";

    while (true) {
        cout << "\n===== TASK 3: COLLECTION COUNTER & DELIVERY RESOURCE MANAGEMENT =====\n"
             << "  1. Register a new resource (C## counter / D## delivery)\n"
             << "  2. Display all resources\n"
             << "  3. Display current availability (and rotation order)\n"
             << "  4. Assign request to next available resource (round-robin)\n"
             << "  5. Assign request to a specific resource\n"
             << "  6. Complete job & release resource\n"
             << "  7. Cancel a request's assignment\n"
             << "  8. Mark resource unavailable\n"
             << "  9. Mark resource available again\n"
             << " 10. Find a resource / find which resource handles a request\n"
             << "  0. Exit\n";
        int choice = readInt("Choice: ");

        switch (choice) {
        case 1: {
            string id   = readUpper("  Resource ID (e.g. C04 / D03): ");
            string name = readLine("  Name/description: ");
            manager.registerResource(id, name);
            break;
        }
        case 2: manager.displayAllResources(); break;
        case 3: manager.displayAvailability(); break;
        case 4: {
            string requestId = readUpper("  Request ID (R001): ");
            string parcelId  = readUpper("  Parcel ID (P001): ");
            ResourceType type;
            if (readType(type)) manager.assignNext(requestId, parcelId, type);
            break;
        }
        case 5: {
            string resId     = readUpper("  Resource ID: ");
            string requestId = readUpper("  Request ID (R001): ");
            string parcelId  = readUpper("  Parcel ID (P001): ");
            manager.assignToResource(resId, requestId, parcelId);
            break;
        }
        case 6: manager.releaseResource(readUpper("  Resource ID: ")); break;
        case 7: manager.cancelAssignment(readUpper("  Request ID: ")); break;
        case 8: manager.markUnavailable(readUpper("  Resource ID: ")); break;
        case 9: manager.markAvailable(readUpper("  Resource ID: ")); break;
        case 10: {
            string key = readUpper("  Resource ID or Request ID: ");
            const Resource* r = manager.findResource(key);
            if (r) {
                cout << "  " << r->id << " | " << r->name << " | " << typeToString(r->type)
                     << " | " << statusToString(r->status);
                if (r->status == ResourceStatus::OCCUPIED)
                    cout << " | " << r->assignedRequestId << ", Parcel " << r->assignedParcelId;
                cout << " | Completed: " << r->jobsCompleted << "\n";
            } else if (!manager.findResourceOfRequest(key).empty()) {
                cout << "  " << key << " is being handled by "
                     << manager.findResourceOfRequest(key) << ".\n";
            } else if (manager.isRequestWaiting(key)) {
                cout << "  " << key << " is PENDING in a waiting line.\n";
            } else {
                cout << "  [Error] No resource or active request found for '" << key << "'.\n";
            }
            break;
        }
        case 0:
            cout << "Exiting Task 3 module.\n";
            return 0;
        default:
            cout << "  [Error] Invalid choice. Enter 0-10.\n";
        }
        syncUpdates(manager);
    }
}
