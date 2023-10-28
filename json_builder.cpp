#include "json_builder.h"

namespace json {


Builder::BaseContext Builder::Value(json::Value val) {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.top()->IsDict() && last_op_ != "Key") {
            //return DictKeyContext (*this);
            throw std::logic_error ("Value dict error");
        }
        else if (!nodes_stack_.top()->IsArray() && last_op_ == "Value") {
            throw std::logic_error ("Value dict error");
        }
    }

    if (last_op_ != "Build" && last_op_ != "Key" && last_op_ != "StartArray" && last_op_ != "Value" && last_op_ != "StartDict") { 
        throw std::logic_error ("Value error");
    }

    
    if (last_op_ == "Build") {
        root_.GetValue() = std::move(val);
        nodes_stack_.push(&root_);
        last_op_ = "Value";
        //return BaseContext(*this);
    }
    else if (nodes_stack_.top()->IsArray()) {
            Node* temp = & (nodes_stack_.top()->AsArray().emplace_back(Node()));
            temp->GetValue() = std::move(val);
            last_op_ = "Value";
            return Builder::ArrayItemContext(*this);
    }
    else if (nodes_stack_.top()->IsDict()) {
        nodes_stack_.top()->AsDict()[key_].GetValue() = std::move(val);
        last_op_ = "Value";
        return Builder::DictValueContext(*this);
    }
    
    return Builder::BaseContext(*this);
    //return Builder::DictValueContext(*this);
}

Builder::DictValueContext Builder::Key(std::string str) {
    if (last_op_ == "Key" || !nodes_stack_.top()->IsDict()) {
       //return DictValueContext(*this);
        throw std::logic_error ("Key error");
    }

    key_= std::move(str);
    last_op_ = "Key";

    return Builder::DictValueContext (*this);
}


json::Node Builder::Build() {
    if (last_op_ == "Build") {
        throw std::logic_error ("Build не завершен");
    }
    else if (!nodes_stack_.empty() && last_op_ != "Value") {
        if (nodes_stack_.top()->IsArray() || nodes_stack_.top()->IsDict()) {
            throw std::logic_error ("Build не завершен");
        }

    }
return root_;
}


Builder::ArrayItemContext Builder::StartArray() {
    if (!nodes_stack_.empty()) {
        if (last_op_ == "Value" && (!nodes_stack_.top()->IsArray() || !nodes_stack_.top()->IsDict())) {
            throw std::logic_error ("StartArray error");
        }
    }
    if (last_op_ != "Build" && last_op_ != "Key" && last_op_ != "Value" && last_op_ != "StartArray"&& last_op_ != "StartDict") {
        throw std::logic_error ("StartArray error");
    }
   
    if ( last_op_ == "Build" ) {
        root_ = Array();
        nodes_stack_.push(&root_);
    }
    else if ( nodes_stack_.top()->IsArray() )  {
        // добавляем значение в массив
        
        // auto new_ptr = std::get<Array>(nodes_stack_.top()->GetValue()).emplace_back(Array());
        // nodes_stack_.emplace(&new_ptr);
        
          Node& temp = ((nodes_stack_.top())->AsArray()).emplace_back(Array());
          nodes_stack_.push(&temp);  
            // Array* new_ptr = &(std::get<Array>(nodes_stack_.top()->GetValue()));
            // new_ptr->emplace_back(Array());
            // Node* temp = &((nodes_stack_.top().));
            // nodes_stack_.push(temp);
    }
    else if ( last_op_ == "Key" ) { 

        nodes_stack_.top()->AsDict()[key_] = Node (Array());
        nodes_stack_.push(& (nodes_stack_.top()->AsDict()[key_])); 
    }
    last_op_ = "StartArray";
    

    return Builder::ArrayItemContext(*this);
}

Builder::BaseContext Builder::EndArray() {
    if (!nodes_stack_.top()->IsArray()) {
        throw std::logic_error ("EndArray error");
    }
    //last_op_ = "EndArray";
    nodes_stack_.pop();

    return Builder::BaseContext(*this);
}

Builder::DictKeyContext Builder::StartDict() {
    if (!nodes_stack_.empty()) {
        if (!nodes_stack_.top()->IsArray() && last_op_ == "Value") {
            throw std::logic_error ("StartDict error");
        }
    }
    else if (last_op_ != "Build" && last_op_ != "Key" && last_op_ != "Value" && last_op_ != "StartArray" && last_op_ != "StartDict") {
        throw std::logic_error ("StartDict error");
    }
    
    if ( last_op_ == "Build" ) {
        root_ = Dict();
        nodes_stack_.push(&root_);
    }
    else if (last_op_ == "Key") {
        //nodes_stack_.top()->AsDict()[key_].GetValue() = std::move(val);
        Node* temp = &((nodes_stack_.top())->AsDict())[key_];
        *temp = Node(Dict());

          nodes_stack_.push(temp); 
    }
    else if (nodes_stack_.top()->IsArray()) {
        nodes_stack_.top()->AsArray().emplace_back(Dict());
        nodes_stack_.push(& (nodes_stack_.top()->AsArray().back()));
    }

    last_op_ = "StartDict";
   
    return Builder::DictKeyContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error ("EndDict error");
    }
    else if (!nodes_stack_.top()->IsDict()) {
        throw std::logic_error ("EndDict error");
    }

    nodes_stack_.pop();
    
   return Builder::BaseContext(*this);
}


} // конец namespace json