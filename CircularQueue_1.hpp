// ============================================================================
// CircularQueue.hpp
// Task 3 - Collection Counter and Delivery Resource Management Module
//
// A fixed-size circular queue built on a raw array (no STL containers).
//
//   front -> index of the first (oldest) element
//   rear  -> index of the last (newest) element
//   count -> number of elements currently stored
//
// When front/rear reach the end of the array they "wrap around" to index 0
// using modulo (%). Slots freed by dequeue are reused, so nothing is ever
// shifted and enqueue/dequeue/peek are all O(1).
// ============================================================================
#ifndef CIRCULAR_QUEUE_HPP
#define CIRCULAR_QUEUE_HPP

template <typename T>
class CircularQueue {
private:
    T*  data;       // dynamically allocated fixed-size array
    int capacity;   // maximum number of elements
    int front;      // index of first element
    int rear;       // index of last element
    int count;      // current number of elements

public:
    explicit CircularQueue(int cap = 10)
        : capacity(cap > 0 ? cap : 10), front(0), rear(-1), count(0) {
        data = new T[capacity];
    }

    ~CircularQueue() { delete[] data; }

    // Copying is disabled: two queues sharing one array would double-delete it.
    CircularQueue(const CircularQueue&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;

    bool isEmpty() const { return count == 0; }
    bool isFull()  const { return count == capacity; }
    int  size()    const { return count; }
    int  getCapacity() const { return capacity; }

    // Add to the rear. O(1). Returns false if the queue is full.
    bool enqueue(const T& item) {
        if (isFull()) return false;
        rear = (rear + 1) % capacity;     // wrap around
        data[rear] = item;
        count++;
        return true;
    }

    // Remove from the front. O(1). Returns false if the queue is empty.
    bool dequeue(T& out) {
        if (isEmpty()) return false;
        out = data[front];
        front = (front + 1) % capacity;   // wrap around
        count--;
        return true;
    }

    // Look at the front without removing it. O(1).
    bool peek(T& out) const {
        if (isEmpty()) return false;
        out = data[front];
        return true;
    }

    // i-th element counted from the front (0 = front). Used for display/search.
    // Caller must make sure 0 <= i < size().
    const T& at(int i) const { return data[(front + i) % capacity]; }

    // Linear search. O(n).
    bool contains(const T& target) const {
        for (int i = 0; i < count; i++)
            if (at(i) == target) return true;
        return false;
    }

    // Remove a specific element from anywhere in the queue. O(n).
    // Done by ROTATION: every element is dequeued once and re-enqueued,
    // except the target. Only queue operations are used and the original
    // order of the remaining elements is preserved.
    bool remove(const T& target) {
        bool found = false;
        int n = count;
        for (int i = 0; i < n; i++) {
            T item;
            dequeue(item);
            if (!found && item == target) {
                found = true;          // drop it (only the first match)
                continue;
            }
            enqueue(item);
        }
        return found;
    }

    // Insert an element so that it becomes the FRONT. O(n).
    // Enqueue at the rear, then rotate the other (size-1) elements behind it.
    bool enqueueFront(const T& item) {
        if (!enqueue(item)) return false;
        for (int i = 0; i < count - 1; i++) {
            T moved;
            dequeue(moved);
            enqueue(moved);
        }
        return true;
    }
};

#endif
