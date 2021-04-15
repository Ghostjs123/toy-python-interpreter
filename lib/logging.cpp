#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "logging.h"

using std::cout;
using std::endl;

Logger* Logger::logger = nullptr;

Logger::Logger() {
    cout << "Logger set to '" << "output_log" << "'" << endl;
    this->f.open("output_log", std::fstream::out | std::fstream::trunc); 
}

Logger::Logger(std::string fname) {
    cout << "Logger set to '" << fname << "'" << endl;
    this->f.open(fname, std::fstream::out | std::fstream::trunc);
}

Logger* Logger::get_instance() {
    if(logger == nullptr){
        logger = new Logger();
    }
    return logger;
}

Logger* Logger::get_instance(const std::string& fname) {
    if(logger == nullptr){
        logger = new Logger(fname);
    }
    return logger;
}

Logger::~Logger() {
    this->close();
}

std::string Logger::get_mode_string() {
    if (this->mode == DEBUG) return "DEBUG";
    if (this->mode == INFO) return "INFO";
    if (this->mode == WARNING) return "WARNING";
    throw std::runtime_error("logger - unknown mode: " + this->mode);
}

void Logger::log(std::string msg, int mode) {
    // cout << msg << endl;
    if (this->f.is_open()) this->f << msg << '\n';
}
    
void Logger::set_mode(int mode) {
    if (mode == 1 || mode == 2 || mode == 3) this->mode = mode;
}

void Logger::close() {
    if (this->f.is_open()) {
        this->f.close();
    }
}
