#pragma once
/*
 * Библиотека для работы с JSON данными
 * Класс Node должен хранить значения одного из следующих типов:
 * 1) Целые числа типа int.
 * 2) Вещественные числа типа double.
 * 3) Строки — тип std::string.
 * 4) Логический тип bool.
 * 5) Массивы: using Array = std::vector<Node>;
 * 6) Словари: using Dict = std::map<std::string, Node>;
 * 7) std::nullptr_t. В C++ значение nullptr имеет тип std::nullptr_t. Используется, чтобы представить значение null в JSON документе.
 *
 * Следующие методы Node сообщают, хранится ли внутри значение некоторого типа:
 * 1) bool IsInt() const;
 * 2) bool IsDouble() const; Возвращает true, если в Node хранится int либо double.
 * 3) bool IsPureDouble() const; Возвращает true, если в Node хранится double.
 * 4) bool IsBool() const;
 * 5) bool IsString() const;
 * 6) bool IsNull() const;
 * 7) bool IsArray() const;
 * 8) bool IsMap() const;
 *
 * Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа.
 * Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
 * 1) int AsInt() const;
 * 2) bool AsBool() const;
 * 3) double AsDouble() const;. Возвращает значение типа double, если внутри хранится double либо int.
 * В последнем случае возвращается приведённое в double значение.
 * 4) const std::string& AsString() const;
 * 5) const Array& AsArray() const;
 * 6) const Map& AsMap() const;
 * Объекты Node можно сравнивать между собой при помощи == и !=. Значения равны, если внутри них значения имеют одинаковый тип и содержимое.
 *
 * Валидные JSON-документы должны успешно проходить загрузку. При загрузке невалидных JSON-документов должно выбрасываться исключение json::ParsingError.
 */

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final {
    public:
        friend class Builder;
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t)
            : value_(nullptr) {
        }
        Node(int val)
            : value_(val) {
        }
        Node(double val)
            : value_(val) {
        }
        Node(std::string val)
            : value_(std::move(val)) {
        }
        Node(Array val)
            : value_(std::move(val)) {
        }
        Node(Dict val)
            : value_(std::move(val)) {
        }
        Node(bool val)
            : value_(val) {
        }

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string &AsString() const;
        const Array &AsArray() const;
        const Dict &AsMap() const;

        const Value &GetValue() const {
            return value_;
        }
        bool operator==(const Node &rhs) const {
            return value_ == rhs.value_;
        }

    private:
        Value &GetValue() {
            return value_;
        }
        Value value_;
    };

    inline bool operator!=(const Node &lhs, const Node &rhs) {
        return !(lhs == rhs);
    }

    std::ostream &operator<<(std::ostream &output, const Node &node);
    std::ostream &operator<<(std::ostream &output, const Array &value);

    class Document {
    public:
        explicit Document(Node root) : root_(std::move(root)) {}

        const Node &GetRoot() const {
            return root_;
        }

    private:
        Node root_;
    };

    inline bool operator==(const Document &left, const Document &right);
    inline bool operator!=(const Document &left, const Document &right);

    Document Load(std::istream &input);

    // Контекст вывода, хранит ссылку на поток вывода и текущий отступ
    struct PrintContext {
        std::ostream &out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i != indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return {out, indent_step, indent_step + indent};
        }
    };

    void Print(const Document &doc, std::ostream &output);

    template <typename Value>
    void PrintValue(const Value &value, const PrintContext &ctx) {
        ctx.out << value;
    }

    void PrintValue(std::nullptr_t, const PrintContext &ctx);
    void PrintValue(const Array &value, const PrintContext &ctx);
    void PrintValue(const Dict &value, const PrintContext &ctx);
    void PrintValue(const bool value, const PrintContext &ctx);
    void PrintValue(const std::string value, const PrintContext &ctx);

    void PrintNode(const Node &node, const PrintContext &ctx);

} // namespace json