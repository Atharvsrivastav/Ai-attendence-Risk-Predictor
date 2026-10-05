/**
 * DSA: SET (HASH SET)
 * 
 * Purpose:
 * Enforces uniqueness of student IDs, session dates, subject enrollments, and absence patterns.
 * Enables mathematical set operations to isolate cross-subject chronic absentees and track distinct attendance anomalies.
 * 
 * Viva Explainability:
 * - Time Complexity: Average O(1) for Add, Contains, and Remove.
 * - Set Operations:
 *    * Union: O(A + B)
 *    * Intersection: O(min(|A|, |B|))
 *    * Difference: O(|A|)
 * - Space Complexity: O(n) where n is the number of distinct elements.
 */

export class AttendanceSet<T extends string | number> {
  private map: Map<T, boolean>;

  constructor(initialElements?: Iterable<T>) {
    this.map = new Map();
    if (initialElements) {
      for (const el of initialElements) {
        this.add(el);
      }
    }
  }

  public add(value: T): this {
    this.map.set(value, true);
    return this;
  }

  public has(value: T): boolean {
    return this.map.has(value);
  }

  public delete(value: T): boolean {
    return this.map.delete(value);
  }

  public size(): number {
    return this.map.size;
  }

  public clear(): void {
    this.map.clear();
  }

  public toArray(): T[] {
    return Array.from(this.map.keys());
  }

  /**
   * Set Intersection: Elements present in both sets.
   * Useful for: Identifying students who missed multiple subjects on the same date.
   */
  public intersection(other: AttendanceSet<T>): AttendanceSet<T> {
    const result = new AttendanceSet<T>();
    const [smaller, larger] = this.size() < other.size() ? [this, other] : [other, this];

    for (const item of smaller.toArray()) {
      if (larger.has(item)) {
        result.add(item);
      }
    }
    return result;
  }

  /**
   * Set Union: Elements present in either set.
   * Useful for: Aggregating all distinct students flagged across any risk category.
   */
  public union(other: AttendanceSet<T>): AttendanceSet<T> {
    const result = new AttendanceSet<T>(this.toArray());
    for (const item of other.toArray()) {
      result.add(item);
    }
    return result;
  }

  /**
   * Set Difference: Elements in this set that are not in the other set.
   * Useful for: Finding students who recovered this week (present in past at-risk set, but absent from current at-risk set).
   */
  public difference(other: AttendanceSet<T>): AttendanceSet<T> {
    const result = new AttendanceSet<T>();
    for (const item of this.toArray()) {
      if (!other.has(item)) {
        result.add(item);
      }
    }
    return result;
  }
}
