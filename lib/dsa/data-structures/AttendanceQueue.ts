/**
 * DSA: QUEUE (FIFO - FIRST IN FIRST OUT)
 * 
 * Purpose:
 * Processes sequential attendance events, alert dispatches, and risk notifications
 * in strict chronological order. For example, when a batch of attendance records is submitted,
 * events are placed in this queue and consumed sequentially to update risk calculations and send alerts.
 * 
 * Viva Explainability:
 * - Principle: First-In-First-Out (FIFO)
 * - Time Complexity: O(1) for Enqueue, O(1) for Dequeue, O(1) for Peek.
 * - Space Complexity: O(n) where n is the number of queued events.
 * - Implementation: Singly-linked list with head and tail pointers to prevent O(n) array shift operations.
 */

export interface QueueNode<T> {
  value: T;
  next: QueueNode<T> | null;
}

export interface AttendanceEvent {
  id: string;
  type: "RECORD_CREATED" | "ATTENDANCE_CORRECTED" | "RISK_ALERT" | "NOTIFICATION";
  studentId: string;
  studentName?: string;
  subjectCode?: string;
  message: string;
  timestamp: Date;
  severity: "INFO" | "WARNING" | "CRITICAL";
}

export class AttendanceQueue<T = AttendanceEvent> {
  private head: QueueNode<T> | null = null;
  private tail: QueueNode<T> | null = null;
  private length = 0;

  /**
   * Adds an element to the rear of the queue.
   * Time: O(1)
   */
  public enqueue(value: T): void {
    const newNode: QueueNode<T> = { value, next: null };
    if (this.tail === null) {
      this.head = newNode;
      this.tail = newNode;
    } else {
      this.tail.next = newNode;
      this.tail = newNode;
    }
    this.length++;
  }

  /**
   * Removes and returns the element at the front of the queue.
   * Time: O(1)
   */
  public dequeue(): T | null {
    if (this.head === null) {
      return null;
    }
    const removedValue = this.head.value;
    this.head = this.head.next;
    if (this.head === null) {
      this.tail = null;
    }
    this.length--;
    return removedValue;
  }

  /**
   * Inspects the front element without removing it.
   * Time: O(1)
   */
  public peek(): T | null {
    return this.head ? this.head.value : null;
  }

  public isEmpty(): boolean {
    return this.length === 0;
  }

  public size(): number {
    return this.length;
  }

  /**
   * Converts queue to array for visualization/inspection.
   */
  public toArray(): T[] {
    const arr: T[] = [];
    let current = this.head;
    while (current !== null) {
      arr.push(current.value);
      current = current.next;
    }
    return arr;
  }
}

// Global in-memory event alert queue instance
export const globalAlertQueue = new AttendanceQueue<AttendanceEvent>();
