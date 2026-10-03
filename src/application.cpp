#include "../include/application.hpp"

ApplicationData loadData() {
    ApplicationData data;

    data.courses = loadCoursesCSV("courses.csv");
    data.rooms = loadRoomsCSV("rooms.csv");
    data.exams = loadExamsCSV("exams.csv", data.courses);
    data.events = loadEventsCSV("events.csv");
    data.registrations =
        loadRegSubCSV("regsub.csv", data.courses.size());

    return data;
}

void printUsage() {
    cout << "Invalid choice.\n";
    cout << "Available modes:\n";
    cout << "  2 - Exam slot scheduling\n";
    cout << "  3 - Exam slot + room allocation\n";
    cout << "  4 - Event room scheduling\n";
    cout << "  5 - Class timetable generation\n";
}
