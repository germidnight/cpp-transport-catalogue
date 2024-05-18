/*
 * Простая библиотека для вывода векторных изображений в формате SVG.
 * Три типа объектов:
 * 1) Круг Circle. Описывается координатами центра и радиусом.
 * 2) Ломаная Polyline. Описывается координатами вершин.
 * 3) Текст Text. Описывается текстовым содержимым, координатами опорной точки, смещением относительно опорной точки, размером и названием шрифта.
 *
 * Вспомогательные классы:
 * - svg::Point — структура из двух полей x и y типа double. Должно быть два способа создать точку:
 * 1) использовать выражение Point{x, y};
 * 2) создать точку конструктором по умолчанию, а затем задать значения полей x и y прямым обращением к ним.
 * - svg::Object — абстрактный базовый класс для элементов, которые могут находиться внутри SVG-документа. Применяет паттерн проектирования «Шаблонный метод» для отображения тегов.
 * - svg::Document — класс, который производит отрисовку и компоновку SVG-документа. Класс должен поддерживать следующие операции:
 * 1) Создание с применением конструктора по умолчанию. svg::Document svg;
 * 2) Добавление объекта. svg.Add(object), где object имеет тип Circle, Polyline или Text, а также любой конкретный класс-наследник svg::Object.
 * Так поддерживается лишь линейная структура документа — составляющие его объекты по сути образуют массив;
 * 3) Добавление объекта. svg.AddPtr(unique_ptr<Object>&& object_ptr) добавляет unique_ptr, ссылающийся на любой класс-наследник svg::Object.
 * 4) Отрисовка (формирование результирующей строки). svg.Render(out), где out — наследник ostream.
 *
 * Классы Circle, Polyline и Text должны иметь конструктор по умолчанию и поддерживать описанные ниже методы. Умолчания — значения свойств, когда соответствующий Set-метод не вызывается.
 * - svg::Circle
 * 1) SetCenter(Point center): задаёт значения свойств cx и cy — координаты центра круга. Значение по умолчанию — точка с координатами {0.0, 0.0}.
 * 2) SetRadus(double radius): задаёт значение свойства r — радиус круга. Значение по умолчанию — 1.0.
 * - svg::Polyline
 * 1) AddPoint(Point p): добавляет вершину ломаной — элемент свойства points, записываемый в виде x,y и отделяемый пробелами от соседних элементов (см. примеры). Значение свойства по умолчанию — пустая строка.
 * - svg::Text
 * 1) SetPosition(Point pos): задаёт значения свойств x и y — координаты текста. Значение по умолчанию — точка с координатами {0.0, 0.0}.
 * 2) SetOffset(Point offset): задаёт значения свойств dx и dy — величины отступа текста от координаты. Значение по умолчанию — {0.0, 0.0}.
 * 3) SetFontSize(uint32_t size): задаёт значение свойства font-size — размер шрифта. Значение по умолчанию: 1.
 * 4) SetFontWeight(string font_weight): задаёт значение свойства font-weight — толщина шрифта. По умолчанию свойство не выводится.
 * 5) SetFontFamily(string font_family): задаёт значение свойства font-family — название семейства шрифта. По умолчанию свойство не выводится.
 * 6) SetData(string data): задаёт содержимое тега <text> — непосредственно выводимый текст. По умолчанию текст пуст.
 * Set-методы должны поддерживать method chaining (цепочки вызовов методов) — приём, который позволяет сократить запись нескольких вызов методов одного и того же объекта.
 * Реализуется он путём возврата ссылки на текущий экземпляр класса в Set-методах.
 *
 * Содержимое, выводимое методом svg::Document::Render, должно состоять из следующих частей:
 * 1) <?xml version="1.0" encoding="UTF-8" ?>
 * 2) <svg xmlns="http://www.w3.org/2000/svg" version="1.1">
 * 3) Объекты, добавленные с помощью svg::Document::Add, в порядке их добавления
 * 4) </svg>
 * Все свойства объектов выводятся в следующем формате: название свойства, символ =, затем значение свойства в кавычках.
 * Лишние пробельные символы допускаются:
 * 1) Между тегами, за исключением текста между открывающими и закрывающими тегами <text>.
 * 2) Между свойствами, перед и после списка свойств, вокруг символа =, разделяющего название свойства и значение.
 * 3) Между координатами в значении свойства points тега polyline должен быть ровно один пробел.  В начале и конце списка координат пробелов быть не должно: <polyline points="20,40 22.9389,45.9549 29.5106,46.9098" />
 *
 * - Круг отображается следующим образом: строка “<circle ”, затем через пробел свойства в произвольном порядке, затем строка “/>”.
 * - Ломаная отображается так: строка “<polyline ”, затем через пробел свойства в произвольном порядке, затем строка “/>”.
 * - Текст отображается следующим образом: строка “<text ”, затем через пробел свойства в произвольном порядке, затем символ “>”, содержимое надписи и, наконец, закрывающий тег “</text>”.
 *
 * Интерфейсы Drawable и ObjectContainer. Позаботьтесь, чтобы класс svg::Document являлся ObjectContainer-ом.
 * 1) Интерфейс Drawable унифицирует работу с объектами, которые можно нарисовать, подключив SVG-библиотеку. Для этого в нём есть метод Draw, принимающий ссылку на интерфейс ObjectContainer.
 * 2) ObjectContainer задаёт интерфейс для доступа к контейнеру SVG-объектов. Через этот интерфейс Drawable-объекты могут визуализировать себя, добавляя в контейнер SVG-примитивы.
 *
 * Добавьте к классам Text, Circle и Polyline SVG-библиотеки поддержку следующих методов:
 * 1) SetFillColor(Color color) задаёт значение свойства fill — цвет заливки. По умолчанию свойство не выводится.
 * 2) SetStrokeColor(Color color) задаёт значение свойства stroke — цвет контура. По умолчанию свойство не выводится.
 * 3) SetStrokeWidth(double width) задаёт значение свойства stroke-width — толщину линии. По умолчанию свойство не выводится.
 * 4) SetStrokeLineCap(StrokeLineCap line_cap) задаёт значение свойства stroke-linecap — тип формы конца линии. По умолчанию свойство не выводится.
 * 5) SetStrokeLineJoin(StrokeLineJoin line_join) задаёт значение свойства stroke-linejoin — тип формы соединения линий. По умолчанию свойство не выводится.
 * Эти методы должны поддерживать method chaining, то есть возвращать ссылку на тип объекта, у которого были вызваны.
 *
 * Добавьте новые способы задавать цвет:
 * Для этого объявите в библиотеке тип svg::Color как std::variant, который объединяет типы std::monostate, std::string, svg::Rgb и svg::Rgba.
 * Значение std::monostate обозначает отсутствующий цвет, который выводится в виде строки "none".
 * 1) svg::Rgb - Задаёт цвет в формате RGB, в виде трёх компонент red, green, blue типа uint8_t.
 * 2) svg::Rgba - Задаёт цвет в формате RGBA, в виде трёх компонент red, green, blue типа uint8_t и компоненты opacity типа double.
 * Компонента opacity или альфа-канал задаёт степень непрозрачности цвета от 0.0 (абсолютно прозрачно) до 1.0 (абсолютно непрозрачный цвет).
 */

#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<polyline points=\""sv;
        bool first_time = true;
        for(const Point point : points_) {
            if(!first_time) {
                out << " "sv;
            } else {
                first_time = false;
            }
            out << point.x << ","sv << point.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text &Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text &Text::SetOffset(Point offset) {
        text_offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text &Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text &Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << text_offset_.x << "\" dy=\""sv << text_offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if(!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if(!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        out << data_;
        out << "</text>"sv;
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream &out) const {
        const int ident = 1;
        const int ident_step = 1;
        RenderContext rndr(out, ident_step, ident);

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for(size_t i = 0; i != objects_.size(); ++i) {
            rndr.RenderIndent();
            objects_[i]->Render(rndr);
        }
        out << "</svg>"sv;
    }

    std::ostream &operator<<(std::ostream &output, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            output << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            output << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            output << "square"sv;
            break;
        }
        return output;
    }

    std::ostream &operator<<(std::ostream &output, StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            output << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            output << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            output << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            output << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            output << "round"sv;
            break;
        }
        return output;
    }

    std::ostream &operator<<(std::ostream &output, Color color) {
        std::string color_str;
        std::visit(ColorToStringPrinter{&color_str}, color);
        output << color_str;
        return output;
    }

} // namespace svg