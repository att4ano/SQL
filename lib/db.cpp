#include "db.h"
#include <sstream>

struct Column {
    bool is_null = false;
    bool is_primary = false;
    bool is_foreign = false;
    std::string table_name;
    std::string column_name;
    TYPE type = TYPE::NONE;
};

bool IsDigit(const std::string& str) {
    std::regex numeric_regex(R"(^[-]?\d+(\.\d+)?$)");
    return std::regex_match(str, numeric_regex);
}

bool IsString(const std::string& input) {
    std::regex regex(R"(^(['"])\w*(['"]))");
    return std::regex_match(input, regex);
}

bool IsBool(const std::string& input) {
    return input == "true" || input == "false";
}

std::string RemoveQuotes(const std::string& str) {
    std::regex quotesRegex(R"(["'])");
    return std::regex_replace(str, quotesRegex, "");
}

template<typename T>
T CastFromString(const std::string& string) {
    T value;
    std::istringstream iss(string);
    if (iss >> value) {
        return value;
    } else {
        throw std::logic_error("Bad cast");
    }
}

template<>
bool CastFromString<bool>(const std::string& string) {
    bool value;
    std::istringstream iss(string);
    if (iss >> std::boolalpha >> value) {
        return value;
    } else {
        throw std::logic_error("Bad cast");
    }
}

Column ParseColumn(const std::string& request) {
    Column column;
    std::regex regex(
            R"(\s*(\w+)\s*(INT|FLOAT|VARCHAR|DOUBLE|BOOL)?(\(\d+\))?\s*\s*(PRIMARY KEY)?\s*(NOT NULL)?\s*((FOREIGN KEY) REFERENCES(\((\w+)\)))?\s*)");
    std::smatch match;
    if (std::regex_match(request, match, regex)) {
        column.column_name = match[1].str();
        std::string type_ = match[2].str();
        if (type_ == "INT") {
            column.type = TYPE::INT;
        } else if (type_ == "BOOL") {
            column.type = TYPE::BOOL;
        } else if (type_ == "FLOAT") {
            column.type = TYPE::FLOAT;
        } else if (type_ == "DOUBLE") {
            column.type = TYPE::DOUBLE;
        } else if (type_ == "VARCHAR") {
            column.type = TYPE::STRING;
        } else {
            throw std::runtime_error("Syntax error");
        }
        if (!match[4].str().empty()) {
            column.is_primary = true;
        } else if (!match[5].str().empty()) {
            column.is_null = true;
        } else if (!match[7].str().empty()) {
            column.is_foreign = true;
            std::smatch other_match;
            column.table_name = match[9].str();
        }
        return column;
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void SplitString(const std::string& input, std::vector<std::string>& result) {
    std::regex split_element(R"(\s*,\s*)");
    std::sregex_token_iterator iter(input.begin(), input.end(), split_element, -1);
    std::sregex_token_iterator end;
    while (iter != end) {
        result.push_back(*iter);
        ++iter;
    }
}

std::string ParsePredicate(std::vector<std::string>& predicate, Element& element) {
    if (predicate.size() == 1) {
        return predicate[0];
    } else {
        if (element.GetParameters().find(predicate[0]) == element.GetParameters().end()) {
            if (!IsDigit(predicate[0]) && !IsString(predicate[0])) {
                throw std::logic_error("This parameter does not exist");
            }
        }
        if (element.GetParameters().find(predicate[2]) == element.GetParameters().end()) {
            if (!IsDigit(predicate[2]) && !IsString(predicate[2])) {
                throw std::logic_error("This parameter does not exist");
            }
        }

        Parameter first_parameter;
        bool is_first_parameter;
        Parameter second_parameter;
        bool is_second_parameter;

        is_first_parameter = !IsDigit(predicate[0]) && !IsString(predicate[0]) && !IsBool(predicate[0]);
        is_second_parameter = !IsDigit(predicate[2]) && !IsString(predicate[2]) && !IsBool(predicate[2]);

        if (is_first_parameter && is_second_parameter) {
            first_parameter = element[predicate[0]];
            second_parameter = element[predicate[2]];

            if (first_parameter.Type() != second_parameter.Type()) {
                throw std::logic_error("Different types of parameters");
            }
        }

        if (is_first_parameter && !is_second_parameter) {
            first_parameter = element[predicate[0]];

            if (IsDigit(predicate[2])) {
                if (first_parameter.Type() == TYPE::INT) {
                    second_parameter = std::stoi(predicate[2]);
                } else if (first_parameter.Type() == TYPE::FLOAT) {
                    second_parameter = std::stof(predicate[2]);
                } else if (first_parameter.Type() == TYPE::DOUBLE) {
                    second_parameter = std::stod(predicate[2]);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsString(predicate[2])) {
                if (first_parameter.Type() == TYPE::STRING) {
                    second_parameter = RemoveQuotes(predicate[2]);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsBool(predicate[2])) {
                if (first_parameter.Type() == TYPE::BOOL) {
                    second_parameter = (predicate[2] == "true");
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            }
        }

        if (!is_first_parameter && is_second_parameter) {
            second_parameter = element[predicate[2]];

            if (IsDigit(predicate[0])) {
                if (second_parameter.Type() == TYPE::INT) {
                    first_parameter = std::stoi(predicate[0]);
                } else if (second_parameter.Type() == TYPE::FLOAT) {
                    first_parameter = std::stof(predicate[0]);
                } else if (second_parameter.Type() == TYPE::DOUBLE) {
                    first_parameter = std::stod(predicate[0]);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsString(predicate[0])) {
                if (second_parameter.Type() == TYPE::STRING) {
                    first_parameter = RemoveQuotes(predicate[0]);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsBool(predicate[0])) {
                if (second_parameter.Type() == TYPE::BOOL) {
                    first_parameter = (predicate[0] == "true");
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            }
        }

        if (!is_first_parameter && !is_second_parameter) {
            if (IsDigit(predicate[0]) && IsDigit(predicate[2])) {
                first_parameter = std::stod(predicate[0]);
                second_parameter = std::stod(predicate[2]);
            } else if (IsString(predicate[0]) && IsString(predicate[2])) {
                first_parameter = RemoveQuotes(predicate[0]);
                second_parameter = RemoveQuotes(predicate[2]);
            } else if (IsBool(predicate[0]) && IsBool(predicate[0])) {
                first_parameter = (predicate[0] == "true");
                second_parameter = (predicate[2] == "true");
            } else {
                throw std::logic_error("Different types of parameters");
            }
        }

        if (predicate[1] == ">") {
            return (first_parameter > second_parameter ? "true" : "false");
        } else if (predicate[1] == "=") {
            std::string flag = (first_parameter == second_parameter ? "true" : "false");
            return (first_parameter == second_parameter ? "true" : "false");
        } else if (predicate[1] == "<") {
            bool flag = first_parameter < second_parameter;
            return (first_parameter < second_parameter ? "true" : "false");
        } else {
            throw std::logic_error("Such symbol does not exist");
        }
    }
}

std::vector<std::string> ParseComplexLogic(std::string& string) noexcept {
    std::regex regex(R"((\b\w+\b)|([()])|(>=|<=|<|>|=)|(\"([^\"]*)\")|(OR|AND))");
    std::smatch match;
    std::vector<std::string> elements;
    std::string tmp = string;
    while (std::regex_search(tmp, match, regex)) {
        elements.push_back(match.str(0));
        tmp = match.suffix();
    }
    return elements;
}

std::string ParseLogicExpression(const std::vector<std::string>& predicates, Element& element) {
    std::vector<std::string> current_predicate;
    std::vector<std::string> new_expression;
    std::stack<std::string> stack;
    for (const auto& i: predicates) {
        if (i != "AND" && i != "OR") {
            current_predicate.push_back(i);
        } else {
            new_expression.push_back(ParsePredicate(current_predicate, element));
            new_expression.push_back(i);
            current_predicate.clear();
        }
    }
    new_expression.push_back(ParsePredicate(current_predicate, element));
    bool is = false;
    current_predicate.clear();
    for (const auto& i: new_expression) {
        if (!is && i != "AND") {
            current_predicate.push_back(i);
        } else if (i == "AND" && !is) {
            is = true;
        } else {
            if (!is) {
                current_predicate.push_back(i);
            } else {
                if (!(current_predicate.back() == "true" && i == "true")) {
                    current_predicate.pop_back();
                    current_predicate.emplace_back("false");
                }
                is = false;
            }
        }
    }
    new_expression.clear();
    for (const auto& i: current_predicate) {
        if (new_expression.empty()) {
            new_expression.push_back(i);
        } else if (i != "OR") {
            if (!(i == "false" && new_expression.back() == "false")) {
                new_expression.pop_back();
                new_expression.emplace_back("true");
            }
        }
    }
    return new_expression[0];
}

std::string ExecuteLogicExpression(const std::vector<std::string>& elements, Element& element) {
    std::vector<std::string> main_part;
    std::vector<std::string> string_quotes;
    std::stack<std::string> stack;
    bool is_open = false;
    for (const auto& i: elements) {
        if (!is_open && (i != "(" && i != ")")) {
            main_part.push_back(i);
        } else if (i == "(" && !is_open) {
            is_open = true;
        } else if (!is_open && i == ")") {
            throw std::runtime_error("Syntax error");
        } else if (is_open) {
            if (i == "(") {
                stack.push(i);
                string_quotes.push_back(i);
            } else if (i == ")") {
                if (!stack.empty()) {
                    stack.pop();
                    string_quotes.push_back(i);
                } else {
                    main_part.emplace_back(ExecuteLogicExpression(string_quotes, element));
                    string_quotes.clear();
                    is_open = false;
                }
            } else {
                string_quotes.push_back(i);
            }
        }
    }
    return ParseLogicExpression(main_part, element);
}

void DataBase::CreateTable(const std::string& request) {
    std::regex regex(R"(CREATE\s+TABLE\s+(\w+)\s+\((.*?)\);)");
    std::regex regex_reference(R"((\w+)(\(\s*(\w+)\s*\)))");
    std::smatch match;
    std::smatch new_match;
    if (std::regex_match(request, match, regex)) {
        std::string table_name = match[1].str();
        std::string table_content = match[2].str();
        tables_[table_name] = Table();
        std::vector<std::string> columns;
        SplitString(table_content, columns);
        for (auto& i: columns) {
            Column column = ParseColumn(i);
            if (column.is_primary) {
                tables_[table_name].GetPrimary() = column.column_name;
            }
            if (column.is_null) {
                tables_[table_name].GetNull().insert(column.column_name);
            }
            tables_[table_name].GetParameters()[column.column_name] = column.type;
            tables_[table_name].GetOrder().push_back(column.column_name);
            if (column.is_foreign) {
                if (tables_.find(column.table_name) != tables_.end()) {
                    if (std::regex_match(column.column_name, new_match, regex_reference)) {
                        connections_.emplace_back(std::make_pair(table_name, match[1]), match[3]);
                    }
                }
            }
        }
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void DataBase::DropTable(const std::string& input) {
    std::regex drop_table_regex(R"(^\s*DROP\s+TABLE\s+(\w+)\s*;?\s*$)");
    std::smatch match;
    if (std::regex_match(input, match, drop_table_regex)) {
        std::string table_name = match[1].str();
        tables_.erase(table_name);
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void DataBase::Insert(const std::string& request) {
    std::regex pattern(R"(\bINSERT\s+INTO\s+(\w+)\s*(\(([\w\s,]+)\))?\s+VALUES\s*\(([^;]+)\);)");
    std::smatch match;
    bool all_columns = false;
    if (std::regex_match(request, match, pattern)) {
        std::string table_name = match[1].str();
        std::string column_names = match[3].str();
        std::string values = match[4].str();
        std::vector<std::string> vector_columns;
        all_columns = column_names.empty();
        if (!all_columns) {
            SplitString(column_names, vector_columns);
        } else {
            for (const auto& i: tables_[table_name].GetOrder()) {
                vector_columns.push_back(i);
            }
        }
        std::vector<std::string> vector_values;
        SplitString(values, vector_values);
        if (tables_.find(table_name) == tables_.end()) {
            throw std::logic_error("This table does not exist");
        }

        if (!all_columns) {
            for (auto& i: vector_columns) {
                if (tables_[table_name].GetParameters().find(i) == tables_[table_name].GetParameters().end()) {
                    throw std::logic_error("This parameter does not exist in this table");
                }
            }
        }

        if (!all_columns) {
            for (auto& i: tables_[table_name].GetNull()) {
                if (std::find(vector_columns.begin(), vector_columns.end(), i) == vector_columns.end()) {
                    throw std::logic_error("NOT NULL parameters is not in parameter list");
                }
            }
        }

        Element element = Element();
        element.GetParameterList() = tables_[table_name].GetParameters();
        element.GetOrder() = tables_[table_name].GetOrder();
        for (auto& i: element.GetParameterList()) {
            element[i.first];
        }

        for (size_t i = 0; i < vector_columns.size(); ++i) {
            if (tables_[table_name].GetParameters()[vector_columns[i]] == TYPE::INT) {
                element[vector_columns[i]] = CastFromString<int>(vector_values[i]);
                element[vector_columns[i]].Type() = TYPE::INT;
            } else if (tables_[table_name].GetParameters()[vector_columns[i]] == TYPE::BOOL) {
                element[vector_columns[i]] = CastFromString<bool>(vector_values[i]);
                element[vector_columns[i]].Type() = TYPE::BOOL;
            } else if (tables_[table_name].GetParameters()[vector_columns[i]] == TYPE::FLOAT) {
                element[vector_columns[i]] = CastFromString<float>(vector_values[i]);
                element[vector_columns[i]].Type() = TYPE::FLOAT;
            } else if (tables_[table_name].GetParameters()[vector_columns[i]] == TYPE::DOUBLE) {
                element[vector_columns[i]] = CastFromString<double>(vector_values[i]);
                element[vector_columns[i]].Type() = TYPE::DOUBLE;
            } else if (tables_[table_name].GetParameters()[vector_columns[i]] == TYPE::STRING) {
                element[vector_columns[i]] = RemoveQuotes(vector_values[i]);
                element[vector_columns[i]].Type() = TYPE::STRING;
            }
        }
        tables_[table_name].GetElement().push_back(element);
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void DataBase::SelectRequest(const std::string& request) {

    std::regex regex_select_pattern1(R"(SELECT\s+(.*?)\s+FROM\s+(.*?);)");
    std::regex regex_select_pattern2(
            R"(SELECT\s+(.*?)\s+FROM\s+(.*?)\s*(LEFT|RIGHT|INNER)?\s+JOIN\s+(.*?)\s+ON\s+(.*?);)");
    std::regex regex_select_pattern3(R"(SELECT\s+(.*?)\s+FROM\s+(.*?)\s+WHERE\s+(.*?);)");
    std::regex regex_select_pattern4(
            R"(SELECT\s+(.*?)\s+FROM\s+(.*?)\s*(LEFT|RIGHT|INNER)?\s+JOIN\s+(.*?)\s+ON\s+(.*?)\s+WHERE\s+(.*?);)");

    std::smatch match;

    if (std::regex_match(request, match, regex_select_pattern4)) {
        std::string columns = match[1].str();
        std::string type_join = match[3].str();
        std::string left_table = match[2].str();
        std::string right_table = match[4].str();
        std::string on_conditional = match[5].str();
        std::string where_conditional = match[6].str();

        std::vector<std::pair<std::string, std::string>> tables_with_columns;
        std::vector<std::string> all_column;
        SplitString(columns, all_column);
        std::regex separator(R"((\w+)\.(\w+))");
        std::smatch tmp;
        for (const auto& i: all_column) {
            if (std::regex_match(i, tmp, separator)) {
                std::string f = tmp[1];
                std::string s = tmp[2];
                if (tables_.find(tmp[1]) != tables_.end() &&
                    tables_[tmp[1]].GetParameters().find(tmp[2]) != tables_[tmp[1]].GetParameters().end()) {
                    tables_with_columns.emplace_back(tmp[1], tmp[2]);
                } else {
                    throw std::logic_error("Table error");
                }
            }
        }

        SelectWithWhereAndJoin(tables_with_columns, left_table, right_table, on_conditional,
                               where_conditional, type_join);

    } else if (std::regex_match(request, match, regex_select_pattern3)) {
        std::string columns = match[1].str();
        std::string table1 = match[2].str();
        std::string where_condition = match[3];
        std::unordered_set<std::string> select_set;
        std::vector<std::string> tmp;
        SplitString(columns, tmp);
        for (const auto& i: tmp) {
            select_set.insert(i);
        }
        SelectWithWhere(select_set, table1, where_condition);
    } else if (std::regex_match(request, match, regex_select_pattern2)) {
        std::string columns = match[1].str();
        std::string type_join = match[3].str();
        std::string left_table = match[2].str();
        std::string right_table = match[4].str();
        std::string on_conditional = match[5].str();

        std::vector<std::pair<std::string, std::string>> tables_with_columns;
        std::vector<std::string> all_column;
        SplitString(columns, all_column);
        std::regex separator(R"((\w+)\.(\w+))");
        std::smatch tmp;

        for (const auto& i: all_column) {
            if (std::regex_match(i, tmp, separator)) {
                std::string f = tmp[1];
                std::string s = tmp[2];
                if (tables_.find(tmp[1]) != tables_.end() &&
                    tables_[tmp[1]].GetParameters().find(tmp[2]) != tables_[tmp[1]].GetParameters().end()) {
                    tables_with_columns.emplace_back(tmp[1], tmp[2]);
                } else {
                    throw std::logic_error("Table error");
                }
            }
        }

        SelectWithJoin(tables_with_columns, left_table, right_table, on_conditional, type_join);

    } else if (std::regex_match(request, match, regex_select_pattern1)) {
        std::string columns = match[1].str();
        std::string table1 = match[2].str();
        std::unordered_set<std::string> select_set;
        std::vector<std::string> tmp;
        SplitString(columns, tmp);
        for (const auto& i: tmp) {
            select_set.insert(i);
        }
        Select(select_set, table1);
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void DataBase::Select(const std::unordered_set<std::string>& columns, const std::string& table_name) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    bool all_columns = false;
    if (columns.size() == 1 && *columns.begin() == "*") {
        all_columns = true;
    }
    for (auto& i: tables_[table_name].GetElement()) {
        for (auto& parameter: i.GetOrder()) {
            if (all_columns) {
                i[parameter].Print();
                std::cout << " ";
            } else {
                if (columns.find(parameter) != columns.end()) {
                    i[parameter].Print();
                    std::cout << " ";
                }
            }
        }
        std::cout << "\n";
    }
}

void DataBase::SelectWithWhere(const std::unordered_set<std::string>& columns, const std::string& table_name,
                               std::string& where_condition) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    bool all_columns = false;
    if (columns.size() == 1 && *columns.begin() == "*") {
        all_columns = true;
    }
    for (auto& i: tables_[table_name].GetElement()) {
        if (CastFromString<bool>(ExecuteLogicExpression(ParseComplexLogic(where_condition), i))) {
            for (auto& parameter: i.GetOrder()) {
                if (all_columns) {
                    i[parameter].Print();
                    std::cout << " ";
                } else {
                    if (columns.find(parameter) != columns.end()) {
                        i[parameter].Print();
                        std::cout << " ";
                    }
                }
            }
            std::cout << "\n";
        }
    }
}

void DataBase::DeleteRequest(const std::string& request) {
    std::regex regex_delete_pattern1(R"(DELETE\s+FROM\s+(\w+)\s+WHERE\s+(.*);)");
    std::regex regex_delete_pattern2(R"(DELETE\s+FROM\s+(\w+)\s*;)");

    std::smatch match;

    if (std::regex_match(request, match, regex_delete_pattern1)) {
        std::string table_name = match[1];
        std::string where_condition = match[2];

        DeleteWithWhere(table_name, where_condition);
    } else if (std::regex_match(request, match, regex_delete_pattern2)) {
        std::string table_name = match[1];

        Delete(table_name);
    } else {
        throw std::runtime_error("Syntax error");
    }

}

void DataBase::Delete(const std::string& table_name) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    tables_[table_name].GetElement().clear();
}

void DataBase::DeleteWithWhere(const std::string& table_name, std::string& where_condition) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    for (auto i = tables_[table_name].GetElement().begin(); i != tables_[table_name].GetElement().end();) {
        if (CastFromString<bool>(ExecuteLogicExpression(ParseComplexLogic(where_condition), *i))) {
            i = tables_[table_name].GetElement().erase(i);
        } else {
            ++i;
        }
    }
}

void DataBase::UpdateRequest(const std::string& request) {
    std::regex update_regex_pattern1(R"(\s*UPDATE\s+(\w+)\s+SET\s+(.*)\s+WHERE\s+(.*);)");
    std::regex update_regex_pattern2(R"(\s*UPDATE\s+(\w+)\s+SET\s+(.*);)");

    std::smatch match;

    if (std::regex_match(request, match, update_regex_pattern1)) {
        std::string table_name = match[1];
        std::string set_values = match[2];
        std::string where_condition = match[3];

        UpdateWithWhere(table_name, set_values, where_condition);
    } else if (std::regex_match(request, match, update_regex_pattern2)) {
        std::string table_name = match[1];
        std::string set_values = match[2];

        Update(table_name, set_values);
    }

}

std::pair<std::string, Parameter> DataBase::SetValue(const std::string& table_name, const std::string& input) {
    std::regex regex(R"(\s*(\w+)\s*=\s*([",']?[\w,\s]+[",']?))");
    std::smatch match;
    Parameter parameter_value;
    if (std::regex_match(input, match, regex)) {

        std::string name = match[1];
        std::string value = match[2];

        if (tables_[table_name].GetParameters().find(name) != tables_[table_name].GetParameters().end()) {
            if (IsDigit(value)) {
                if (tables_[table_name].GetParameters()[name] == TYPE::INT) {
                    parameter_value = std::stoi(value);
                } else if (tables_[table_name].GetParameters()[name] == TYPE::FLOAT) {
                    parameter_value = std::stof(value);
                } else if (tables_[table_name].GetParameters()[name] == TYPE::DOUBLE) {
                    parameter_value = std::stod(value);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsString(value)) {
                if (tables_[table_name].GetParameters()[name] == TYPE::STRING) {
                    parameter_value = RemoveQuotes(value);
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            } else if (IsBool(value)) {
                if (tables_[table_name].GetParameters()[name] == TYPE::BOOL) {
                    parameter_value = (value == "true");
                } else {
                    throw std::logic_error("Different types of parameters");
                }
            }
        }
        return std::make_pair(name, parameter_value);
    } else {
        throw std::runtime_error("Syntax error");
    }
}

void
DataBase::UpdateWithWhere(const std::string& table_name, const std::string& new_values, std::string& where_condition) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    std::vector<std::string> vector_values;
    SplitString(new_values, vector_values);

    std::vector<std::pair<std::string, Parameter>> values;

    values.reserve(vector_values.size());
    for (const auto& i: vector_values) {
        values.push_back(SetValue(table_name, i));
    }

    for (auto& i: tables_[table_name].GetElement()) {
        if (CastFromString<bool>(ExecuteLogicExpression(ParseComplexLogic(where_condition), i))) {
            for (auto& value: values) {
                i[value.first] = value.second;
            }
        }
    }
}

void DataBase::Update(const std::string& table_name, const std::string& new_values) {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::logic_error("This table doesn't exist");
    }
    std::vector<std::string> vector_values;
    SplitString(new_values, vector_values);

    std::vector<std::pair<std::string, Parameter>> values;

    values.reserve(vector_values.size());
    for (const auto& i: vector_values) {
        values.push_back(SetValue(table_name, i));
    }

    for (auto& i: tables_[table_name].GetElement()) {
        for (auto& value: values) {
            i[value.first] = value.second;
        }
    }
}

bool DataBase::JoinPredicate(Element& first_element, const std::string& first_column, Element& second_element,
                             const std::string& sign) {
    if (sign == "=") {
        return first_element[first_column] == second_element[first_column];
    } else if (sign == ">") {
        return first_element[first_column] > second_element[first_column];
    } else if (sign == "<") {
        return first_element[first_column] < second_element[first_column];
    } else {
        return false;
    }
}

void DataBase::SelectWithJoin(const std::vector<std::pair<std::string, std::string>>& columns_list,
                              const std::string& left_table, const std::string& right_table,
                              const std::string& on_conditional, const std::string& type) {
    Table tmp_table;
    Element tmp_element;
    for (const auto& i: columns_list) {
        tmp_table.GetParameters()[i.second] = tables_[i.first].GetParameters()[i.second];
        tmp_element.GetOrder().push_back(i.second);
        tmp_element[i.second];
    }

    std::regex on_conditional_pattern(R"(\w+\.(\w+)\s+([=|>|<])\s+\w+\.\w+)");
    std::smatch tmp;
    std::string related_column;
    std::string sign;
    if (std::regex_match(on_conditional, tmp, on_conditional_pattern)) {
        related_column = tmp[1].str();
        sign = tmp[2].str();
    } else {
        throw std::runtime_error("Syntax error");
    }

    if (type == "INNER") {
        for (auto& i: tables_[left_table].GetElement()) {
            for (auto& j: tables_[right_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = i[k.second];
                        } else {
                            tmp_element[k.second] = j[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
        }
    } else if (type == "LEFT") {
        for (auto& i: tables_[left_table].GetElement()) {
            bool find_flag = false;
            for (auto& j: tables_[right_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    find_flag = true;
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = i[k.second];
                        } else {
                            tmp_element[k.second] = j[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
            if (!find_flag) {
                for (auto& k: columns_list) {
                    if (k.first == left_table) {
                        tmp_element[k.second] = i[k.second];
                    } else {
                        tmp_element[k.second] = Parameter();
                    }
                }
                tmp_table.GetElement().push_back(tmp_element);
            }
        }
    } else if (type == "RIGHT") {
        for (auto& i: tables_[right_table].GetElement()) {
            bool find_flag = false;
            for (auto& j: tables_[left_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    find_flag = true;
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = j[k.second];
                        } else {
                            tmp_element[k.second] = i[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
            if (!find_flag) {
                for (auto& k: columns_list) {
                    if (k.first == right_table) {
                        tmp_element[k.second] = i[k.second];
                    } else {
                        tmp_element[k.second] = Parameter();
                    }
                }
                tmp_table.GetElement().push_back(tmp_element);
            }
        }
    }

    for (auto& i: tmp_table.GetElement()) {
        for (auto& parameter: i.GetOrder()) {
            if (i[parameter].Type() == TYPE::NONE) {
                std::cout << "NULL";
            } else {
                i[parameter].Print();
            }
            std::cout << " ";
        }
        std::cout << "\n";
    }
}

void DataBase::SelectWithWhereAndJoin(const std::vector<std::pair<std::string, std::string>>& columns_list,
                                      const std::string& left_table, const std::string& right_table,
                                      const std::string& on_conditional, std::string& where_conditional,
                                      const std::string& type) {
    Table tmp_table;
    Element tmp_element;
    for (const auto& i: columns_list) {
        tmp_table.GetParameters()[i.second] = tables_[i.first].GetParameters()[i.second];
        tmp_element.GetOrder().push_back(i.second);
        tmp_element[i.second];
    }

    std::regex on_conditional_pattern(R"(\w+\.(\w+)\s+([=|>|<])\s+\w+\.\w+)");
    std::smatch tmp;
    std::string related_column;
    std::string sign;
    if (std::regex_match(on_conditional, tmp, on_conditional_pattern)) {
        related_column = tmp[1].str();
        sign = tmp[2].str();
    } else {
        throw std::runtime_error("Syntax error");
    }

    if (type == "INNER") {
        for (auto& i: tables_[left_table].GetElement()) {
            for (auto& j: tables_[right_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = i[k.second];
                        } else {
                            tmp_element[k.second] = j[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
        }
    } else if (type == "LEFT") {
        for (auto& i: tables_[left_table].GetElement()) {
            bool find_flag = false;
            for (auto& j: tables_[right_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    find_flag = true;
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = i[k.second];
                        } else {
                            tmp_element[k.second] = j[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
            if (!find_flag) {
                for (auto& k: columns_list) {
                    if (k.first == left_table) {
                        tmp_element[k.second] = i[k.second];
                    } else {
                        tmp_element[k.second] = Parameter();
                    }
                }
                tmp_table.GetElement().push_back(tmp_element);
            }
        }
    } else if (type == "RIGHT") {
        for (auto& i: tables_[right_table].GetElement()) {
            bool find_flag = false;
            for (auto& j: tables_[left_table].GetElement()) {
                if (JoinPredicate(i, related_column, j, sign)) {
                    find_flag = true;
                    for (auto& k: columns_list) {
                        if (k.first == left_table) {
                            tmp_element[k.second] = j[k.second];
                        } else {
                            tmp_element[k.second] = i[k.second];
                        }
                    }
                    tmp_table.GetElement().push_back(tmp_element);
                }
            }
            if (!find_flag) {
                for (auto& k: columns_list) {
                    if (k.first == right_table) {
                        tmp_element[k.second] = i[k.second];
                    } else {
                        tmp_element[k.second] = Parameter();
                    }
                }
                tmp_table.GetElement().push_back(tmp_element);
            }
        }
    }

    for (auto& i: tmp_table.GetElement()) {
        if (CastFromString<bool>(ExecuteLogicExpression(ParseComplexLogic(where_conditional), i))) {
            for (auto& parameter: i.GetOrder()) {
                if (i[parameter].Type() == TYPE::NONE) {
                    std::cout << "NULL";
                } else {
                    i[parameter].Print();
                }
                std::cout << " ";
            }
            std::cout << "\n";
        }
    }
}