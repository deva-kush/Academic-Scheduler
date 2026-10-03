#pragma once
#include "common.hpp"

string trim(const string& value);
vector<string> splitCSV(const string& line);
bool isInteger(const string& value);
bool tryParseInt(const string& value, int& output);
bool tryParseDouble(const string& value, double& output);
string col(const string& value, int width);
void printHLine(const vector<int>& widths);
void printRow(const vector<string>& cells, const vector<int>& widths);