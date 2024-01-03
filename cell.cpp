#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
// Cell::Cell():impl_(std::make_unique<EmptyImpl>(sheet_, this_cell_position_)) {
//     //impl_=std::make_unique<EmptyImpl>();
// }
Cell::Cell(std::string& text, const SheetInterface* sheet, Position pos)
{
    SetSheetPtr (sheet);
    SetThisCellPosition (pos);
    impl_ = std::make_unique<EmptyImpl>(sheet_, this_cell_position_);
    Set(text);
}
Cell::~Cell() {
    impl_.~unique_ptr();
}

void Cell::Set(std::string text) {
    if (text[0] == '=' && text.size() > 1) {
       impl_=std::make_unique<FormulaImpl>(text, sheet_, this_cell_position_);
    }
    else {
        impl_=std::make_unique<TextImpl>(text, sheet_, this_cell_position_);
    }
}

void Cell::Clear() {
    impl_.~unique_ptr();
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    return impl_.get()->GetValueImpl();
}
std::string Cell::GetText() const {
    return impl_.get()->GetTextImpl();
}

std::vector<Position> Cell::GetReferencedCells() const {
    FormulaImpl* form_ptr = dynamic_cast<FormulaImpl*>(impl_.get());
    if (form_ptr != nullptr) {return  form_ptr->GetReferencedCells();} 
    else {
        return {};
    }
}