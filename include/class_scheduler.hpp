#pragma once
#include "models.hpp"
#include "utilities.hpp"

class ClassScheduler {
    struct SlotInfo {
        int day;
        int period;
        int room;
    };

    CourseList& courses_;
    RoomList& rooms_;
    const CourseStudentRegister& registrations_;

    array<array<array<int, Config::PERIODS>, Config::DAYS>,
          Config::MAX_ROOMS> timetable_{};

    array<array<bool, Config::DAYS>, Config::MAX_COURSES> courseOnDay_{};
    array<vector<SlotInfo>, Config::MAX_COURSES> assigned_{};

    static constexpr int EMPTY = -1;

    bool canPlace(int courseIndex, int day, int period) const {
        if (courseOnDay_[courseIndex][day]) return false;

        for (int room = 0; room < rooms_.size(); ++room) {
            int otherCourse = timetable_[room][day][period];

            if (otherCourse != EMPTY &&
                registrations_.conflict(courseIndex, otherCourse)) {
                return false;
            }
        }

        return true;
    }

    int bestRoom(int students, int day, int period) const {
        int best = -1;
        int minimumWaste = numeric_limits<int>::max();

        for (int room = 0; room < rooms_.size(); ++room) {
            if (timetable_[room][day][period] != EMPTY) continue;

            const int capacity = rooms_[room].capacity;

            if (capacity >= students) {
                const int waste = capacity - students;

                if (waste < minimumWaste) {
                    minimumWaste = waste;
                    best = room;
                }
            }
        }

        return best;
    }

    void printTimetable() const {
        static const array<string, Config::DAYS> DAYS = {
            "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
        };

        constexpr int dayWidth = 5;
        constexpr int cellWidth = 12;

        cout << "\n=== 7-DAY OPTIMIZED CLASS TIMETABLE ===\n";
        cout << "Rule: each course appears at most once per day.\n\n";

        auto bar = [&]() {
            cout << string(dayWidth + 2, '-');
            for (int p = 0; p < Config::PERIODS; ++p) {
                cout << string(cellWidth + 2, '-');
            }
            cout << '\n';
        };

        bar();
        cout << ' ' << col("Day", dayWidth) << ' ';

        for (int p = 0; p < Config::PERIODS; ++p) {
            cout << ' ' << col("Per " + to_string(p + 1), cellWidth) << ' ';
        }
        cout << '\n';
        bar();

        for (int day = 0; day < Config::DAYS; ++day) {
            cout << ' ' << col(DAYS[day], dayWidth) << ' ';

            for (int period = 0; period < Config::PERIODS; ++period) {
                string cell = "-";

                for (int room = 0; room < rooms_.size(); ++room) {
                    int courseIndex = timetable_[room][day][period];

                    if (courseIndex != EMPTY) {
                        cell = courses_[courseIndex].name;
                        break;
                    }
                }

                cout << ' ' << col(cell, cellWidth) << ' ';
            }

            cout << '\n';
        }

        bar();
    }

    void printAssignments() const {
        static const array<string, Config::DAYS> DAYS = {
            "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
        };

        const vector<int> widths = {18, 8, 5, 6, 8, 8, 8};

        cout << "\n--- Detailed Slot Assignments ---\n";
        printHLine(widths);
        printRow({"Course", "Students", "Occ", "Day",
                  "Period", "Room", "RoomCap"}, widths);
        printHLine(widths);

        for (int courseIndex = 0; courseIndex < courses_.size(); ++courseIndex) {
            const Course& course = courses_[courseIndex];

            if (assigned_[courseIndex].empty()) {
                printRow({
                    course.name,
                    to_string(course.students),
                    to_string(course.occurrences),
                    "NOT SCHEDULED",
                    "", "", ""
                }, widths);
                continue;
            }

            for (const SlotInfo& slot : assigned_[courseIndex]) {
                printRow({
                    course.name,
                    to_string(course.students),
                    to_string(course.occurrences),
                    DAYS[slot.day],
                    "P" + to_string(slot.period + 1),
                    "R" + to_string(rooms_[slot.room].id),
                    to_string(rooms_[slot.room].capacity)
                }, widths);
            }
        }

        printHLine(widths);
    }

public:
    ClassScheduler(
        CourseList& courses,
        RoomList& rooms,
        const CourseStudentRegister& registrations
    )
        : courses_(courses),
          rooms_(rooms),
          registrations_(registrations) {

        for (auto& rooms : timetable_) {
            for (auto& days : rooms) {
                days.fill(EMPTY);
            }
        }

        for (auto& course : courseOnDay_) {
            course.fill(false);
        }
    }

    void generate() {
        vector<int> order(courses_.size());
        iota(order.begin(), order.end(), 0);

        // Do NOT sort courses_ itself. Registration matrix depends on
        // the original course index.
        stable_sort(order.begin(), order.end(), [&](int a, int b) {
            if (courses_[a].students != courses_[b].students) {
                return courses_[a].students > courses_[b].students;
            }
            return courses_[a].id < courses_[b].id;
        });

        for (int courseIndex : order) {
            const Course& course = courses_[courseIndex];
            int placed = 0;

            for (int day = 0;
                 day < Config::DAYS && placed < course.occurrences;
                 ++day) {

                if (courseOnDay_[courseIndex][day]) continue;

                for (int period = 0;
                     period < Config::PERIODS &&
                     placed < course.occurrences;
                     ++period) {

                    if (!canPlace(courseIndex, day, period)) continue;

                    int room = bestRoom(course.students, day, period);
                    if (room == -1) continue;

                    timetable_[room][day][period] = courseIndex;
                    courseOnDay_[courseIndex][day] = true;
                    assigned_[courseIndex].push_back({day, period, room});
                    ++placed;
                    break;
                }
            }

            if (placed < course.occurrences) {
                cout << "[WARN] " << course.name
                     << " requires " << course.occurrences
                     << " slots but only " << placed
                     << " were scheduled.\n";
            }
        }

        printTimetable();
        printAssignments();
    }
};

// -----------------------------------------------------------------------------
