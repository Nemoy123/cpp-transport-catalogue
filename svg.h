#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <optional>
#include <variant>
#include <sstream>

using namespace std::literals;
namespace svg {

class Rgb {
    public:
        Rgb () {} 
        Rgb (uint8_t red_color, uint8_t green_color, uint8_t blue_color) : red(red_color), green(green_color), blue(blue_color){}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
};

class Rgba {
    public:
        Rgba () {}
        Rgba (uint8_t red_color, uint8_t green_color, uint8_t blue_color, double opacity_op) : red(red_color), green(green_color), blue(blue_color), opacity(opacity_op){}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
};

//using Color = std::string;
using Color = std::variant < std::monostate, std::string, svg::Rgb, svg::Rgba >;

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{"none"};


enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline std::ostream& operator<<(std::ostream& os, const StrokeLineCap& token)
{
  switch(token)
  {
    case StrokeLineCap::BUTT:
      os << "butt";
    break;
    case StrokeLineCap::ROUND:
      os << "round";
    break;
    case StrokeLineCap::SQUARE:
      os << "square";
    break;
  }
  return os;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& token)
{
  switch(token)
  {
    case StrokeLineJoin::ARCS:
      os << "arcs";
    break;
    case StrokeLineJoin::BEVEL:
      os << "bevel";
    break;
    case StrokeLineJoin::MITER:
      os << "miter";
    break;
    case StrokeLineJoin::MITER_CLIP:
      os << "miter-clip";
    break;
    case StrokeLineJoin::ROUND:
      os << "round";
    break;
  }
  return os;
}

struct ColorFabric {
    
    std::optional<std::string> operator()(std::monostate) const { 
        return "none"s; 
       //return std::nullopt;
        }
    std::optional<std::string> operator()(std::string str) const { 
        //return str == "NoneColor"s ? "none"s : str ;
        return str;
        }
    std::optional<std::string> operator()(Rgb rgb_) const { 
        std::stringstream sstm;
        sstm << "rgb("sv << static_cast <int> (rgb_.red) << ","sv << static_cast <int> (rgb_.green) << ","sv<< static_cast <int> (rgb_.blue) << ")"sv;
         return {sstm.str()}; 
    }
    std::optional<std::string> operator()(Rgba rgba_) const {
        std::stringstream sstm;
        sstm << "rgba("sv << static_cast <int> (rgba_.red) << ","sv 
             << static_cast <int> (rgba_.green) << ","sv<< static_cast <int> (rgba_.blue) 
             << ","sv << static_cast <double> (rgba_.opacity)  << ")"sv;
        return {sstm.str()}; 
    }
        
};

inline std::ostream& operator<<(std::ostream& out, const Color& col){
        auto temp_color = std::visit(ColorFabric{}, col);
        out << temp_color.value(); 
        return out;
}



template <typename Owner>
class PathProps {
public:
    

    //задаёт значение свойства fill — цвет заливки
    Owner& SetFillColor(Color color) {
        auto temp_color = std::visit(ColorFabric{}, color);
        fill_color_ = std::move(temp_color);
        return AsOwner();
    }
    //задаёт значение свойства stroke — цвет контура.
    Owner& SetStrokeColor(Color color) {
        auto temp_color = std::visit(ColorFabric{}, color);
        stroke_color_ = std::move(temp_color);
        return AsOwner();
    }
    //SetStrokeWidth(double width) задаёт значение свойства stroke-width — толщину линии. По умолчанию свойство не выводится.
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    //SetStrokeLineCap(StrokeLineCap line_cap) задаёт значение свойства stroke-linecap — тип формы конца линии. 
    //По умолчанию свойство не выводится.
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = std::move(line_cap);
        return AsOwner();
    }
    //SetStrokeLineJoin(StrokeLineJoin line_join) задаёт значение свойства stroke-linejoin — тип формы соединения линий. 
    //По умолчанию свойство не выводится.
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return AsOwner();
    }



protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_linecap_) {
            out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    

    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<std::string> fill_color_;
    std::optional<std::string> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> line_join_;

};

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}

    double x = 0;
    double y = 0;
    bool operator==(const Point& rhs) const {
          return this->x == rhs.x &&  this->y == rhs.y; 
    }
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};




/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
protected:
    // Этот конструктор доступен только классам-наследникам
    explicit Object(){}
    
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
    public:
        template <typename T>
        void Add (T& obj) {
            std::unique_ptr<T> p3 = std::make_unique<T>(obj);
            AddPtr ( std::move(p3) );
        }

        virtual ~ObjectContainer() = default;
    private:
       // std::deque <std::unique_ptr<Object>> conteiner_;
        // virtual void AddPtr (std::unique_ptr<Object>&& obj) {
        //     conteiner_.push_back(std::move (obj) );
        // }
        virtual void AddPtr (std::unique_ptr<Object>&& obj) = 0;

};

class Drawable { 
    public:
        
        virtual void Draw(ObjectContainer& g) const = 0;
        virtual ~Drawable() = default;
    private:

};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public svg::PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public svg::PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;


    std::deque <Point> points_;

};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public svg::PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;

    Point main_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t size_ = 1;
    std::string font_weight_;
    std::string font_family_;
    std::string data_;
    
};

//template <typename Item>
class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
     template <typename Item>
    void Add(Item one);

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document



private:
    
    std::deque<std::unique_ptr<Object>> conteiner_;

};

template <typename Item>
void Document::Add (Item one) {
    ObjectContainer::Add(one);
    //conteiner_.emplace_back(std::make_unique<Item>(std::move(one)));
}




}  // namespace svg