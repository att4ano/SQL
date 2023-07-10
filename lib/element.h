#include <iostream>
#include <vector>
#include <unordered_map>

#include "parameter.h"

class Element {
public:
    using Row = std::unordered_map<std::string, Parameter>;
private:
    std::string primary_parameter_;
    Row parameters_;
    std::unordered_map<std::string, TYPE> parameter_list_;
    std::vector<std::string> order_;
public:
    Element() = default;

    Element(const std::string& primary_parameter, const Row& parameters) :primary_parameter_(primary_parameter),
                                                                       parameters_(parameters) {}

    explicit Element(const Row& parameters) : parameters_(parameters) {}

    void SetKeyParameter(const std::string& primary_key) {
        primary_parameter_ = primary_key;
    }

    Parameter& operator[] (const std::string& key) {
        return parameters_[key];
    }

    std::unordered_map<std::string, TYPE>& GetParameterList() {
        return parameter_list_;
    }

    Row& GetParameters() {
        return parameters_;
    }

    TYPE GetCurrentParameter(const std::string& name) {
        for (const auto& i : parameter_list_) {
            if (i.first == name) {
                return i.second;
            }
        }
        return TYPE::NONE;
    }

    std::vector<std::string>& GetOrder() {
        return order_;
    }

};
