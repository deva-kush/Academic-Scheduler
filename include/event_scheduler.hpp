#pragma once
#include "models.hpp"
#include "utilities.hpp"

class EventScheduler {
    EventList& events_;
    RoomList& rooms_;

public:
    EventScheduler(EventList& events, RoomList& rooms)
        : events_(events), rooms_(rooms) {}

    void schedule() {
        vector<int> order(events_.size());
        iota(order.begin(), order.end(), 0);

        sort(order.begin(), order.end(), [&](int a, int b) {
            if (events_[a].priority != events_[b].priority) {
                return events_[a].priority > events_[b].priority;
            }
            return events_[a].participants > events_[b].participants;
        });

        vector<int> roomOrder(rooms_.size());
        iota(roomOrder.begin(), roomOrder.end(), 0);

        sort(roomOrder.begin(), roomOrder.end(), [&](int a, int b) {
            if (rooms_[a].capacity != rooms_[b].capacity) {
                return rooms_[a].capacity < rooms_[b].capacity;
            }
            return rooms_[a].id < rooms_[b].id;
        });

        vector<bool> used(rooms_.size(), false);

        cout << "\n=== OPTIMIZED EVENT SCHEDULING ===\n";

        const vector<int> widths = {4, 18, 8, 12, 10, 12, 6};
        printHLine(widths);
        printRow({"ID", "Event", "Room", "Building",
                  "Capacity", "Partcpnts", "Waste"}, widths);
        printHLine(widths);

        for (int eventIndex : order) {
            const Event& event = events_[eventIndex];
            int bestRoom = -1;

            for (int roomIndex : roomOrder) {
                if (!used[roomIndex] &&
                    rooms_[roomIndex].capacity >= event.participants) {
                    bestRoom = roomIndex;
                    break;
                }
            }

            if (bestRoom == -1) {
                printRow({
                    to_string(event.id),
                    event.title,
                    "NOT SCHEDULED - no suitable room available"
                }, {4, 18, 60});
                printHLine({4, 18, 60});
                continue;
            }

            used[bestRoom] = true;
            const Classroom& room = rooms_[bestRoom];
            const int waste = room.capacity - event.participants;

            printRow({
                to_string(event.id),
                event.title,
                "R" + to_string(room.id),
                room.building,
                to_string(room.capacity),
                to_string(event.participants),
                to_string(waste)
            }, widths);

            cout << " Reason: ";
            if (waste == 0) {
                cout << "perfect capacity fit.\n";
            } else {
                cout << "smallest available suitable room; "
                     << waste << " seats unused.\n";
            }

            printHLine(widths);
        }
    }
};

// -----------------------------------------------------------------------------
