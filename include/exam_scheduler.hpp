#pragma once
#include "models.hpp"
#include "utilities.hpp"

class ExamHeap {
    priority_queue<pair<int, int>> heap_;

public:
    void push(int priority, int examIndex) {
        heap_.push({priority, examIndex});
    }

    bool empty() const { return heap_.empty(); }

    pair<int, int> pop() {
        auto value = heap_.top();
        heap_.pop();
        return value;
    }
};

// -----------------------------------------------------------------------------

class ExamSlotScheduler {
    ExamList& exams_;
    const CourseStudentRegister& registrations_;
    const CourseList& courses_;
    int roomCount_;

    array<int, Config::MAX_EXAMS> slotOfExam_{};
    vector<int> order_;

    int courseIndexById(int courseId) const {
        for (int i = 0; i < courses_.size(); ++i) {
            if (courses_[i].id == courseId) return i;
        }
        return -1;
    }

    bool examsConflict(int examA, int examB) const {
        int courseA = courseIndexById(exams_[examA].courseId);
        int courseB = courseIndexById(exams_[examB].courseId);

        if (courseA == -1 || courseB == -1) return false;

        return registrations_.conflict(courseA, courseB);
    }

    int slotCount(int slot) const {
        int count = 0;

        for (int i = 0; i < exams_.size(); ++i) {
            if (slotOfExam_[i] == slot) ++count;
        }

        return count;
    }

    bool canPlace(int examIndex, int slot) const {
        for (int i = 0; i < exams_.size(); ++i) {
            if (slotOfExam_[i] == slot &&
                examsConflict(examIndex, i)) {
                return false;
            }
        }

        return true;
    }

    void buildOrder() {
        ExamHeap heap;

        for (int i = 0; i < exams_.size(); ++i) {
            heap.push(exams_[i].priority, i);
        }

        order_.clear();

        while (!heap.empty()) {
            order_.push_back(heap.pop().second);
        }
    }

    int computeMinSlots() const {
        array<int, Config::MAX_EXAMS> temporary;
        temporary.fill(-1);

        int maxSlot = 0;

        for (int examIndex : order_) {
            for (int slot = 0; slot < Config::MAX_SLOTS; ++slot) {
                bool valid = true;

                for (int other = 0; other < exams_.size(); ++other) {
                    if (temporary[other] == slot &&
                        examsConflict(examIndex, other)) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    temporary[examIndex] = slot;
                    maxSlot = max(maxSlot, slot + 1);
                    break;
                }
            }
        }

        return maxSlot;
    }

public:
    ExamSlotScheduler(
        ExamList& exams,
        const CourseStudentRegister& registrations,
        const CourseList& courses,
        int roomCount
    )
        : exams_(exams),
          registrations_(registrations),
          courses_(courses),
          roomCount_(roomCount) {
        slotOfExam_.fill(-1);
    }

    void assignSlots() {
        buildOrder();
        const int minSlots = computeMinSlots();

        cout << "\n=== EXAM SLOT ANALYSIS ===\n";
        cout << "-----------------------------------------\n";
        cout << "Total exams      : " << exams_.size() << '\n';
        cout << "Available rooms  : " << roomCount_ << '\n';
        cout << "Min slots reqd   : " << minSlots << '\n';
        cout << "-----------------------------------------\n";
        cout << "\nPriority queue + first-fit conflict-free slot assignment.\n\n";

        const vector<int> widths = {4, 14, 12, 8, 4, 5};
        printHLine(widths);
        printRow({"ID", "Course", "Date", "Slot", "Stu", "Prio"}, widths);
        printHLine(widths);

        for (int examIndex : order_) {
            bool placed = false;

            for (int slot = 0; slot < Config::MAX_SLOTS; ++slot) {
                if (canPlace(examIndex, slot) &&
                    slotCount(slot) < roomCount_) {

                    slotOfExam_[examIndex] = slot;

                    printRow({
                        to_string(exams_[examIndex].id),
                        exams_[examIndex].courseName,
                        exams_[examIndex].date,
                        "Slot " + to_string(slot + 1),
                        to_string(exams_[examIndex].students),
                        to_string(exams_[examIndex].priority)
                    }, widths);

                    placed = true;
                    break;
                }
            }

            if (!placed) {
                printRow({
                    to_string(exams_[examIndex].id),
                    exams_[examIndex].courseName,
                    exams_[examIndex].date,
                    "UNPLACED",
                    to_string(exams_[examIndex].students),
                    to_string(exams_[examIndex].priority)
                }, widths);
            }
        }

        printHLine(widths);
    }

    int getSlot(int examIndex) const {
        if (examIndex < 0 || examIndex >= Config::MAX_EXAMS) return -1;
        return slotOfExam_[examIndex];
    }

    const vector<int>& getOrder() const {
        return order_;
    }
};

// -----------------------------------------------------------------------------
