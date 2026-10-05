/**
 * DSA: STACK (LIFO - LAST IN FIRST OUT)
 * 
 * Purpose:
 * Provides an Undo mechanism for faculty attendance operations.
 * If a faculty member inadvertently marks a student "ABSENT" instead of "PRESENT",
 * or updates a session attendance record incorrectly, the change record is pushed
 * onto this Stack. When "Undo Correction" is triggered, the most recent operation
 * is popped (LIFO) and reverted in the database.
 * 
 * Viva Explainability:
 * - Principle: Last-In-First-Out (LIFO)
 * - Time Complexity: O(1) for Push, O(1) for Pop, O(1) for Peek.
 * - Space Complexity: O(k) where k is the maximum undo-history depth (capped to prevent memory leakage).
 * - Implementation: Linked stack with dynamic node allocation.
 */

import { AttendanceStatus } from "../types";

export interface StackNode<T> {
  value: T;
  next: StackNode<T> | null;
}

export interface AttendanceCorrectionAction {
  id: string;
  recordId: string;
  sessionId: string;
  studentId: string;
  studentName: string;
  subjectCode: string;
  previousStatus: AttendanceStatus;
  newStatus: AttendanceStatus;
  timestamp: Date;
  facultyId: string;
  reason?: string;
}

export class AttendanceStack<T = AttendanceCorrectionAction> {
  private top: StackNode<T> | null = null;
  private count = 0;
  private maxDepth: number;

  constructor(maxDepth = 100) {
    this.maxDepth = maxDepth;
  }

  /**
   * Pushes a new correction action onto the stack.
   * Time: O(1)
   */
  public push(value: T): void {
    const newNode: StackNode<T> = { value, next: this.top };
    this.top = newNode;
    this.count++;

    // Prune bottom if exceeding max depth to protect memory
    if (this.count > this.maxDepth) {
      this.pruneBottom();
    }
  }

  /**
   * Pops and returns the most recent correction action.
   * Time: O(1)
   */
  public pop(): T | null {
    if (this.top === null) {
      return null;
    }
    const poppedValue = this.top.value;
    this.top = this.top.next;
    this.count--;
    return poppedValue;
  }

  /**
   * Peeks at the most recent action without removing it.
   * Time: O(1)
   */
  public peek(): T | null {
    return this.top ? this.top.value : null;
  }

  public isEmpty(): boolean {
    return this.count === 0;
  }

  public size(): number {
    return this.count;
  }

  public clear(): void {
    this.top = null;
    this.count = 0;
  }

  /**
   * Converts stack contents to array (from top to bottom).
   */
  public toArray(): T[] {
    const arr: T[] = [];
    let current = this.top;
    while (current !== null) {
      arr.push(current.value);
      current = current.next;
    }
    return arr;
  }

  private pruneBottom(): void {
    let current = this.top;
    if (!current) return;
    let index = 0;
    while (current.next && index < this.maxDepth - 1) {
      current = current.next;
      index++;
    }
    if (current) {
      current.next = null;
      this.count = this.maxDepth;
    }
  }
}

// Global in-memory undo stack for attendance corrections
export const globalAttendanceUndoStack = new AttendanceStack<AttendanceCorrectionAction>();
