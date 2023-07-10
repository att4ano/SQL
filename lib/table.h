#include "element.h"
#include "unordered_set"

class Table;

class Connection {
private:
    std::pair<std::string, std::string> link_;
    std::string foreign_key_;
public:

    Connection() = default;

    Connection(const std::pair<std::string, std::string>& link, const std::string& foreign_key) :
    foreign_key_(foreign_key), link_(link) {}

    std::string& GetKey() {
        return foreign_key_;
    }

    std::pair<std::string, std::string>& GetLink() {
        return link_;
    }
};

class Table {
private:
    std::string primary_key_;
    std::unordered_set<std::string> null_parameters;
    std::vector<Element> elements_;
    std::unordered_map<std::string, TYPE> parameter_list_;
    std::vector <std::string> parameters_;
public:

    Table() = default;

    explicit Table(const std::string& primary_key, const std::unordered_map<std::string, TYPE>& parameter_list) :
    primary_key_(primary_key), parameter_list_(parameter_list) {}

    explicit Table(const std::unordered_map<std::string, TYPE>& parameter_list) :
    parameter_list_(parameter_list) {}

    std::string& GetPrimary() {
        return primary_key_;
    }

    std::unordered_set<std::string>& GetNull() {
        return null_parameters;
    }

    std::unordered_map<std::string, TYPE>& GetParameters() {
        return parameter_list_;
    }

    std::vector<Element>& GetElement() {
        return elements_;
    }

    std::vector<std::string>& GetOrder() {
        return parameters_;
    }

};
