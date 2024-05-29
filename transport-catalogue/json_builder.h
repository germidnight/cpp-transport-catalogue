#pragma once

/*
 * Реализуйте класс json::Builder, позволяющий сконструировать JSON-объект, используя цепочки вызовов методов.
 * Методы класса json::Builder:
 * - Key(std::string). При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение.
 * Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода Value
 * или начинать его определение с помощью StartDict или StartArray.
 * - Value(Node::Value). Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или,
 * если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта.
 * Может принимать как простой объект — число или строку — так и целый массив или словарь.
 * Здесь Node::Value — это синоним для базового класса Node, шаблона variant с набором возможных типов-значений.
 * - StartDict(). Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value.
 * Следующим вызовом обязательно должен быть Key или EndDict.
 * - StartArray(). Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value.
 * Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value, StartDict или StartArray.
 * - EndDict(). Завершает определение сложного значения-словаря. Последним незавершённым вызовом Start* должен быть StartDict.
 * - EndArray(). Завершает определение сложного значения-массива. Последним незавершённым вызовом Start* должен быть StartArray.
 * - Build(). Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов.
 * К этому моменту для каждого Start* должен быть вызван соответствующий End*. При этом сам объект должен быть определён,
 * то есть вызов json::Builder{}.Build() недопустим.
 *
 * Описанный синтаксис позволяет указывать ключи словаря в определённом порядке. Тем не менее, в данном случае это учитывать не нужно.
 * Словари всё так же должны храниться с помощью контейнера map.
 *
 * В случае использования методов в неверном контексте ваш код должен выбросить исключение типа std::logic_error с понятным сообщением об ошибке.
 * Это должно происходить в следующих ситуациях:
 * 1) Вызов метода Build при неготовом описываемом объекте, то есть сразу после конструктора или при незаконченных массивах и словарях.
 * 2) Вызов любого метода, кроме Build, при готовом объекте.
 * 3) Вызов метода Key снаружи словаря или сразу после другого Key.
 * 4) Вызов Value, StartDict или StartArray где-либо, кроме как после конструктора, после Key или после предыдущего элемента массива.
 * 5) Вызов EndDict или EndArray в контексте другого контейнера.
 *
 * Методы класса должны иметь амортизированную линейную сложность относительно размера входных данных.
 * Исключение — дополнительный логарифмический множитель при добавлении в словарь.
 * Принимайте тяжёлые объекты в методах таким образом, чтобы при вызове этих методов объекты можно было переместить.
 * Например, принимайте по значению и перемещайте в коде метода.
 *
 * Некоторые явные ошибки должны находиться на этапе компиляции.
 * Код не должен компилироваться в следующих ситуациях:
 * 1) Непосредственно после Key вызван не Value, не StartDict и не StartArray.
 * 2) После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.
 * 3) За вызовом StartDict следует не Key и не EndDict.
 * 4) За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.
 * 5) После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.
 *
 * Останутся ошибки, которые продолжат выявляться лишь на этапе запуска:
 * 1) Вызов некорректного метода сразу после создания json::Builder.
 * 2) Вызов некорректного метода после End*.
 */

#include "json.h"

#include <optional>
#include <string>
#include <vector>

namespace json {

    struct DictItemContext;
    struct ArrayItemContext;
    struct KeyContext;
    struct DictValueContext;
    struct ArrayValueContext;

    class Builder {
    public:
        Builder();
        Builder(const Builder &) = delete;
        Builder(Builder &&) = default;
        ~Builder() = default;

        Builder& operator=(const Builder&) = delete;
        Builder& operator=(Builder&&) = default;

        KeyContext Key(const std::string &);
        Builder &Value(Node::Value &&value);
        DictItemContext StartDict();
        Builder &EndDict();
        ArrayItemContext StartArray();
        Builder &EndArray();
        Node &Build();

    private:
        void ThrowIfReadyObject(std::string func_name);

        Node root_;
        std::vector<Node *> nodes_stack_; /*стек указателей на те вершины JSON, которые ещё не построены*/
        std::optional<std::string> last_key_;
    };

    struct BaseContext {
        BaseContext(Builder &builder) : builder_(builder) {}

        KeyContext Key(std::string key);
        Builder &Value(Node::Value value);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        Node& Build();

        Builder &builder_;
    };

    /*За вызовом StartDict следует не Key и не EndDict.*/
    struct DictItemContext : BaseContext {

        DictItemContext(Builder &builder) : BaseContext(builder) {}

        // KeyContext Key(std::string key) = delete;
        Builder &Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        // Builder& EndDict() = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        Node& Build() = delete;
    };

    /*За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.*/
    struct ArrayItemContext : BaseContext {
        ArrayItemContext(Builder &builder) : BaseContext(builder) {}

        KeyContext Key(std::string key) = delete;
        ArrayValueContext Value(Node::Value value);
        // DictItemContext StartDict() = delete;
        Builder& EndDict() = delete;
        // ArrayItemContext StartArray() = delete;
        // Builder &EndArray() = delete;
        Node &Build() = delete;
    };

    /*Непосредственно после Key вызван не Value, не StartDict и не StartArray.*/
    struct KeyContext : BaseContext {
        KeyContext(Builder &builder) : BaseContext(builder) {}

        KeyContext Key(std::string key) = delete;
        DictValueContext Value(Node::Value value);
        // DictItemContext StartDict() = delete;
        Builder& EndDict() = delete;
        // ArrayItemContext StartArray() = delete;
        Builder &EndArray() = delete;
        Node &Build() = delete;
    };

    /*После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.*/
    struct DictValueContext : BaseContext {
        DictValueContext(Builder &builder) : BaseContext(builder) {}

        // KeyContext Key(std::string key) = delete;
        Builder &Value(Node::Value value) = delete; // !!!!!!!!!!!!!!!!!! BaseContext&   ???????????????
        DictItemContext StartDict() = delete;
        // Builder& EndDict() = delete;
        ArrayItemContext StartArray() = delete;
        Builder &EndArray() = delete;
        Node &Build() = delete;
    };

    /*После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.*/
    struct ArrayValueContext : BaseContext {
        ArrayValueContext(Builder &builder) : BaseContext(builder) {}

        KeyContext Key(std::string key) = delete;
        ArrayValueContext Value(Node::Value value);
        // DictItemContext StartDict() = delete;
        Builder& EndDict() = delete;
        // ArrayItemContext StartArray() = delete;
        // Builder &EndArray() = delete;
        Node &Build() = delete;
    };
} // namespace json