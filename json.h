#pragma once


#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;


class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   
    Node () {}
    Node (std::nullptr_t): value_(std::nullptr_t{}){}
    Node (int value): value_(value){}
    Node (bool value): value_(value){}
    Node (double value): value_(value){}
    Node (std::string value): value_(value){}
    Node (Array value): value_(value){}
    Node (Dict value): value_(value){}

    const Value& GetValue() const { return value_; }
    
    bool IsInt() const { return std::holds_alternative<int>(value_); } 
    //IsDouble() Возвращает true, если в Node хранится int либо double.
    bool IsDouble() const{ return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }; 
    //IsPureDouble() Возвращает true, если в Node хранится double.
    bool IsPureDouble() const { return std::holds_alternative<double>(value_); }; 
    bool IsBool() const{ return std::holds_alternative<bool>(value_); }; 
    bool IsString() const { return std::holds_alternative<std::string>(value_); }; 
    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }; 
    bool IsArray() const { return std::holds_alternative<Array>(value_); }; 
    bool IsMap() const { return std::holds_alternative<Dict>(value_); }; 

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const { return !(*this == rhs);}

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const; //Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool empty() const {return value_.index() == 0;}
    

private:
    Value value_;
    
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& rhs) const {
        return this->root_ == rhs.root_;
    }
    bool operator!=(const Document& rhs) const {
        return !(*this == rhs);
    }
    bool empty () const {return root_.empty();}
private:
    Node root_;
};

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);
void PrintNode(const Node& node, std::ostream& out, int tab = 0);

}  // namespace json