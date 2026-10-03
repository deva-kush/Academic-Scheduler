#pragma once
#include "common.hpp"

class BipartiteGraph {
    vector<vector<int>> adjacency_;
    int leftSize_ = 0;
    int rightSize_ = 0;

public:
    void setSizes(int left, int right) {
        leftSize_ = left;
        rightSize_ = right;
        adjacency_.assign(left, {});
    }

    void addEdge(int left, int right) {
        if (left >= 0 && left < leftSize_ &&
            right >= 0 && right < rightSize_) {
            adjacency_[left].push_back(right);
        }
    }

    const vector<int>& neighbors(int left) const {
        return adjacency_[left];
    }

    int leftSize() const { return leftSize_; }
    int rightSize() const { return rightSize_; }
};

class BipartiteMatcher {
    const BipartiteGraph& graph_;
    vector<int> matchRight_;
    vector<bool> visited_;

    bool dfs(int left) {
        for (int right : graph_.neighbors(left)) {
            if (visited_[right]) continue;

            visited_[right] = true;

            if (matchRight_[right] == -1 ||
                dfs(matchRight_[right])) {
                matchRight_[right] = left;
                return true;
            }
        }

        return false;
    }

public:
    explicit BipartiteMatcher(const BipartiteGraph& graph)
        : graph_(graph),
          matchRight_(graph.rightSize(), -1),
          visited_(graph.rightSize(), false) {}

    int match() {
        int matched = 0;

        for (int left = 0; left < graph_.leftSize(); ++left) {
            fill(visited_.begin(), visited_.end(), false);

            if (dfs(left)) ++matched;
        }

        return matched;
    }

    int leftForRight(int right) const {
        if (right < 0 || right >= static_cast<int>(matchRight_.size())) {
            return -1;
        }
        return matchRight_[right];
    }
};

// -----------------------------------------------------------------------------
