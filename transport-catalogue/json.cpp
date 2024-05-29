#include "json.h"

using namespace std::literals;

namespace json {

    namespace {
        bool IsSpaceSymbol(const char c) {
            return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
        }
        bool IsDelimiterSymbol(const char c) {
            return (c == ',' || c == ']' || c == '}' || c == '\0' || c == '\"');
        }

        void SkipSpaces(std::istream &input) {
            char c;
            while (input >> c && IsSpaceSymbol(c)) {
            }
            if (!input.eof() && !input.bad()) {
                input.putback(c);
            } else {
                throw ParsingError("Unexpected end of stream"s);
            }
        }

        Node LoadNode(std::istream &input);

        Node LoadNull(std::istream &input) {
            char c;
            input >> c;

            if (!(input >> c) || c != 'u') {
                throw ParsingError("Error reading literal null"s);
            }
            if (!(input >> c) || c != 'l') {
                throw ParsingError("Error reading literal null"s);
            }
            if (!(input >> c) || c != 'l') {
                throw ParsingError("Error reading literal null"s);
            }
            c = static_cast<char>(input.peek());
            if (!(input.eof() || input.bad()) && !(IsSpaceSymbol(c) || IsDelimiterSymbol(c))) {
                throw ParsingError("Error reading literal null"s);
            }
            return Node();
        }

        Node LoadBool(std::istream &input) {
            char c;
            input >> c;
            if (c == 't') { // читаем литерал true
                if (!(input >> c) || c != 'r') {
                    throw ParsingError("Error reading literal true"s);
                }
                if (!(input >> c) || c != 'u') {
                    throw ParsingError("Error reading literal true"s);
                }
                if (!(input >> c) || c != 'e') {
                    throw ParsingError("Error reading literal true"s);
                }
                c = static_cast<char>(input.peek());
                if (!(input.eof() || input.bad()) && !(IsSpaceSymbol(c) || IsDelimiterSymbol(c))) {
                    throw ParsingError("Error reading literal true"s);
                }
                return Node(true);
            } else { // читаем литерал false
                if (!(input >> c) || c != 'a') {
                    throw ParsingError("Error reading literal false"s);
                }
                if (!(input >> c) || c != 'l') {
                    throw ParsingError("Error reading literal false"s);
                }
                if (!(input >> c) || c != 's') {
                    throw ParsingError("Error reading literal false"s);
                }
                if (!(input >> c) || c != 'e') {
                    throw ParsingError("Error reading literal false"s);
                }
                c = static_cast<char>(input.peek());
                if (!(input.eof() || input.bad()) && !(IsSpaceSymbol(c) || IsDelimiterSymbol(c))) {
                    throw ParsingError("Error reading literal false"s);
                }
                return Node(false);
            }
        }

        Node LoadNumber(std::istream &input) {
            std::string parsed_num;

            SkipSpaces(input);

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Error reading number from input stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("Number expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream &input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(std::move(s));
        }

        Node LoadArray(std::istream &input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (IsSpaceSymbol(c)) {
                    continue;
                }
                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }
            if ((input.eof() || input.bad()) && c != ']') {
                throw ParsingError("Unexpected end of stream"s);
            }
            return Node(std::move(result));
        }

        Node LoadDict(std::istream &input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (IsSpaceSymbol(c)) {
                    continue;
                }
                if (c == ',') {
                    input >> c;
                }

                std::string key = LoadString(input).AsString();
                input >> c;
                result.insert({std::move(key), LoadNode(input)});
            }
            if ((input.eof() || input.bad()) && c != '}') {
                throw ParsingError("Unexpected end of stream"s);
            }
            return Node(std::move(result));
        }

        Node LoadNode(std::istream &input) {
            char c;

            SkipSpaces(input);
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    } // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        } else {
            throw std::logic_error("Not integer");
        }
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        } else {
            throw std::logic_error("Not boolean");
        }
    }
    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(value_);
        } else if (IsInt()) {
            return static_cast<double>(std::get<int>(value_));
        } else {
            throw std::logic_error("Not double");
        }
    }
    const std::string &Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        } else {
            throw std::logic_error("Not string");
        }
    }
    const Array &Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        } else {
            throw std::logic_error("Not Array");
        }
    }
    const Dict &Node::AsDict() const {
        if (IsDict()) {
            return std::get<Dict>(value_);
        } else {
            throw std::logic_error("Not Dict");
        }
    }

    std::ostream &operator<<(std::ostream &output, const Node &node) {
        PrintNode(node, PrintContext{output});
        return output;
    }

    std::ostream &operator<<(std::ostream &output, const Array &value) {
        PrintValue(value, PrintContext{output});
        return output;
    }

    inline bool operator==(const Document &left, const Document &right) {
        return left.GetRoot() == right.GetRoot();
    }

    inline bool operator!=(const Document &left, const Document &right) {
        return !(left == right);
    }

    Document Load(std::istream &input) {
        return Document{LoadNode(input)};
    }

    void PrintString(const std::string &value, std::ostream &out) {
        out << "\""sv;
        for (const char c : value) {
            if (c == '\\' || c == '\"') {
                out << '\\';
                out << c;
            } else if (c == '\n') {
                out << "\\n"sv;
            } else if (c == '\t') {
                out << "\\t"sv;
            } else if (c == '\r') {
                out << "\\r"sv;
            } else {
                out << c;
            }
        }
        out << "\""sv;
    }

    void PrintValue(std::nullptr_t, const PrintContext &ctx) {
        ctx.out << "null"sv;
    }
    void PrintValue(const Array &value, const PrintContext &ctx) {
        std::ostream &out = ctx.out;
        bool first_time = true;
        out << "[\n"sv;
        auto next_ctx = ctx.Indented();

        for (const Node &node : value) {
            if (!first_time) {
                out << ",\n"sv;
            } else {
                first_time = false;
            }
            next_ctx.PrintIndent();
            PrintNode(node, next_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }
    void PrintValue(const Dict &value, const PrintContext &ctx) {
        std::ostream &out = ctx.out;
        bool first_time = true;
        out << "{\n"sv;
        auto next_ctx = ctx.Indented();

        for (const auto &[key, node] : value) {
            if (!first_time) {
                out << ",\n"sv;
            } else {
                first_time = false;
            }
            next_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, next_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }
    void PrintValue(const bool value, const PrintContext &ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    void PrintValue(const std::string value, const PrintContext &ctx) {
        PrintString(value, ctx.out);
    }

    void Print(const Document &doc, std::ostream &output) {
        PrintNode(doc.GetRoot(), PrintContext{output});
    }

    void PrintNode(const Node &node, const PrintContext &ctx) {
        std::visit([&ctx](const auto &value) { PrintValue(value, ctx); }, node.GetValue());
    }

} // namespace json