/**
 * DSA: HASHMAP (HASH TABLE)
 * 
 * Purpose:
 * Fast O(1) average-time lookups for student records, subject attendance,
 * and roll-number indexing without iterating through arrays or running database scans.
 * 
 * Viva Explainability:
 * - Time Complexity: Average O(1) for Insert, Delete, and Search. Worst-case O(n) during high collision.
 * - Space Complexity: O(n) where n is the number of student entries.
 * - Collision Resolution: Separate Chaining (Linked Nodes in buckets) with dynamic load-factor tracking.
 */

export interface HashNode<K, V> {
  key: K;
  value: V;
  next: HashNode<K, V> | null;
}

export class StudentHashMap<K extends string, V> {
  private buckets: Array<HashNode<K, V> | null>;
  private capacity: number;
  private size: number;
  private collisionCount: number;

  constructor(initialCapacity = 32) {
    this.capacity = initialCapacity;
    this.buckets = new Array(this.capacity).fill(null);
    this.size = 0;
    this.collisionCount = 0;
  }

  /**
   * Polynomial Rolling Hash Function (djb2 variant)
   * Converts string key into a deterministic bucket index.
   */
  private hash(key: string): number {
    let hash = 5381;
    for (let i = 0; i < key.length; i++) {
      hash = ((hash << 5) + hash) + key.charCodeAt(i);
      hash = hash & hash; // Convert to 32bit integer
    }
    return Math.abs(hash) % this.capacity;
  }

  /**
   * Inserts or updates a key-value pair.
   * Average Time: O(1)
   */
  public set(key: K, value: V): void {
    const index = this.hash(key);
    let head = this.buckets[index];

    // Check if key already exists (update)
    let current = head;
    while (current !== null) {
      if (current.key === key) {
        current.value = value;
        return;
      }
      current = current.next;
    }

    // Insert new node at bucket head
    if (head !== null) {
      this.collisionCount++;
    }

    const newNode: HashNode<K, V> = {
      key,
      value,
      next: head,
    };
    this.buckets[index] = newNode;
    this.size++;

    // Resize if load factor exceeds 0.75
    if (this.size / this.capacity > 0.75) {
      this.resize(this.capacity * 2);
    }
  }

  /**
   * Retrieves a value by key.
   * Average Time: O(1)
   */
  public get(key: K): V | undefined {
    const index = this.hash(key);
    let current = this.buckets[index];

    while (current !== null) {
      if (current.key === key) {
        return current.value;
      }
      current = current.next;
    }
    return undefined;
  }

  /**
   * Checks if key exists.
   * Average Time: O(1)
   */
  public has(key: K): boolean {
    return this.get(key) !== undefined;
  }

  /**
   * Removes a key.
   * Average Time: O(1)
   */
  public delete(key: K): boolean {
    const index = this.hash(key);
    let current = this.buckets[index];
    let prev: HashNode<K, V> | null = null;

    while (current !== null) {
      if (current.key === key) {
        if (prev === null) {
          this.buckets[index] = current.next;
        } else {
          prev.next = current.next;
        }
        this.size--;
        return true;
      }
      prev = current;
      current = current.next;
    }
    return false;
  }

  public getSize(): number {
    return this.size;
  }

  public getCapacity(): number {
    return this.capacity;
  }

  public values(): V[] {
    const results: V[] = [];
    for (const head of this.buckets) {
      let current = head;
      while (current !== null) {
        results.push(current.value);
        current = current.next;
      }
    }
    return results;
  }

  public entries(): Array<{ key: K; value: V }> {
    const list: Array<{ key: K; value: V }> = [];
    for (const head of this.buckets) {
      let current = head;
      while (current !== null) {
        list.push({ key: current.key, value: current.value });
        current = current.next;
      }
    }
    return list;
  }

  /**
   * Rehashes all entries when table load factor triggers resizing.
   */
  private resize(newCapacity: number): void {
    const oldBuckets = this.buckets;
    this.capacity = newCapacity;
    this.buckets = new Array(this.capacity).fill(null);
    this.size = 0;
    this.collisionCount = 0;

    for (const head of oldBuckets) {
      let current = head;
      while (current !== null) {
        this.set(current.key, current.value);
        current = current.next;
      }
    }
  }

  /**
   * Diagnostic statistics for Viva viva demonstration.
   */
  public getDiagnostics() {
    let emptyBuckets = 0;
    let maxChainLength = 0;

    for (const head of this.buckets) {
      if (head === null) {
        emptyBuckets++;
      } else {
        let length = 0;
        let curr: HashNode<K, V> | null = head;
        while (curr) {
          length++;
          curr = curr.next;
        }
        if (length > maxChainLength) maxChainLength = length;
      }
    }

    return {
      size: this.size,
      capacity: this.capacity,
      loadFactor: Number((this.size / this.capacity).toFixed(3)),
      emptyBuckets,
      occupiedBuckets: this.capacity - emptyBuckets,
      maxChainLength,
      collisionCount: this.collisionCount,
    };
  }
}
