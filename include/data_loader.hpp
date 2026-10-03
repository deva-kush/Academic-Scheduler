#pragma once
#include "models.hpp"

RoomList loadRoomsCSV(const string& filename);
CourseList loadCoursesCSV(const string& filename);
ExamList loadExamsCSV(const string& filename, const CourseList& courses);
EventList loadEventsCSV(const string& filename);
CourseStudentRegister loadRegSubCSV(const string& filename, int courseCount);