#ifndef EXPRESSION_INT128_HPP
#define EXPRESSION_INT128_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

class Int128 {
    int64_t high;
    uint64_t low;

public:
    // Конструирование без аргументов
    Int128();

    // Конструирование от int64_t
    explicit Int128(int64_t num);

    // Конструирование от std::string_view
    explicit Int128(std::string_view str);

    // Явное приведение к int64_t
    explicit operator int64_t() const;

    // Явное приведение к double
    explicit operator double() const;

    // Перевод в строку: std::string str()
    std::string str() const;

    // Сложение: +, +=
    Int128 operator+(const Int128& rhs) const;
    Int128& operator+=(const Int128& rhs);

    // Вычитание: -, -=
    Int128 operator-(const Int128& rhs) const;
    Int128& operator-=(const Int128& rhs);

    // Умножение: *, *=
    Int128 operator*(const Int128& rhs) const;
    Int128& operator*=(const Int128& rhs);

    // Деление: /, /=
    Int128 operator/(const Int128& rhs) const;
    Int128& operator/=(const Int128& rhs);

    // Унарный минус: -
    Int128 operator-() const;

    // Сравнение на равенство: ==, !=
    bool operator==(const Int128& rhs) const;
    bool operator!=(const Int128& rhs) const;

    // Вывод в поток: <<
    friend std::ostream& operator<<(std::ostream& out, const Int128& num);

    // ============================== Custom ==============================

private:
    // Конструирование от int64_t(high) и uint64_t(low)
    explicit Int128(int64_t high, uint64_t low);

    // Получение бита по номеру от 0 до 127
    bool bit(int index) const;

    // Беззнаковое деление битового представления this на rhs
    void div_mod_unsigned(const Int128& rhs, Int128& quotient, Int128& remainder) const;

    // Сравнение побитовое (беззнаковое): <, <=
    bool bit_lt(const Int128& rhs) const;
    bool bit_le(const Int128& rhs) const;

public:
    // Строка из бит (с разделителем)
    std::string bit_string(bool separator = false) const;

    // Побитовое NOT (~)
    Int128 operator~() const;

    // Константы 0 и 1
    static const Int128 ZERO;
    static const Int128 ONE;

    // Битовый сдвиг влево
    Int128 operator<<(int shift) const;
    Int128& operator<<=(int shift);

    // Битовый сдвиг вправо
    Int128 operator>>(int shift) const;
    Int128& operator>>=(int shift);

    // Остаток от деления: %, %=
    Int128 operator%(const Int128& rhs) const;
    Int128& operator%=(const Int128& rhs);

    // Сравнение: <, <=
    bool operator<(const Int128& rhs) const;
    bool operator<=(const Int128& rhs) const;

    // Сравнение: >, >=
    bool operator>(const Int128& rhs) const;
    bool operator>=(const Int128& rhs) const;

    // Побитовое И: &, &=
    Int128 operator&(const Int128& rhs) const;
    Int128& operator&=(const Int128& rhs);

    // Побитовое ИЛИ: |, |=
    Int128 operator|(const Int128& rhs) const;
    Int128& operator|=(const Int128& rhs);

    // Модуль числа: abs()
    Int128 abs() const;
};

class Expression {
public:
    using Variables = std::unordered_map<std::string, Int128>;

    virtual ~Expression() = default;

    virtual Int128 eval(const Variables& variables) const = 0;
    virtual Expression* clone() const = 0;
    virtual std::string str() const = 0;
};

std::ostream& operator<<(std::ostream& out, const Expression& expression);

class Const final : public Expression {
    Int128 value;

public:
    explicit Const(const Int128& value);
    explicit Const(int64_t value);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Variable final : public Expression {
    std::string name;

public:
    explicit Variable(std::string name);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class UnaryExpression : public Expression {
protected:
    std::unique_ptr<Expression> operand;

public:
    explicit UnaryExpression(const Expression& operand);
    UnaryExpression(const UnaryExpression& other);
    UnaryExpression& operator=(const UnaryExpression& other);
    UnaryExpression(UnaryExpression&& other) noexcept = default;
    UnaryExpression& operator=(UnaryExpression&& other) noexcept = default;
};

class BinaryExpression : public Expression {
protected:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

public:
    BinaryExpression(const Expression& left, const Expression& right);
    BinaryExpression(const BinaryExpression& other);
    BinaryExpression& operator=(const BinaryExpression& other);
    BinaryExpression(BinaryExpression&& other) noexcept = default;
    BinaryExpression& operator=(BinaryExpression&& other) noexcept = default;
};

class Negate final : public UnaryExpression {
public:
    explicit Negate(const Expression& operand);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Add final : public BinaryExpression {
public:
    Add(const Expression& left, const Expression& right);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Subtract final : public BinaryExpression {
public:
    Subtract(const Expression& left, const Expression& right);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Multiply final : public BinaryExpression {
public:
    Multiply(const Expression& left, const Expression& right);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Divide final : public BinaryExpression {
public:
    Divide(const Expression& left, const Expression& right);

    Int128 eval(const Variables& variables) const override;
    Expression* clone() const override;
    std::string str() const override;
};

Negate operator-(const Expression& operand);
Add operator+(const Expression& left, const Expression& right);
Subtract operator-(const Expression& left, const Expression& right);
Multiply operator*(const Expression& left, const Expression& right);
Divide operator/(const Expression& left, const Expression& right);

#endif  // EXPRESSION_INT128_HPP