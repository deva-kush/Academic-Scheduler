#include "../include/utilities.hpp"

string trim(const string& value) {
    size_t first = 0;
    while (first < value.size() &&
           isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }

    size_t last = value.size();
    while (last > first &&
           isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return value.substr(first, last - first);
}

// Handles basic CSV fields and quoted commas.
vector<string> splitCSV(const string& line) {
    vector<string> result;
    string field;
    bool quoted = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];

        if (ch == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (ch == ',' && !quoted) {
            result.push_back(trim(field));
            field.clear();
        } else {
            field += ch;
        }
    }

    result.push_back(trim(field));
    return result;
}

bool isInteger(const string& value) {
    if (value.empty()) return false;

    size_t i = (value[0] == '-' || value[0] == '+') ? 1 : 0;
    if (i == value.size()) return false;

    for (; i < value.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(value[i]))) return false;
    }
    return true;
}

bool tryParseInt(const string& value, int& output) {
    if (!isInteger(value)) return false;

    try {
        size_t used = 0;
        long long parsed = stoll(value, &used);
        if (used != value.size() ||
            parsed < numeric_limits<int>::min() ||
            parsed > numeric_limits<int>::max()) {
            return false;
        }
        output = static_cast<int>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

bool tryParseDouble(const string& value, double& output) {
    try {
        size_t used = 0;
        output = stod(value, &used);
        return used == value.size();
    } catch (...) {
        return false;
    }
}

string col(const string& value, int width) {
    if (static_cast<int>(value.size()) >= width) {
        return value.substr(0, width);
    }
    return value + string(width - static_cast<int>(value.size()), ' ');
}

void printHLine(const vector<int>& widths) {
    for (int width : widths) {
        cout << string(width + 2, '-');
    }
    cout << '\n';
}

void printRow(const vector<string>& cells, const vector<int>& widths) {
    const size_t count = min(cells.size(), widths.size());
    for (size_t i = 0; i < count; ++i) {
        cout << ' ' << col(cells[i], widths[i]) << ' ';
    }
    cout << '\n';
}

// -----------------------------------------------------------------------------
