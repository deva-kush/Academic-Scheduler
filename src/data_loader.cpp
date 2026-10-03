#include "../include/data_loader.hpp"
#include "../include/utilities.hpp"

template <typename Parser, typename List>
void loadCSVRows(const string& filename, List& list, Parser parser) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Unable to open " + filename);
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        ++lineNumber;
        line = trim(line);

        if (line.empty()) continue;

        try {
            auto fields = splitCSV(line);
            parser(fields, list);
        } catch (const exception& ex) {
            cerr << "[WARN] Skipping " << filename << " line "
                 << lineNumber << ": " << ex.what() << '\n';
        }
    }
}

RoomList loadRoomsCSV(const string& filename) {
    RoomList rooms;

    loadCSVRows(filename, rooms, [](const vector<string>& d, RoomList& list) {
        if (d.size() < 5 || !isInteger(d[0])) return;

        Classroom room;
        if (!tryParseInt(d[0], room.id) ||
            !tryParseInt(d[1], room.capacity) ||
            !tryParseDouble(d[3], room.x) ||
            !tryParseDouble(d[4], room.y)) {
            throw runtime_error("invalid room values");
        }

        room.building = d[2];

        if (!list.add(room)) {
            throw runtime_error("maximum room limit reached");
        }
    });

    return rooms;
}

CourseList loadCoursesCSV(const string& filename) {
    CourseList courses;

    loadCSVRows(filename, courses, [](const vector<string>& d, CourseList& list) {
        if (d.size() < 4 || !isInteger(d[0])) return;

        Course course;
        if (!tryParseInt(d[0], course.id) ||
            !tryParseInt(d[2], course.students)) {
            throw runtime_error("invalid course values");
        }

        course.name = d[1];
        course.dept = d[3];

        if (d.size() >= 5 && !tryParseInt(d[4], course.occurrences)) {
            throw runtime_error("invalid occurrences");
        }

        course.occurrences = max(1, course.occurrences);

        if (!list.add(course)) {
            throw runtime_error("maximum course limit reached");
        }
    });

    return courses;
}

ExamList loadExamsCSV(const string& filename, const CourseList& courses) {
    ExamList exams;

    loadCSVRows(filename, exams,
        [&courses](const vector<string>& d, ExamList& list) {
            if (d.size() < 4 || !isInteger(d[0])) return;

            Exam exam;
            if (!tryParseInt(d[0], exam.id) ||
                !tryParseInt(d[1], exam.courseId) ||
                !tryParseInt(d[3], exam.priority)) {
                throw runtime_error("invalid exam values");
            }

            exam.date = d[2];

            for (int i = 0; i < courses.size(); ++i) {
                if (courses[i].id == exam.courseId) {
                    exam.students = courses[i].students;
                    exam.courseName = courses[i].name;
                    break;
                }
            }

            if (exam.courseName.empty()) {
                exam.courseName = "Course#" + to_string(exam.courseId);
            }

            if (!list.add(exam)) {
                throw runtime_error("maximum exam limit reached");
            }
        });

    return exams;
}

EventList loadEventsCSV(const string& filename) {
    EventList events;

    loadCSVRows(filename, events, [](const vector<string>& d, EventList& list) {
        if (d.size() < 4 || !isInteger(d[0])) return;

        Event event;
        if (!tryParseInt(d[0], event.id) ||
            !tryParseInt(d[2], event.participants) ||
            !tryParseInt(d[3], event.priority)) {
            throw runtime_error("invalid event values");
        }

        event.title = d[1];

        if (!list.add(event)) {
            throw runtime_error("maximum event limit reached");
        }
    });

    return events;
}

CourseStudentRegister loadRegSubCSV(
    const string& filename,
    int courseCount
) {
    CourseStudentRegister registerData;
    registerData.setCounts(Config::MAX_STUDENTS, courseCount);

    ifstream file(filename);
    if (!file) {
        throw runtime_error("Unable to open " + filename);
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty()) continue;

        auto d = splitCSV(line);
        if (d.size() < 2) continue;

        int studentId = 0;
        int courseId = 0;

        if (!tryParseInt(d[0], studentId) || !tryParseInt(d[1], courseId)) {
            continue;
        }

        // CSV stores 1-based student/course IDs; internal matrix is 0-based.
        if (!registerData.add(studentId - 1, courseId - 1)) {
            cerr << "[WARN] Invalid registration at line "
                 << lineNumber << '\n';
        }
    }

    return registerData;
}

// -----------------------------------------------------------------------------
