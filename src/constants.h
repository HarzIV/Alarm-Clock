#ifndef CONSTANTS_H
#define CONSTANTS_H

// Includes
#include <Preferences.h>
#include <LedControl.h>
#include <unordered_map>
#include <vector>
#include <string>
#include "time.h"

Preferences alarmStorage;

LedControl display=LedControl(23, 18, 5, 2);

// WiFi
const char *ssidOWN = "Alarm Clock";  // SSID for the ESP32's own WiFi network
const char *passwordOWN = "12345678"; // Password for the ESP32's own WiFi network

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// Pins
const std::int8_t alarmPin = 15;

// Beep time
const std::int8_t beepTime = 100; // Time per beep in miliseconds

// All time zones
std::unordered_map<std::string, int> timeZoneOffsets = {
    {"IDLW", -12 * 3600}, // International Date Line West
    {"NUT", -11 * 3600},  // Niue Time
    {"SST", -11 * 3600},  // Samoa Standard Time
    {"HST", -10 * 3600},  // Hawaii Standard Time
    {"AKST", -9 * 3600},  // Alaska Standard Time
    {"PST", -8 * 3600},   // Pacific Standard Time
    {"MST", -7 * 3600},   // Mountain Standard Time
    {"CST", -6 * 3600},   // Central Standard Time
    {"EST", -5 * 3600},   // Eastern Standard Time
    {"AST", -4 * 3600},   // Atlantic Standard Time
    {"NST", -3.5 * 3600}, // Newfoundland Standard Time
    {"BRST", -3 * 3600},  // Brasilia Summer Time
    {"ART", -3 * 3600},   // Argentina Time
    {"UYT", -3 * 3600},   // Uruguay Time
    {"GST", 4 * 3600},    // Gulf Standard Time
    {"MSK", 3 * 3600},    // Moscow Standard Time
    {"EET", 2 * 3600},    // Eastern European Time
    {"CET", 1 * 3600},    // Central European Time
    {"GMT", 0},           // Greenwich Mean Time
    {"WET", 0},           // Western European Time
    {"IST", 5.5 * 3600},  // Indian Standard Time
    {"NPT", 5.75 * 3600}, // Nepal Time
    {"CST", 8 * 3600},    // China Standard Time
    {"JST", 9 * 3600},    // Japan Standard Time
    {"AEST", 10 * 3600},  // Australian Eastern Standard Time
    {"NZST", 12 * 3600}   // New Zealand Standard Time
    // Add more time zones as needed
};

std::unordered_map<char, std::vector<String>> characters = {
    {'0', {"111", "101", "101", "101", "111", "000", "000", "000"}},
    {'1', {"011", "101", "001", "001", "001", "000", "000", "000"}},
    {'2', {"010", "101", "001", "010", "111", "000", "000", "000"}},
    {'3', {"111", "001", "111", "001", "111", "000", "000", "000"}},
    {'4', {"101", "101", "111", "001", "001", "000", "000", "000"}},
    {'5', {"111", "100", "111", "001", "111", "000", "000", "000"}},
    {'6', {"111", "100", "111", "101", "111", "000", "000", "000"}},
    {'7', {"111", "001", "010", "010", "100", "000", "000", "000"}},
    {'8', {"111", "101", "111", "101", "111", "000", "000", "000"}},
    {'9', {"111", "101", "111", "001", "001", "000", "000", "000"}},
    {':', {"000", "010", "000", "010", "000", "000", "000", "000"}}
};

/**
 * @brief Displays time on both matrices
 * 
 * @param time 
 */
void displayTime(String time)
{
    std::unordered_map<int, String> columns = {
    {0, ""},
    {1, ""},
    {2, ""},
    {3, ""},
    {4, ""},
    {5, ""},
    {6, ""},
    {7, ""}
    };

    for (char ch : time)
    {
        std::vector<String> bitMap = characters.at(ch);

        int n = 7;
        for (size_t i = 0; i <= bitMap.size(); i++){
            columns[n] = columns[n] + bitMap[i];
            n--;
        }
    }

    for (int i = 0; i <= 7; i++) {
        columns[i] = columns[i] + "0";
    }

    for (int i = 0; i <= 7; i++){
        String display1 = columns[i].substring(0, 8);
        String display2 = columns[i].substring(8);
        Serial.print(display1);
        Serial.print("|");
        Serial.println(display2);

        int intDisplay1 = 0;
        for (char digit : display1) {
          // Shift the existing value left by 1 bit (multiply by 2) and add the new digit
          intDisplay1 = (intDisplay1 << 1) | (digit - '0');
        }

        int intDisplay2 = 0;
        for (char digit : display2) {
          // Shift the existing value left by 1 bit (multiply by 2) and add the new digit
          intDisplay2 = (intDisplay2 << 1) | (digit - '0');
        }

        display.setColumn(0, i, intDisplay1);
        display.setColumn(1, i, intDisplay2);
    }
}

// Functions
String combineAlarms(String words[], int size)
{
    String result = "";
    for (int i = 0; i < size; i++)
    {
        result += words[i];
        if (i < size - 1)
        {
            result += ",";
        }
    }
    return result;
}

/**
 * @brief Splits string of times into array.
 * 
 * @param input 
 * @param size 
 * @return String* 
 */
String *splitTimes(const String &input, int& size)
{
    // Count the number of commas to determine the array size
    int count = 1; // There is always one more element than commas
    for (int i = 0; i < input.length(); i++)
    {
        if (input[i] == ',')
        {
            count++;
        }
    }

    // Allocate the array
    String *times = new String[count];
    size = count;

    // Split the string
    int start = 0;
    int index = 0;
    for (int i = 0; i < input.length(); i++)
    {
        if (input[i] == ',')
        {
            times[index++] = input.substring(start, i);
            start = i + 1;
        }
    }
    // Add the last element
    times[index] = input.substring(start);

    return times;
}

/**
 * @brief Saves alarms in NVS
 *
 * @param allAlarms
 */
void saveAlarms(String allAlarms)
{
    alarmStorage.begin("alarms", false);
    alarmStorage.putString("alarmString", allAlarms);
    alarmStorage.end();
}

/**
 * Retrieves alarms from NVS.
 *
 * @return String(11:11,22:22)
 */
String retrieveAlarms()
{
    alarmStorage.begin("alarms", true);
    String alarms = alarmStorage.getString("alarmString", "");
    alarmStorage.end();

    return alarms;
}

/**
 * Checks if current time is a set alarm.
 *
 * This function returns true if the current time
 * matches any of the time in the array, else it
 * returns false.
 *
 * @param parameter1 String of the current time(22:22)
 * @param parameter2 String of alarms(11:11,22:22)
 * @return bool
 */
bool soundAlarm(String currentTime, String alarmString)
{
    int arraySize;
    String *alarmArray = splitTimes(alarmString, arraySize);

    for (int i = 0; i < arraySize; i++)
    {
        String alarm = alarmArray[i];

        if (alarm == currentTime)
        {
            delete[] alarmArray;
            return true;
        }
    }

    delete[] alarmArray;
    return false;
}

/**
 * @brief Get the current time
 *
 * @return String(22:22)
 */
String getTime()
{
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
    {
        return "22:22";
    }

    String hourString;
    String minuteString;

    int hourInt = timeInfo.tm_hour;
    int minuteInt = timeInfo.tm_min;
    if (hourInt < 10) {
        hourString = ('0'+std::to_string(hourInt)).c_str();
    } else {
        hourString = (std::to_string(hourInt)).c_str();
    }

    if (minuteInt < 10) {
        minuteString = ('0'+std::to_string(minuteInt)).c_str();
    } else {
        minuteString = (std::to_string(minuteInt)).c_str();
    }

    String currentTime = hourString + ":" + minuteString;

    return currentTime;
}

#endif