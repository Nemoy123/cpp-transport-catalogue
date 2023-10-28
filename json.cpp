#include "json.h"
#include <sstream>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (c = {}; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        
        result.push_back(LoadNode(input));
    }
    if (result.empty() && c != ']') {
        throw json::ParsingError("Failed array"s);
    }
    return   Node(move(result)) ;
}



using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
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
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}


// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string ReadStringFromInput (std::istream& input) {
    using namespace std::literals;
    
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

    return s;
}

Node LoadString(istream& input) {
   string line = ReadStringFromInput (input);
    //string line;
    //getline(input, line, '"');
    
    return Node(move(line));
}
//{"key1"s, "value1"s}, {"key2"s, 42}
Node LoadDict(istream& input) {
    Dict result;
    char c;
    for ( c = {} ; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        //input.putback(c);
        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (result.empty() && c != '}') {
        throw json::ParsingError("Failed array"s);
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    
    char c;
    input >> c;
    if (c == 0) {return {};}
   
    if (c == '[') {
        return LoadArray(input);
    } 
    else if (c == '{') {
        
        return LoadDict(input);
    } 
    else if (c == '"') {
        return LoadString(input);
    } 
    else if (c == 'n' || c == 'f' || c == 't') {
        input.putback(c);
        //char test;
        std::string temp;
        int count = 0;
        for (char ch_1; input >> ch_1 && ch_1 != '}';) {
            //input >> ch_1;
            temp += ch_1;
            if (temp == "null"s) {
                return Node(nullptr);
            } 
            else if (temp == "true"s || temp == "false"s) {
                bool b;
                std::istringstream is (temp);
                is >> std::boolalpha >> b;
                return Node(b);
            }
            ++count;
            if (count > 5) {break;}
        }
        
        throw json::ParsingError("Failed null"s);
    }
    
    else {
        input.putback(c);
        auto it = LoadNumber(input);
        if (std::holds_alternative<double>(it))  {
             return Node(std::get<double>(it));
        }
        else if (std::holds_alternative<int>(it))  {
             return Node(std::get<int>(it));
        }
        else {throw json::ParsingError("Failed number"s);}
    }
}

}  // namespace


int Node::AsInt() const {
    if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        else {
            throw std::logic_error ("logic_error");
    }
}

bool Node::AsBool() const  {
    if (std::holds_alternative<bool>(value_)) {
            return std::get<bool>(value_);
        }
        else {
            throw std::logic_error ("logic_error");
    }
}

double Node::AsDouble() const {
    if (std::holds_alternative<double>(value_)) {
            return std::get<double>(value_);
        }
    else if (std::holds_alternative<int>(value_)) {
        return static_cast<double> (std::get<int>(value_));
    }
    else {
            throw std::logic_error ("logic_error");
    }
}


const std::string& Node::AsString() const {
     if (std::holds_alternative<std::string>(value_)) {
            const std::string& x_val = std::get<std::string>(value_);
            return x_val;
        }
        else {
            throw std::logic_error ("logic_error");
        }
}

const Array& Node::AsArray() const {
        if (std::holds_alternative<Array>(value_)) {
            const Array& x_val = std::get<Array>(value_);
            return x_val;
        }
        else {
            throw std::logic_error ("logic_error");
        }
}

Array& Node::AsArray() {
        
        if (!IsArray()) {
            throw std::logic_error("Not an array"s);
        }
        return std::get<Array>(value_);
}


const Dict& Node::AsDict() const {
        if (std::holds_alternative<Dict>(value_)) {
            const Dict& x_val = std::get<Dict>(value_);
            return x_val;
        }
        else {
            throw std::logic_error ("logic_error");
        }
}

Dict& Node::AsDict() {
        using namespace std::literals;
        if (!IsDict()) {
            throw std::logic_error("Not a dict"s);
        }

        return std::get<Dict>(value_);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintValue(const std::string& value, std::ostream& out, int tab) {
    (void)tab;
    out << "\""s;
    for (auto ch = value.cbegin(); ch != value.cend(); ch++) {
        
        switch (*ch) {
            case '"': out << "\\\""s; break;
            case '\\': out << "\\\\"s; break;
            case '\b': out << "\\b"s; break;
            case '\f': out << "\\f"s; break;
            case '\n': out << "\\n"s; break;
            case '\r': out << "\\r"s; break;
           // case '\t': out << "\\t"s; break;
            default:
            out << *ch;
        }
    }
    out << "\""s;
}

void PrintValue(const int& value, std::ostream& out, int tab) {
    (void)tab;
   out << value;
}

void PrintValue(const double& value, std::ostream& out, int tab) {
    (void)tab;
   out << value;
}

void PrintValue(const bool& value, std::ostream& out, int tab) {
    (void)tab;
   out << std::boolalpha << value;
}

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array& array, std::ostream& out, int tab) {
    //out << std::endl << std::string (tab, 32);
    int old = tab;
    out << "[";
    out << std::endl<<std::string (tab+=3, 32);
    
    bool begin = true;
    for (const auto& val : array) {
        if (!begin) { 
            out << ",";
            out << std::endl << std::string(tab, 32);
        }
        PrintNode (val, out, tab);
        begin = false;
    }
    out << std::endl<<std::string (old, 32);
    out << "]";

}

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict& dict, std::ostream& out, int ots) {
    bool begin = true;
   // out << std::endl << std::string(ots, 32);
    
    int old = ots;
    out << "{";
    out << std::endl;
    out << std::string(ots += 3, 32);
    for (const auto& val : dict) {
        if (!begin) {
            out << ", ";
            out << std::endl << std::string(ots, 32);
        }
        out << "\"";
        out << val.first ;
        out << "\": ";
        PrintNode (val.second, out, ots);
        begin = false;
    }
    out << std::endl;
    out << std::string(old, 32);
    out << "}";
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out, int tab) {
    (void)tab;
    out << "null"sv;
}

void PrintNode(const Node& node, std::ostream& out, int tab) {
    
    std::visit(
        [&out, &tab](const auto& value){ 
            PrintValue(value, out, tab); 
        },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
     PrintNode (doc.GetRoot(), output);
}

bool Node::operator==(const Node& rhs) const {
        if (this->IsNull() && rhs.IsNull()) {
            return true;
        }
        if (this->IsInt() && rhs.IsInt()) {
           auto lhs_value = std::holds_alternative<int>(this->value_);
           auto rhs_value = std::holds_alternative<int>(rhs.value_);
           return lhs_value == rhs_value;
        }
        if (this->IsPureDouble() && rhs.IsPureDouble()) {
           auto lhs_value = std::holds_alternative<double>(this->value_);
           auto rhs_value = std::holds_alternative<double>(rhs.value_);
           return lhs_value == rhs_value;
        }
        if (this->IsBool() && rhs.IsBool()) {
           auto lhs_value = std::holds_alternative<bool>(this->value_);
           auto rhs_value = std::holds_alternative<bool>(rhs.value_);
           return lhs_value == rhs_value;
        }
        if (this->IsString() && rhs.IsString()) {
           auto lhs_value = std::holds_alternative<std::string>(this->value_);
           auto rhs_value = std::holds_alternative<std::string>(rhs.value_);
           return lhs_value == rhs_value;
        }
        if (this->IsArray() && rhs.IsArray()) {
           auto lhs_value = std::holds_alternative<Array>(this->value_);
           auto rhs_value = std::holds_alternative<Array>(rhs.value_);
           return lhs_value == rhs_value;
        }
        if (this->IsDict() && rhs.IsDict()) {
           auto lhs_value = std::holds_alternative<Dict>(this->value_);
           auto rhs_value = std::holds_alternative<Dict>(rhs.value_);
           return lhs_value == rhs_value;
        }
    
        return false;
}



}  // namespace json