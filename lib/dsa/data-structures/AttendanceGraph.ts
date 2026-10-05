/**
 * DSA: GRAPH (ADJACENCY LIST)
 * 
 * Purpose:
 * Models relationships within the college ecosystem:
 * Faculty -> teaches -> Subject -> enrolls -> Student.
 * Enables graph traversals (BFS, DFS) to discover peer-networks, analyze faculty reach,
 * find multi-subject student cohorts, and explore structural dependencies.
 * 
 * Viva Explainability:
 * - Representation: Adjacency List (Map of NodeId -> Set of NeighborNodeIds)
 * - Traversal Algorithms:
 *    * BFS (Breadth-First Search): Level-by-level traversal using a Queue. Time O(V + E), Space O(V).
 *      Use case: Finding all students within k hops of a Faculty member or subject.
 *    * DFS (Depth-First Search): Deep exploration using Recursion / Stack. Time O(V + E), Space O(V).
 *      Use case: Path finding, cycle detection, and detecting connected subgraphs of at-risk students.
 */

import { GraphNode, GraphEdge } from "../types";

export interface TraversalResult {
  order: string[];
  nodesVisited: number;
  visitedSet: string[];
  levels?: Record<string, number>;
  pathFound?: string[];
}

export class AttendanceGraph {
  private nodes: Map<string, GraphNode> = new Map();
  private adjacencyList: Map<string, Set<string>> = new Map();
  private edges: GraphEdge[] = [];

  public addNode(node: GraphNode): void {
    if (!this.nodes.has(node.id)) {
      this.nodes.set(node.id, node);
      this.adjacencyList.set(node.id, new Set());
    }
  }

  public addEdge(edge: GraphEdge, bidirectional = false): void {
    if (!this.adjacencyList.has(edge.source)) {
      this.adjacencyList.set(edge.source, new Set());
    }
    if (!this.adjacencyList.has(edge.target)) {
      this.adjacencyList.set(edge.target, new Set());
    }

    this.adjacencyList.get(edge.source)!.add(edge.target);
    this.edges.push(edge);

    if (bidirectional) {
      this.adjacencyList.get(edge.target)!.add(edge.source);
      this.edges.push({
        source: edge.target,
        target: edge.source,
        relationship: edge.relationship,
      });
    }
  }

  public getNode(id: string): GraphNode | undefined {
    return this.nodes.get(id);
  }

  public getNeighbors(id: string): string[] {
    const neighbors = this.adjacencyList.get(id);
    return neighbors ? Array.from(neighbors) : [];
  }

  /**
   * Breadth-First Search (BFS)
   * Explores graph layer-by-layer starting from startNodeId using a Queue.
   * Time Complexity: O(V + E)
   * Space Complexity: O(V)
   */
  public bfs(startNodeId: string): TraversalResult {
    const order: string[] = [];
    const visited = new Set<string>();
    const queue: string[] = [];
    const levels: Record<string, number> = {};

    if (!this.adjacencyList.has(startNodeId)) {
      return { order, nodesVisited: 0, visitedSet: [] };
    }

    visited.add(startNodeId);
    queue.push(startNodeId);
    levels[startNodeId] = 0;

    while (queue.length > 0) {
      const current = queue.shift()!;
      order.push(current);

      const currentLevel = levels[current];
      const neighbors = this.adjacencyList.get(current) || new Set();

      for (const neighbor of neighbors) {
        if (!visited.has(neighbor)) {
          visited.add(neighbor);
          levels[neighbor] = currentLevel + 1;
          queue.push(neighbor);
        }
      }
    }

    return {
      order,
      nodesVisited: visited.size,
      visitedSet: Array.from(visited),
      levels,
    };
  }

  /**
   * Depth-First Search (DFS)
   * Explores as deep as possible along each branch before backtracking.
   * Time Complexity: O(V + E)
   * Space Complexity: O(V)
   */
  public dfs(startNodeId: string): TraversalResult {
    const order: string[] = [];
    const visited = new Set<string>();

    if (!this.adjacencyList.has(startNodeId)) {
      return { order, nodesVisited: 0, visitedSet: [] };
    }

    const dfsRecursive = (current: string) => {
      visited.add(current);
      order.push(current);

      const neighbors = this.adjacencyList.get(current) || new Set();
      for (const neighbor of neighbors) {
        if (!visited.has(neighbor)) {
          dfsRecursive(neighbor);
        }
      }
    };

    dfsRecursive(startNodeId);

    return {
      order,
      nodesVisited: visited.size,
      visitedSet: Array.from(visited),
    };
  }

  /**
   * Finds shortest path between two nodes using BFS.
   */
  public findShortestPath(startId: string, targetId: string): string[] | null {
    if (!this.adjacencyList.has(startId) || !this.adjacencyList.has(targetId)) {
      return null;
    }

    const queue: string[] = [startId];
    const visited = new Set<string>([startId]);
    const parent = new Map<string, string>();

    while (queue.length > 0) {
      const current = queue.shift()!;
      if (current === targetId) {
        // Reconstruct path
        const path: string[] = [];
        let step: string | undefined = targetId;
        while (step) {
          path.unshift(step);
          step = parent.get(step);
        }
        return path;
      }

      for (const neighbor of this.getNeighbors(current)) {
        if (!visited.has(neighbor)) {
          visited.add(neighbor);
          parent.set(neighbor, current);
          queue.push(neighbor);
        }
      }
    }

    return null;
  }

  public getGraphData(): { nodes: GraphNode[]; edges: GraphEdge[] } {
    return {
      nodes: Array.from(this.nodes.values()),
      edges: this.edges,
    };
  }
}
