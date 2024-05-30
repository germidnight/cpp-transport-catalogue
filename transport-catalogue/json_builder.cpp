#include "json_builder.h"

namespace json {

    void Builder::ThrowIfReadyObject(std::string func_name) {
        if (nodes_stack_.empty()) {
            std::string err_str = "Calling method " + func_name + " on a ready object / Вызов метода " + func_name + " при готовом объекте";
            throw std::logic_error(err_str);
        }
    }

    Builder::Builder() {
        nodes_stack_.emplace_back(&root_);
    }

    KeyContext Builder::Key(const std::string &key) {
        ThrowIfReadyObject("Key()");

        if (nodes_stack_.back()->IsMap()) {
            if(!last_key_.has_value()) {
                std::get<Dict>(nodes_stack_.back()->GetValue())[key];
                last_key_ = key;
            } else {
                throw std::logic_error("Calling Key() immediately after the previous Key() call / Вызов метода Key() сразу после предыдущего вызова Key()");
            }
        } else {
            throw std::logic_error("Calling Key() method not for Dict / Вызов метода Key() не для словаря");
        }
        return *this;
    }
    Builder &Builder::Value(Node::Value &&value) {
        ThrowIfReadyObject("Value()");

        if (nodes_stack_.back()->IsArray()) {
            Node temp_node;
            temp_node.GetValue() = value;
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(temp_node);
            return *this;
        }
        if (nodes_stack_.back()->IsMap()) {
            Dict &temp = std::get<Dict>(nodes_stack_.back()->GetValue());
            if (last_key_.has_value() && (temp.count(last_key_.value()) > 0)) {
                temp.at(last_key_.value()).GetValue() = value;
                last_key_.reset();
                return *this;
            } else {
                throw std::logic_error("Calling Value() in Dict without the previous Key() call / Вызов метода Value() в Dict без предыдущего вызова Key()");
            }
        }
        nodes_stack_.back()->GetValue() = value;
        nodes_stack_.pop_back();

        return *this;
    }
    DictItemContext Builder::StartDict() {
        ThrowIfReadyObject("StartDict()");

        if(nodes_stack_.back()->IsArray()) {
            auto &temp_node = std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Node{Dict{}});
            nodes_stack_.emplace_back(&temp_node);
        } else if (nodes_stack_.back()->IsMap()) {
            Dict &dict_temp = std::get<Dict>(nodes_stack_.back()->GetValue());
            if ((last_key_.has_value()) && (dict_temp.count(last_key_.value()) > 0)) {
                auto &temp_node = dict_temp.at(last_key_.value()) = std::move(Node{Dict{}});
                nodes_stack_.emplace_back(&temp_node);
                last_key_.reset();
            } else {
                throw std::logic_error("Calling StartDict() inside the Dict without the previous Key() call / Вызов метода StartDict() в Dict без предыдущего вызова Key()");
            }
        } else if (nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->GetValue() = Dict{};
        }
        return *this;
    }
    Builder &Builder::EndDict() {
        ThrowIfReadyObject("EndDict()");
        if (!(nodes_stack_.back()->IsMap())) {
            throw std::logic_error("Calling EndDict() method for wrong container / Вызов EndDict() не для контейнера Dict");
        }
        nodes_stack_.pop_back();
        return *this;
    }
    ArrayItemContext Builder::StartArray() {
        ThrowIfReadyObject("StartArray()");

        if (nodes_stack_.back()->IsArray()) {
            auto &temp_node = std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Node{Array{}});
            nodes_stack_.emplace_back(&temp_node);
        } else if (nodes_stack_.back()->IsMap()) {
            Dict &temp_dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            if ((last_key_.has_value()) && (temp_dict.count(last_key_.value()) > 0)) {
                auto &temp_node = temp_dict.at(last_key_.value()) = std::move(Node{Array{}});
                nodes_stack_.emplace_back(&temp_node);
                last_key_.reset();
            } else {
                throw std::logic_error("Calling StartArray() inside the Dict without the previous Key() call / Вызов метода StartArray() в Dict без предыдущего вызова Key()");
            }
        } else if (nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->GetValue() = Array{};
            return *this;
        }
        return *this;
    }
    Builder &Builder::EndArray() {
        ThrowIfReadyObject("EndArray()");
        if (!(nodes_stack_.back()->IsArray())) {
            throw std::logic_error("Calling EndArray() method for wrong container / Вызов EndArray() не для контейнера Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }
    Node &Builder::Build() {
        if(!nodes_stack_.empty()) {
            throw std::logic_error("Unfinished arrays and dictionaries / Незаконченные массивы и словари");
        }
        return root_;
    }

    /* --------------------------- Context ------------------------------- */

    KeyContext BaseContext::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder &BaseContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    DictValueContext KeyContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    DictItemContext BaseContext::StartDict() {
        return builder_.StartDict();
    }

    Builder &BaseContext::EndDict() {
        return builder_.EndDict();
    }

    ArrayItemContext BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Node &BaseContext::Build() {
        return builder_.Build();
    }

} // namespace json