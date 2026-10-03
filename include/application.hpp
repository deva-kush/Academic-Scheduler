#pragma once
#include "data_loader.hpp"

struct ApplicationData {
    CourseList courses;
    RoomList rooms;
    ExamList exams;
    EventList events;
    CourseStudentRegister registrations;
};

ApplicationData loadData();
void printUsage();
