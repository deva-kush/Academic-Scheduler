#pragma once
#include "exam_scheduler.hpp"
#include "bipartite_matcher.hpp"
#include "utilities.hpp"

class SlotWiseExamScheduler {
    ExamList& exams_;
    RoomList& rooms_;

public:
    SlotWiseExamScheduler(ExamList& exams, RoomList& rooms)
        : exams_(exams), rooms_(rooms) {}

    void schedule(const ExamSlotScheduler& slotScheduler) {
        vector<vector<int>> groups(Config::MAX_SLOTS);

        for (int exam = 0; exam < exams_.size(); ++exam) {
            int slot = slotScheduler.getSlot(exam);
            if (slot >= 0) groups[slot].push_back(exam);
        }

        cout << "\n=== SLOT-WISE EXAM ROOM ALLOCATION ===\n";
        cout << "Algorithm: Bipartite Matching + Capacity-Fit Ordering\n";

        int grandWaste = 0;
        int grandAssigned = 0;

        for (int slot = 0; slot < Config::MAX_SLOTS; ++slot) {
            if (groups[slot].empty()) continue;

            const auto& group = groups[slot];
            const int leftSize = static_cast<int>(group.size());
            const int rightSize = rooms_.size();

            cout << "\n--- SLOT " << slot + 1
                 << " (" << leftSize << " exam(s)) ---\n";

            BipartiteGraph graph;
            graph.setSizes(leftSize, rightSize);

            // Edges are added from smallest waste to largest waste.
            // Matching still guarantees at most one exam per room.
            for (int localExam = 0; localExam < leftSize; ++localExam) {
                int examIndex = group[localExam];
                int required = exams_[examIndex].students;

                vector<pair<int, int>> candidates;

                for (int room = 0; room < rightSize; ++room) {
                    int capacity = rooms_[room].capacity;

                    if (capacity >= required) {
                        candidates.push_back({
                            capacity - required, room
                        });
                    }
                }

                sort(candidates.begin(), candidates.end());

                for (const auto& [waste, room] : candidates) {
                    (void)waste;
                    graph.addEdge(localExam, room);
                }
            }

            BipartiteMatcher matcher(graph);
            int matched = matcher.match();

            const vector<int> widths = {14, 8, 12, 6, 12, 8, 7, 30};
            printHLine(widths);
            printRow({
                "Course", "Students", "Date", "Room",
                "Building", "RoomCap", "Waste", "Selection Reason"
            }, widths);
            printHLine(widths);

            int slotWaste = 0;

            for (int room = 0; room < rightSize; ++room) {
                int localExam = matcher.leftForRight(room);
                if (localExam == -1) continue;

                int examIndex = group[localExam];
                int required = exams_[examIndex].students;
                int capacity = rooms_[room].capacity;
                int waste = capacity - required;

                slotWaste += waste;

                string reason =
                    (waste == 0)
                        ? "Perfect fit"
                        : "Capacity fit: +" + to_string(waste) + " spare";

                printRow({
                    exams_[examIndex].courseName,
                    to_string(required),
                    exams_[examIndex].date,
                    "R" + to_string(rooms_[room].id),
                    rooms_[room].building,
                    to_string(capacity),
                    to_string(waste),
                    reason
                }, widths);
            }

            for (int localExam = 0; localExam < leftSize; ++localExam) {
                bool found = false;

                for (int room = 0; room < rightSize; ++room) {
                    if (matcher.leftForRight(room) == localExam) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    int examIndex = group[localExam];

                    printRow({
                        exams_[examIndex].courseName,
                        to_string(exams_[examIndex].students),
                        exams_[examIndex].date,
                        "NO ROOM", "-", "-", "-",
                        "No room with sufficient capacity"
                    }, widths);
                }
            }

            printHLine(widths);

            grandAssigned += matched;
            grandWaste += slotWaste;

            cout << "Slot " << slot + 1 << ": "
                 << matched << "/" << leftSize
                 << " assigned | Wasted seats: "
                 << slotWaste << '\n';
        }

        cout << "\n--- Grand Summary ---\n";
        cout << "Total assigned : " << grandAssigned << '\n';
        cout << "Total wasted   : " << grandWaste << " seats\n";
    }
};

// -----------------------------------------------------------------------------
