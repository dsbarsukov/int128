#include "Int128.hpp"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <utility>

namespace {
uint64_t arithmetic_shift_right_u64(uint64_t value, int shift, bool negative) {
    if (shift == 0) {
        return value;
    }
    if (!negative) {
        return value >> shift;
    }
    return ~(~value >> shift);
}
}

// Конструирование без аргументов
Int128::Int128() : high(0), low(0) {}

// Конструирование от int64_t
Int128::Int128(int64_t num) : high(num < 0 ? -1 : 0), low(static_cast<uint64_t>(num)) {}

// Конструирование от int64_t(high) и uint64_t(low)
Int128::Int128(int64_t _high, uint64_t _low) : high(_high), low(_low) {}

// Конструирование от std::string_view
Int128::Int128(std::string_view str) : high(0), low(0) {
    bool is_negative = false;
    size_t start     = 0;
    if (!str.empty() && str[0] == '-') {
        is_negative = true;
        start       = 1;
    }
    for (size_t i = start; i < str.size(); ++i) {
        *this *= Int128(10);
        *this += Int128(str[i] - '0');
    }
    if (is_negative) {
        *this = -*this;
    }
}

// Явное приведение к int64_t
Int128::operator int64_t() const {
    return static_cast<int64_t>(low);
}

// Явное приведение к double
Int128::operator double() const {
    return std::ldexp(static_cast<double>(high), 64) + static_cast<double>(low);
}

// Перевод в строку: std::string str()
std::string Int128::str() const {
    if (*this == ZERO) {
        return "0";
    }

    std::string result;
    Int128 tmp = (*this < ZERO ? -*this : *this);
    while (tmp != ZERO) {
        Int128 quotient;
        Int128 remainder;
        tmp.div_mod_unsigned(Int128(10), quotient, remainder);
        result += static_cast<char>('0' + remainder.low);
        tmp = quotient;
    }
    if (*this < ZERO) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// Сложение: +, +=
Int128 Int128::operator+(const Int128 &_rhs) const {
    Int128 result;
    result.low = low + _rhs.low;
    result.high = static_cast<int64_t>(static_cast<uint64_t>(high) +
                                       static_cast<uint64_t>(_rhs.high) +
                                       (result.low < low ? 1 : 0));
    return result;
}
Int128 &Int128::operator+=(const Int128 &rhs) {
    *this = *this + rhs;
    return *this;
}

// Вычитание: -, -=
Int128 Int128::operator-(const Int128 &rhs) const {
    return *this + -rhs;
}
Int128 &Int128::operator-=(const Int128 &rhs) {
    *this = *this - rhs;
    return *this;
}

// Умножение: *, *=
Int128 Int128::operator*(const Int128& rhs) const {
    Int128 result;
    for (int i = 0; i < 128; ++i) {
        if (rhs.bit(i)) {
            result += (*this << i);
        }
    }
    return result;
}
Int128 &Int128::operator*=(const Int128 &rhs) {
    *this = *this * rhs;
    return *this;
}

// Деление: /, /=
Int128 Int128::operator/(const Int128& rhs) const {
    bool is_negative = (*this < ZERO) != (rhs < ZERO);
    Int128 lhs_abs = (*this < ZERO ? -*this : *this);
    Int128 rhs_abs = (rhs < ZERO ? -rhs : rhs);

    Int128 quotient;
    Int128 remainder;
    lhs_abs.div_mod_unsigned(rhs_abs, quotient, remainder);

    if (is_negative) {
        quotient = -quotient;
    }
    return quotient;
}
Int128 &Int128::operator/=(const Int128 &rhs) {
    *this = *this / rhs;
    return *this;
}

// Унарный минус: -
Int128 Int128::operator-() const {
    Int128 result = ~(*this);
    result += ONE;
    return result;
}

// Сравнение на равенство: ==, !=
bool Int128::operator==(const Int128 &rhs) const {
    return high == rhs.high && low == rhs.low;
}
bool Int128::operator!=(const Int128 &rhs) const {
    return !(*this == rhs);
}

// Вывод в поток: <<
std::ostream &operator<<(std::ostream &out, const Int128 &num) {
    return out << num.str();
}

// ====================================================== Custom =======================================================

bool Int128::bit(int index) const {
    if (index < 64) {
        return ((low >> index) & 1) != 0;
    }
    return ((static_cast<uint64_t>(high) >> (index - 64)) & 1) != 0;
}

void Int128::div_mod_unsigned(const Int128& rhs, Int128& quotient, Int128& remainder) const {
    quotient = ZERO;
    remainder = ZERO;

    for (int i = 127; i >= 0; --i) {
        remainder <<= 1;
        if (bit(i)) {
            remainder |= ONE;
        }
        if (rhs.bit_le(remainder)) {
            remainder -= rhs;
            quotient |= (ONE << i);
        }
    }
}

// Строка из бит (с разделителем)
std::string Int128::bit_string(bool separator) const {
    std::bitset<64> high_bits(static_cast<uint64_t>(high));
    std::bitset<64> low_bits(low);
    std::string result = high_bits.to_string();
    if (separator) {
        result += '|';
    }
    result += low_bits.to_string();
    return result;
}

// Побитовое NOT (~)
Int128 Int128::operator~() const {
    return Int128(~high, ~low);
}

// Константы 0 и 1
const Int128 Int128::ZERO = Int128(0);
const Int128 Int128::ONE  = Int128(1);

// Битовый сдвиг влево
Int128 Int128::operator<<(int shift) const {
    if (shift <= 0) {
        return *this;
    }
    if (shift >= 128) {
        return ZERO;
    }

    Int128 result;
    uint64_t high_bits = static_cast<uint64_t>(high);
    if (shift >= 64) {
        result.high = static_cast<int64_t>(low << (shift - 64));
        result.low = 0;
    } else {
        result.high = static_cast<int64_t>((high_bits << shift) | (low >> (64 - shift)));
        result.low = low << shift;
    }
    return result;
}
Int128 &Int128::operator<<=(int shift) {
    *this = *this << shift;
    return *this;
}

// Битовый сдвиг вправо
Int128 Int128::operator>>(int shift) const {
    if (shift <= 0) {
        return *this;
    }

    bool negative = high < 0;
    if (shift >= 128) {
        return negative ? Int128(-1) : ZERO;
    }

    Int128 result;
    uint64_t high_bits = static_cast<uint64_t>(high);
    if (shift >= 64) {
        int s = shift - 64;
        result.high = negative ? -1 : 0;
        result.low = arithmetic_shift_right_u64(high_bits, s, negative);
    } else {
        result.high = static_cast<int64_t>(arithmetic_shift_right_u64(high_bits, shift, negative));
        result.low = (low >> shift) | (high_bits << (64 - shift));
    }
    return result;
}
Int128 &Int128::operator>>=(int shift) {
    *this = *this >> shift;
    return *this;
}

// Остаток от деления: %, %=
Int128 Int128::operator%(const Int128 &rhs) const {
    return *this - (*this / rhs) * rhs;
}
Int128 &Int128::operator%=(const Int128 &rhs) {
    *this = *this % rhs;
    return *this;
}

// Сравнение: <, <=
bool Int128::operator<(const Int128 &rhs) const {
    if (high == rhs.high) {
        return low < rhs.low;
    }
    return high < rhs.high;
}
bool Int128::operator<=(const Int128 &rhs) const {
    return *this < rhs || *this == rhs;
}

// Сравнение: >, >=
bool Int128::operator>(const Int128 &rhs) const {
    return rhs < *this;
}
bool Int128::operator>=(const Int128 &rhs) const {
    return rhs <= *this;
}

// Сравнение побитовое (беззнаковое): <, <=
bool Int128::bit_lt(const Int128 &rhs) const {
    uint64_t lhs_high = static_cast<uint64_t>(high);
    uint64_t rhs_high = static_cast<uint64_t>(rhs.high);
    if (lhs_high == rhs_high) {
        return low < rhs.low;
    }
    return lhs_high < rhs_high;
}
bool Int128::bit_le(const Int128 &rhs) const {
    return bit_lt(rhs) || *this == rhs;
}

// Побитовое И: &, &=
Int128 Int128::operator&(const Int128 &rhs) const {
    return Int128(high & rhs.high, low & rhs.low);
}
Int128 &Int128::operator&=(const Int128 &rhs) {
    *this = *this & rhs;
    return *this;
}

// Побитовое ИЛИ: |, |=
Int128 Int128::operator|(const Int128 &rhs) const {
    return Int128(high | rhs.high, low | rhs.low);
}
Int128 &Int128::operator|=(const Int128 &rhs) {
    *this = *this | rhs;
    return *this;
}

// Модуль числа: abs()
Int128 Int128::abs() const {
    if (high < 0) {
        return -*this;
    }
    return *this;
}

// ==================================================== Expression =====================================================

std::ostream& operator<<(std::ostream& out, const Expression& expression) {
    return out << expression.str();
}

Const::Const(const Int128& _value) : value(_value) {}
Const::Const(int64_t _value) : value(_value) {}

Int128 Const::eval(const Variables&) const {
    return value;
}
Expression* Const::clone() const {
    return new Const(*this);
}
std::string Const::str() const {
    return value.str();
}

Variable::Variable(std::string _name) : name(std::move(_name)) {}

Int128 Variable::eval(const Variables& variables) const {
    return variables.at(name);
}
Expression* Variable::clone() const {
    return new Variable(*this);
}
std::string Variable::str() const {
    return name;
}

UnaryExpression::UnaryExpression(const Expression& _operand) : operand(_operand.clone()) {}
UnaryExpression::UnaryExpression(const UnaryExpression& other) : operand(other.operand->clone()) {}
UnaryExpression& UnaryExpression::operator=(const UnaryExpression& other) {
    if (this != &other) {
        operand.reset(other.operand->clone());
    }
    return *this;
}

BinaryExpression::BinaryExpression(const Expression& _left, const Expression& _right)
    : left(_left.clone()), right(_right.clone()) {}
BinaryExpression::BinaryExpression(const BinaryExpression& other)
    : left(other.left->clone()), right(other.right->clone()) {}
BinaryExpression& BinaryExpression::operator=(const BinaryExpression& other) {
    if (this != &other) {
        left.reset(other.left->clone());
        right.reset(other.right->clone());
    }
    return *this;
}

Negate::Negate(const Expression& _operand) : UnaryExpression(_operand) {}
Int128 Negate::eval(const Variables& variables) const {
    return -operand->eval(variables);
}
Expression* Negate::clone() const {
    return new Negate(*this);
}
std::string Negate::str() const {
    return "(-" + operand->str() + ")";
}

Add::Add(const Expression& _left, const Expression& _right) : BinaryExpression(_left, _right) {}
Int128 Add::eval(const Variables& variables) const {
    return left->eval(variables) + right->eval(variables);
}
Expression* Add::clone() const {
    return new Add(*this);
}
std::string Add::str() const {
    return "(" + left->str() + " + " + right->str() + ")";
}

Subtract::Subtract(const Expression& _left, const Expression& _right) : BinaryExpression(_left, _right) {}
Int128 Subtract::eval(const Variables& variables) const {
    return left->eval(variables) - right->eval(variables);
}
Expression* Subtract::clone() const {
    return new Subtract(*this);
}
std::string Subtract::str() const {
    return "(" + left->str() + " - " + right->str() + ")";
}

Multiply::Multiply(const Expression& _left, const Expression& _right) : BinaryExpression(_left, _right) {}
Int128 Multiply::eval(const Variables& variables) const {
    return left->eval(variables) * right->eval(variables);
}
Expression* Multiply::clone() const {
    return new Multiply(*this);
}
std::string Multiply::str() const {
    return "(" + left->str() + " * " + right->str() + ")";
}

Divide::Divide(const Expression& _left, const Expression& _right) : BinaryExpression(_left, _right) {}
Int128 Divide::eval(const Variables& variables) const {
    return left->eval(variables) / right->eval(variables);
}
Expression* Divide::clone() const {
    return new Divide(*this);
}
std::string Divide::str() const {
    return "(" + left->str() + " / " + right->str() + ")";
}

Negate operator-(const Expression& operand) {
    return Negate(operand);
}
Add operator+(const Expression& left, const Expression& right) {
    return Add(left, right);
}
Subtract operator-(const Expression& left, const Expression& right) {
    return Subtract(left, right);
}
Multiply operator*(const Expression& left, const Expression& right) {
    return Multiply(left, right);
}
Divide operator/(const Expression& left, const Expression& right) {
    return Divide(left, right);
}
