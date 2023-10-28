#pragma once
#include "json.h"
#include <stack>
#include <string>

namespace json {

//class Builder;

class Builder {
    
private:
    class BaseContext;
    class DictKeyContext;
    class ArrayItemContext;
    class DictValueContext;

public:
    Builder()
    {
        root_ = nullptr;
        last_op_ = "Build";
    };

    DictValueContext Key(std::string key);
    BaseContext Value(json::Value value);
    DictKeyContext StartDict();

    BaseContext EndDict();
    ArrayItemContext StartArray();
    BaseContext EndArray();
    json::Node Build();

private:
    std::string last_op_ = {};
    Node root_;

    std::string key_;
    std::stack<Node*> nodes_stack_;

    class BaseContext {

    public:
        Builder& builder_;
        BaseContext(Builder& builder)
            : builder_(builder)
        {
        }
        json::Node Build() { return builder_.Build(); };
        DictValueContext Key(std::string key) { return builder_.Key(std::move(key)); }
        BaseContext Value(json::Value value) { return builder_.Value(std::move(value)); }
        DictKeyContext StartDict() { return builder_.StartDict(); }

        BaseContext EndDict() { return builder_.EndDict(); };
        ArrayItemContext StartArray() { return builder_.StartArray(); };
        BaseContext EndArray() { return builder_.EndArray(); };
    };

    class DictKeyContext : public BaseContext {
    public:
        DictKeyContext(BaseContext base)
            : BaseContext(base)
        {
        }
        Node Build() = delete;
        BaseContext Value(json::Value value) = delete;
        BaseContext EndArray() = delete;
        DictKeyContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };

    class DictValueContext : public BaseContext {
    public:
        DictValueContext(BaseContext base)
            : BaseContext(base) {};
        DictKeyContext Value(json::Value value) { return BaseContext::Value(std::move(value)); }
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;

        // сверху проверено
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base)
            : BaseContext(base) {};
        DictValueContext Key(std::string key) = delete;
        
        BaseContext EndDict() = delete;

        ArrayItemContext Value(json::Value value) { return BaseContext::Value(std::move(value)); }
        Node Build() = delete;
    };
};

} // конец namespace json
