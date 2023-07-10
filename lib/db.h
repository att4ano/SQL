#pragma once

#include "table.h"
#include <regex>
#include <unordered_set>

static std::unordered_set<std::string> types{"INT", "BOOL", "FLOAT", "DOUBLE", "VARCHAR"};

class DataBase {
private:
    std::string name_;
    std::unordered_map<std::string, Table> tables_;
    std::vector<Connection> connections_;

    void Select(const std::unordered_set<std::string>& columns, const std::string& table_name);

    void SelectWithWhere(const std::unordered_set<std::string>& columns, const std::string& table_name,
                         std::string& where);

    void SelectWithWhereAndJoin(const std::vector<std::pair<std::string, std::string>>& columns_list,
                                     const std::string& left_table, const std::string& right_table,
                                     const std::string& general_column,
                                     std::string& where_conditional, const std::string& type);

    void SelectWithJoin(const std::vector<std::pair<std::string, std::string>>& columns_list,
                             const std::string& left_table, const std::string& right_table,
                             const std::string& general_column, const std::string& type);

    void Delete(const std::string& table_name);

    void DeleteWithWhere(const std::string& table_name, std::string& where_condition);

    void UpdateWithWhere(const std::string& table_name, const std::string& new_values, std::string& where_condition);

    void Update(const std::string& table_name, const std::string& new_values);

    std::pair<std::string, Parameter> SetValue(const std::string& table_name, const std::string& input);

    static bool JoinPredicate(Element& first_element, const std::string& first_column, Element& second_element,
                       const std::string& sign);

public:

    DataBase() = default;

    explicit DataBase(const std::string& name) : name_(name) {}

    [[nodiscard]] size_t Size() const noexcept {
        return tables_.size();
    }

    void CreateTable(const std::string& request);

    void DropTable(const std::string& request);

    void Insert(const std::string& request);

    void SelectRequest(const std::string& request);

    void DeleteRequest(const std::string& request);

    void UpdateRequest(const std::string& request);

    std::unordered_map<std::string, Table>& GetTables() {
        return tables_;
    }

};