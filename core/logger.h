//
// Created by shiyin on 2023/12/26.
//

#pragma once

#ifndef SIMPLERENDERER_LOGGER_H
#define SIMPLERENDERER_LOGGER_H

#include <string>
#include <iostream>
#include <chrono>

class Logger{
public:

    enum LogType{
        MESSAGE,
        ERROR
    };

    template<class T>
    static void Log(const std::string &methodName, const std::string& message, LogType type = MESSAGE){
        std::string currentTime = GetFormatTime();
        std::string className = GetTypeName(typeid(T).name());
        if (type == MESSAGE)
            std::cout << "[" << currentTime << "] " << "[" << className << "::" << methodName << "] " << "[MESSAGE] " << message << std::endl;
        else
            std::cout << "[" << currentTime << "] " << "[" << className << "::" << methodName << "] " << "[ERROR ]" << message << std::endl;
    }

private:

    static std::string GetFormatTime(){
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
        std::string timeString = std::ctime(&currentTime);
        timeString = timeString.substr(0,timeString.size() - 1);
        return timeString;
    }

    static std::string GetTypeName(const char* fullName){
        std::string name(fullName);
        std::size_t pos = name.find_first_not_of("0123456789");
        if (pos != std::string::npos) {
            name = name.substr(pos);
        }
        return name;
    }

};

#endif //SIMPLERENDERER_LOGGER_H
