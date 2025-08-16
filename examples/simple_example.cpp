/**
 * @file simple_example.cpp
 * @brief simple_example.h 的实现
 */

#include "simple_example.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace simple {

// ================================
// 全局函数实现
// ================================

int add(int a, int b) {
    return a + b;
}

std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

bool isEven(int number) {
    return number % 2 == 0;
}

// ================================
// Calculator 类实现
// ================================

Calculator::Calculator() : value_(0.0) {
}

Calculator::Calculator(double initial_value) : value_(initial_value) {
}

double Calculator::getValue() const {
    return value_;
}

void Calculator::setValue(double value) {
    value_ = value;
}

double Calculator::add(double x) {
    value_ += x;
    return value_;
}

double Calculator::subtract(double x) {
    value_ -= x;
    return value_;
}

double Calculator::multiply(double x) {
    value_ *= x;
    return value_;
}

double Calculator::divide(double x) {
    if (x != 0.0) {
        value_ /= x;
    } else {
        std::cout << "Warning: Division by zero!" << std::endl;
    }
    return value_;
}

void Calculator::reset() {
    value_ = 0.0;
}

void Calculator::clear() {
    value_ = 0.0;
}

double Calculator::pi() {
    return 3.141592653589793;
}

double Calculator::e() {
    return 2.718281828459045;
}

// ================================
// DataContainer 类实现
// ================================

DataContainer::DataContainer() : text_("Empty") {
}

std::string DataContainer::getText() const {
    return text_;
}

void DataContainer::setText(const std::string& text) {
    text_ = text;
}

std::vector<int> DataContainer::getNumbers() const {
    return numbers_;
}

void DataContainer::addNumber(int number) {
    numbers_.push_back(number);
}

void DataContainer::clearNumbers() {
    numbers_.clear();
}

int DataContainer::getNumberCount() const {
    return static_cast<int>(numbers_.size());
}

int DataContainer::getNumberAt(int index) const {
    if (index >= 0 && index < static_cast<int>(numbers_.size())) {
        return numbers_[index];
    }
    return 0; // 默认值
}

std::string DataContainer::toString() const {
    std::ostringstream oss;
    oss << "DataContainer[text=\"" << text_ << "\", numbers=[";
    for (size_t i = 0; i < numbers_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << numbers_[i];
    }
    oss << "]]";
    return oss.str();
}

// ================================
// StringUtils 类实现（静态类）
// ================================

std::string StringUtils::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string StringUtils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::reverse(const std::string& str) {
    std::string result = str;
    std::reverse(result.begin(), result.end());
    return result;
}

int StringUtils::length(const std::string& str) {
    return static_cast<int>(str.length());
}

bool StringUtils::contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    return str.substr(0, prefix.length()) == prefix;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            oss << separator;
        }
        oss << parts[i];
    }
    
    return oss.str();
}

} // namespace simple