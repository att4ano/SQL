#include <iostream>
#include <variant>

enum class TYPE {
    INT,
    BOOL,
    FLOAT,
    DOUBLE,
    STRING,
    NONE
};

class Parameter {
public:
    using value_type = std::variant<bool, int, float, double, std::string>;
private:
    TYPE type_ = TYPE::NONE;
    value_type value_;
public:

    Parameter() = default;

    template<typename T> requires (std::is_same<T, int>::value)
    explicit Parameter(const T& value) : value_(value) {
        type_ = TYPE::INT;
    }

    template<typename T> requires (std::is_same<T, bool>::value)
    explicit Parameter(const T& value) : value_(value) {
        type_ = TYPE::BOOL;
    }

    template<typename T> requires (std::is_same<T, float>::value)
    explicit Parameter(const T& value) : value_(value) {
        type_ = TYPE::FLOAT;
    }

    template<typename T> requires (std::is_same<T, double>::value)
    explicit Parameter(const T& value) : value_(value) {
        type_ = TYPE::DOUBLE;
    }

    template<typename T> requires (std::is_same<T, std::string>::value)
    explicit Parameter(const T& value) : value_(value) {
        type_ = TYPE::STRING;
    }

    template<typename T>
    [[nodiscard]] T& GetValue() noexcept {
        return std::get<T>(value_);
    }

    TYPE& Type() noexcept {
        return type_;
    }

    template<typename T> requires (std::is_same<T, int>::value)
    Parameter& operator=(T value) {
        value_ = value;
        type_ = TYPE::INT;
        return *this;
    }

    template<typename T> requires (std::is_same<T, double>::value)
    Parameter& operator=(T value) {
        value_ = value;
        type_ = TYPE::DOUBLE;
        return *this;
    }

    template<typename T> requires (std::is_same<T, float>::value)
    Parameter& operator=(T value) {
        value_ = value;
        type_ = TYPE::FLOAT;
        return *this;
    }

    template<typename T> requires (std::is_same<T, bool>::value)
    Parameter& operator=(T value) {
        value_ = value;
        type_ = TYPE::BOOL;
        return *this;
    }

    template<typename T> requires (std::is_same<T, std::string>::value)
    Parameter& operator=(T value) {
        value_ = value;
        type_ = TYPE::STRING;
        return *this;
    }

    bool operator<(const Parameter& other) const {
        if (type_ != other.type_) {
            throw std::logic_error("Comparing different types");
        } else {
            return value_ < other.value_;
        }
    }

    bool operator>(const Parameter& other) const {
        if (type_ != other.type_) {
            throw std::logic_error("Comparing different types");
        } else {
            return value_ > other.value_;
        }
    }

    bool operator>=(const Parameter& other) const {
        if (type_ != other.type_) {
            throw std::logic_error("Comparing different types");
        } else {
            return !(value_ >= other.value_);
        }
    }

    bool operator<=(const Parameter& other) const {
        if (type_ != other.type_) {
            throw std::logic_error("Comparing different types");
        } else {
            return !(value_ <= other.value_);
        }
    }

    bool operator==(const Parameter& other) const {
        if (type_ != other.type_) {
            throw std::logic_error("Comparing different types");
        } else {
            return value_ == other.value_;
        }
    }

    void Print() {
        if (type_ == TYPE::INT) {
            std::cout << GetValue<int>();
        } else if (type_ == TYPE::BOOL) {
            std::cout << GetValue<bool>();
        } else if (type_ == TYPE::FLOAT) {
            std::cout << GetValue<float>();
        } else if (type_ == TYPE::DOUBLE) {
            std::cout << GetValue<double>();
        } else if (type_ == TYPE::STRING) {
            std::cout << GetValue<std::string>();
        } else {
            return;
        }
    }

};