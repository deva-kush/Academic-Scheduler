#pragma once
#include "config.hpp"

struct Classroom {
    int id = 0;
    int capacity = 0;
    string building;
    double x = 0.0;
    double y = 0.0;
};

struct Exam {
    int id = 0;
    int courseId = 0;
    int priority = 0;
    int students = 0;
    string date;
    string courseName;
};

struct Event {
    int id = 0;
    int participants = 0;
    int priority = 0;
    string title;
};

struct Course {
    int id = 0;
    int students = 0;
    int occurrences = 1;
    string name;
    string dept;
};

// -----------------------------------------------------------------------------
// Fixed-capacity collections
// -----------------------------------------------------------------------------
template <typename T, int Capacity>
class FixedList {
    array<T, Capacity> data_{};
    int size_ = 0;

public:
    bool add(const T& value) {
        if (size_ >= Capacity) return false;
        data_[size_++] = value;
        return true;
    }

    int size() const { return size_; }

    T& operator[](int index) { return data_[index]; }
    const T& operator[](int index) const { return data_[index]; }

    T* begin() { return data_.data(); }
    T* end() { return data_.data() + size_; }
    const T* begin() const { return data_.data(); }
    const T* end() const { return data_.data() + size_; }

};

using RoomList = FixedList<Classroom, Config::MAX_ROOMS>;
using ExamList = FixedList<Exam, Config::MAX_EXAMS>;
using EventList = FixedList<Event, Config::MAX_EVENTS>;
using CourseList = FixedList<Course, Config::MAX_COURSES>;

// -----------------------------------------------------------------------------
// Student-course registration matrix
// -----------------------------------------------------------------------------
class CourseStudentRegister {
    bool matrix_[Config::MAX_STUDENTS][Config::MAX_COURSES]{};
    int studentCount_ = 0;
    int courseCount_ = 0;

public:
    void setCounts(int students, int courses) {
        studentCount_ = clamp(students, 0, Config::MAX_STUDENTS);
        courseCount_ = clamp(courses, 0, Config::MAX_COURSES);
    }

    bool add(int studentIndex, int courseIndex) {
        if (studentIndex < 0 || studentIndex >= Config::MAX_STUDENTS ||
            courseIndex < 0 || courseIndex >= Config::MAX_COURSES) {
            return false;
        }

        matrix_[studentIndex][courseIndex] = true;
        return true;
    }

    bool conflict(int courseA, int courseB) const {
        if (courseA < 0 || courseB < 0 ||
            courseA >= courseCount_ || courseB >= courseCount_) {
            return false;
        }

        for (int student = 0; student < studentCount_; ++student) {
            if (matrix_[student][courseA] && matrix_[student][courseB]) {
                return true;
            }
        }
        return false;
    }
};

// -----------------------------------------------------------------------------
