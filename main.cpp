#include "include/application.hpp"
#include "include/utilities.hpp"
#include "include/event_scheduler.hpp"
#include "include/class_scheduler.hpp"
#include "include/exam_room_scheduler.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
int main() {
    try {
        ApplicationData data = loadData();

        if (data.rooms.size() == 0) {
            cerr << "ERROR: No rooms loaded from rooms.csv\n";
            return 1;
        }

        string input;
        if (!getline(cin, input)) {
            cerr << "ERROR: No scheduling mode received\n";
            return 1;
        }

        int choice = 0;
        if (!tryParseInt(trim(input), choice)) {
            cerr << "ERROR: Invalid scheduling mode\n";
            return 1;
        }

        switch (choice) {
            case 2: {
                ExamSlotScheduler scheduler(
                    data.exams,
                    data.registrations,
                    data.courses,
                    data.rooms.size()
                );
                scheduler.assignSlots();
                break;
            }

            case 3: {
                ExamSlotScheduler slotScheduler(
                    data.exams,
                    data.registrations,
                    data.courses,
                    data.rooms.size()
                );
                slotScheduler.assignSlots();

                SlotWiseExamScheduler roomScheduler(
                    data.exams,
                    data.rooms
                );
                roomScheduler.schedule(slotScheduler);
                break;
            }

            case 4: {
                EventScheduler scheduler(data.events, data.rooms);
                scheduler.schedule();
                break;
            }

            case 5: {
                ClassScheduler scheduler(
                    data.courses,
                    data.rooms,
                    data.registrations
                );
                scheduler.generate();
                break;
            }

            default:
                printUsage();
                return 1;
        }

        return 0;

    } catch (const exception& ex) {
        cerr << "FATAL ERROR: " << ex.what() << '\n';
        return 1;
    }
}
