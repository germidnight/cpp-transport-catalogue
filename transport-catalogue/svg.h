#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const std::string NoneColor{"none"};

    struct ColorToStringPrinter {
        std::string *color_str;
        void operator()(std::monostate) {
            *color_str = NoneColor;
        }
        void operator()(std::string color_string) {
            *color_str = color_string;
        }
        void operator()(Rgb rgb_color) {
            *color_str = "rgb(" + std::to_string(rgb_color.red) + ","
                        + std::to_string(rgb_color.green) + ","
                        + std::to_string(rgb_color.blue) + ")";
        }
        void operator()(Rgba rgba_color) {
            //std::string opacity_str = std::to_string(rgba_color.opacity);
            std::ostringstream out;
            out.precision(6);
            out << rgba_color.opacity;
            std::string opacity_str = out.str();
            *color_str = "rgba(" + std::to_string(rgba_color.red) + ","
                        + std::to_string(rgba_color.green) + ","
                        + std::to_string(rgba_color.blue) + ","
                        + opacity_str.erase(opacity_str.find_last_not_of('0') + 1, std::string::npos) + ")";
        }
    };

    struct Point {
        Point() = default;
        Point(double coord_x, double coord_y)
            : x(coord_x), y(coord_y) {
        }

        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream &out_stream)
            : out(out_stream) {
        }

        RenderContext(std::ostream &out_stream, int indent_step_val, int indent_val = 0)
            : out(out_stream), indent_step(indent_step_val), indent(indent_val) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        virtual void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream &operator<<(std::ostream &output, StrokeLineCap line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream &operator<<(std::ostream &output, StrokeLineJoin line_join);

    /*
     * Объект, который будет содержать свойства, управляющие параметрами заливки и контура
     */
    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            std::string color_str;
            std::visit(ColorToStringPrinter{&color_str}, color);
            fill_color_ = std::move(color_str);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            std::string color_str;
            std::visit(ColorToStringPrinter{&color_str}, color);
            stroke_color_ = std::move(color_str);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;
        /* Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke */
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if(fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if(stroke_color_) {
                out << " stroke=\"" << *stroke_color_ << "\""sv;
            }
            if(stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if(stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if(stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner(){
            /* static_cast безопасно преобразует *this к Owner&, если класс Owner — наследник PathProps */
            return static_cast<Owner&>(*this);
        }
        std::optional<std::string> fill_color_;
        std::optional<std::string> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    std::ostream &operator<<(std::ostream &output, Color color);

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);

    private:
        void RenderObject(const RenderContext &context) const override;

        std::vector<Point> points_;
    };

    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point position_;
        Point text_offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    /*
     * ObjectContainer задаёт интерфейс для доступа к контейнеру SVG-объектов.
     * Через этот интерфейс Drawable-объекты могут визуализировать себя, добавляя в контейнер SVG-примитивы.
     */
    class ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
        */
        template <typename ObjType>
        void Add(ObjType obj) {
            AddPtr(std::make_unique<ObjType>(std::move(obj)));
        }
    };

    /*
     * Интерфейс Drawable унифицирует работу с объектами, которые можно нарисовать, подключив SVG-библиотеку.
     */
    class Drawable {
    public:
        virtual void Draw(ObjectContainer &obj_cont) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) {
            objects_.emplace_back(std::move(obj));
        }

        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

} // namespace svg