#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
  
    // Делегируем вывод тега своим подклассам
    RenderObject(context);
    
    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle "sv;

    this->RenderAttrs(out);
    
    out <<" cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline "sv;

    //this->RenderAttrs(out);

    out << " points=\""sv; 
    size_t count = 0;
    const size_t size = points_.size();
    for (auto& point : points_) {
         out << point.x << "," << point.y;
         if ( count + 1 < size ) {
            out << " ";
         }
         ++count;
    }
    out << "\""sv; 
    this->RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
        main_ = pos;
        return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
}

    // Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
        size_ = size;
        return *this;
}

    // Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
        font_family_ = font_family;
        return *this;
}
    // Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
        font_weight_ = font_weight;
        return *this;
}

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
        data_ = data;
        return *this;
}

std::string TextChange (const std::string& data) {
    std::string result;
    if (data.empty()) {return result;}
    
    for (const auto& ch : data) {
        if ( ch == 34) {
            std::string ch34 {"&quot;"s};
            result += ch34;
            continue;
        }
        if ( ch == 39) {
            //result.push_back("&apos;"s);
             std::string ch39 {"&apos;"s};
             result += ch39;
             continue;
        }
        if ( ch == 60) {
            // result.push_back("&lt;"s);
            std::string ch39 {"&lt;"s};
             result += ch39;
             continue;
        }
        if ( ch == 62) {
            // result.push_back("&gt;"s);
            std::string ch39 {"&gt;"s};
             result += ch39;
             continue;
        }
        if ( ch == 38) {
            // result.push_back("&amp;"s);
            std::string ch39 {"&amp;"s};
             result += ch39;
             continue;
        }

        result.push_back(ch);
    }
    

    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;

    this->RenderAttrs(out);

    out << " x=\""sv << main_.x << "\" y=\""sv << main_.y << "\" "sv;
    out << "dx=\"" << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv<< size_ << "\"";
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv<< font_weight_ << "\""sv;
    }
    if (!font_family_.empty()) {
    out << " font-family=\""sv<< font_family_ << "\""sv;
    }
    out << ">";
    out << TextChange(data_);
    out << "</text>"sv;

}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    //ObjectContainer::AddPtr(std::move (obj));
    conteiner_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
      
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    
    for (auto& elem : conteiner_) {
         elem->Render(out);
    }
    out << "</svg>"sv;
}

}  // namespace svg