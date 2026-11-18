#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include "json.hpp"

namespace attendance::dsa {

struct GraphNode {
    std::string id;
    std::string label;
    std::string type; // FACULTY, SUBJECT, STUDENT
    nlohmann::json metadata;
};

struct GraphEdge {
    std::string source;
    std::string target;
    std::string relationship; // TEACHES, ENROLLED_IN
};

struct TraversalResult {
    std::vector<std::string> order;
    size_t nodesVisited{0};
    std::vector<std::string> visitedSet;
    std::unordered_map<std::string, int> levels;
    std::vector<std::string> pathFound;
};

class AttendanceGraph {
public:
    void addNode(const GraphNode& node) {
        if (nodes_.find(node.id) == nodes_.end()) {
            nodes_[node.id] = node;
            adjacencyList_[node.id] = {};
        }
    }

    void addEdge(const GraphEdge& edge, bool bidirectional = false) {
        if (adjacencyList_.find(edge.source) == adjacencyList_.end()) {
            adjacencyList_[edge.source] = {};
        }
        if (adjacencyList_.find(edge.target) == adjacencyList_.end()) {
            adjacencyList_[edge.target] = {};
        }

        adjacencyList_[edge.source].insert(edge.target);
        edges_.push_back(edge);

        if (bidirectional) {
            adjacencyList_[edge.target].insert(edge.source);
            edges_.push_back({edge.target, edge.source, edge.relationship});
        }
    }

    bool hasNode(const std::string& id) const {
        return nodes_.find(id) != nodes_.end();
    }

    std::vector<std::string> getNeighbors(const std::string& id) const {
        auto it = adjacencyList_.find(id);
        if (it != adjacencyList_.end()) {
            return std::vector<std::string>(it->second.begin(), it->second.end());
        }
        return {};
    }

    // Breadth-First Search (BFS): O(V + E)
    TraversalResult bfs(const std::string& startNodeId) const {
        TraversalResult result;
        if (adjacencyList_.find(startNodeId) == adjacencyList_.end()) {
            return result;
        }

        std::unordered_set<std::string> visited;
        std::queue<std::string> q;

        visited.insert(startNodeId);
        q.push(startNodeId);
        result.levels[startNodeId] = 0;

        while (!q.empty()) {
            std::string current = q.front();
            q.pop();

            result.order.push_back(current);
            int currentLevel = result.levels[current];

            auto it = adjacencyList_.find(current);
            if (it != adjacencyList_.end()) {
                // To maintain deterministic order for tests, sort neighbors
                std::vector<std::string> neighbors(it->second.begin(), it->second.end());
                std::sort(neighbors.begin(), neighbors.end());

                for (const auto& neighbor : neighbors) {
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        result.levels[neighbor] = currentLevel + 1;
                        q.push(neighbor);
                    }
                }
            }
        }

        result.nodesVisited = result.order.size();
        result.visitedSet = std::vector<std::string>(visited.begin(), visited.end());
        return result;
    }

    // Depth-First Search (DFS): O(V + E)
    TraversalResult dfs(const std::string& startNodeId) const {
        TraversalResult result;
        if (adjacencyList_.find(startNodeId) == adjacencyList_.end()) {
            return result;
        }

        std::unordered_set<std::string> visited;
        dfsRecursive(startNodeId, visited, result.order);

        result.nodesVisited = result.order.size();
        result.visitedSet = std::vector<std::string>(visited.begin(), visited.end());
        return result;
    }

    // Shortest path via BFS
    std::vector<std::string> findShortestPath(const std::string& startNodeId, const std::string& targetNodeId) const {
        if (adjacencyList_.find(startNodeId) == adjacencyList_.end() ||
            adjacencyList_.find(targetNodeId) == adjacencyList_.end()) {
            return {};
        }

        std::unordered_set<std::string> visited;
        std::queue<std::string> q;
        std::unordered_map<std::string, std::string> parent;

        visited.insert(startNodeId);
        q.push(startNodeId);

        bool found = false;
        while (!q.empty() && !found) {
            std::string current = q.front();
            q.pop();

            if (current == targetNodeId) {
                found = true;
                break;
            }

            auto it = adjacencyList_.find(current);
            if (it != adjacencyList_.end()) {
                for (const auto& neighbor : it->second) {
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        parent[neighbor] = current;
                        q.push(neighbor);
                        if (neighbor == targetNodeId) {
                            found = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!found) return {};

        std::vector<std::string> path;
        std::string curr = targetNodeId;
        while (curr != startNodeId) {
            path.push_back(curr);
            curr = parent[curr];
        }
        path.push_back(startNodeId);
        std::reverse(path.begin(), path.end());
        return path;
    }

    const std::unordered_map<std::string, GraphNode>& getNodes() const { return nodes_; }
    const std::vector<GraphEdge>& getEdges() const { return edges_; }

private:
    void dfsRecursive(const std::string& node, std::unordered_set<std::string>& visited, std::vector<std::string>& order) const {
        visited.insert(node);
        order.push_back(node);

        auto it = adjacencyList_.find(node);
        if (it != adjacencyList_.end()) {
            std::vector<std::string> neighbors(it->second.begin(), it->second.end());
            std::sort(neighbors.begin(), neighbors.end());
            for (const auto& neighbor : neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    dfsRecursive(neighbor, visited, order);
                }
            }
        }
    }

    std::unordered_map<std::string, GraphNode> nodes_;
    std::unordered_map<std::string, std::unordered_set<std::string>> adjacencyList_;
    std::vector<GraphEdge> edges_;
};

} // namespace attendance::dsa
