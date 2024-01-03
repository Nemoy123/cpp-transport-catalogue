#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>


using namespace std::literals;

Sheet::~Sheet() {
    if (!base_.empty()) {
        for (auto& [pos, ref] : base_) {
            //for (auto& vect2 : vect1){
                ref.~unique_ptr();
           // }
        }
    } 
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException ("InvalidPositionException");
    }

    if (base_.empty()) {
        base_[{0, 0}] = nullptr;
    } 
    auto reserve_cell = std::move (base_[pos]);
    base_[pos] = std::make_unique<Cell> (text, this, pos);
    
    if (!TestingCicling (pos)) {
        //ClearCell(pos);
        base_[pos] = std::move (reserve_cell);
        throw CircularDependencyException ("Find cycle error!!!");
    }
   
    for (const auto& position : base_[pos].get()->GetReferencedCells()) {
        base_[pos].get()->SetForwardRelation(position);
        if (base_[position].get() == nullptr) {
            SetCell (position, "");
        }
        base_[position].get()->SetBackwardRelation(pos);
    }
   
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException ("InvalidPositionException");
    }
    if (base_.count(pos) < 1) {return nullptr;}
    else {
        if (base_.at(pos) == nullptr) {return nullptr;}
        if (base_.at(pos).get()->Empty()) {return nullptr;}
        return base_.at(pos).get() ;
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException ("InvalidPositionException");
    }
    
    if (base_.count(pos) < 1) {return nullptr;}
    else {
        if (base_.at(pos) == nullptr) {return nullptr;}
        if (base_.at(pos).get()->Empty()) {return nullptr;}
        return base_[pos].get() ;
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException ("InvalidPositionException");
    }
       
    if (base_.count(pos) > 0) {
        if (base_[pos].get() != nullptr) {
            base_[pos].get()->Clear();
        }
        base_.erase(pos);
    }

}

bool TestForNull(const std::vector< std::unique_ptr<Cell> >& vect) {
    for (const auto& item :vect) {
        if (item != nullptr) return false;
    }
    return true;
}

Size Sheet::GetPrintableSize() const {
    if (base_.empty()) return Size{0, 0};
    
    int max_col = 0;
    int max_row = 0;
    for (const auto& par : base_) {
        if (par.second.get() != nullptr) {
            if (!par.second.get()->Empty()) {
                if ( par.first.col > max_col ) {max_col = par.first.col;}
                if ( par.first.row > max_row ) {max_row = par.first.row;}
            }
        }
    }

    return Size{ max_row+1, max_col+1 };
}

// Выводит всю таблицу в переданный поток. Столбцы разделяются знаком
// табуляции. После каждой строки выводится символ перевода строки. Для
// преобразования ячеек в строку используются методы GetValue() или GetText()
// соответственно. Пустая ячейка представляется пустой строкой в любом случае.


void Sheet::PrintValues(std::ostream& output) const {
    const Size size_print = GetPrintableSize();
    if (size_print == Size{0,0}) return;
    for (auto i = 0; i < size_print.rows; ++i) {
        
        bool begin = true;
        for (auto t = 0; t < size_print.cols; ++t) {
            Position pos {i, t};
            
                if (!begin) {
                    output << '\t';
                    //std::cout << "\\t";
                }
                begin = false;
                if (base_.count(pos) > 0) {  
                    if (base_.at(pos).get() != nullptr) {
                        if (!base_.at(pos).get()->Empty()) {
                            auto val = base_.at(pos).get()->GetValue();
                            if (std::holds_alternative <std::string> (val)) {
                                output << std::get<std::string>(val);
                               // std::cout << std::get<std::string>(val);
                            }
                            else if (std::holds_alternative <double> (val)) {
                                output << std::get<double>(val);
                                //std::cout << std::get<double>(val);
                            }
                            else if (std::holds_alternative <FormulaError> (val)) {
                                output << std::get<FormulaError>(val);
                                //std::cout << std::get<FormulaError>(val);
                            }
                        }
                    }
                    else {
                        output << "";
                       // std::cout << "";
                    }
                } 
        }
        output << '\n';
        //std::cout << "\\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    const Size size_print = GetPrintableSize();
    if (size_print == Size{0,0}) return;
    bool begin = true;
    size_t count_row = 0;
    
    for (auto i = 0; i < size_print.rows; ++i) {
        if (!begin) {
            output << '\n';
            //std::cout << "\\n";
            }
        begin = false;
        bool begin_st = true;
        for (auto t = 0; t < size_print.cols; ++t) {
            Position pos {i, t};
            if (!begin_st) {
                output << '\t';
                //std::cout << "\\t";
            }
            begin_st = false;
            if (base_.count(pos) > 0) { 
                if (base_.at(pos).get() != nullptr) {
                    if (!base_.at(pos).get()->Empty()) {
                        auto val = base_.at(pos).get()->GetText();
                        output << val;
                        //std::cout << val;
                    }
                }
            }   
        }
        ++count_row;
    }
    output << '\n';
    //std::cout << "\\n";
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

std::optional<Sheet::Value> Sheet::GetValueFromCache (const Position& pos) const {
    if (!pos.IsValid()) {return std::nullopt;}
    auto iter = cache_.find(pos);
    if (iter != cache_.end()) {
        return iter->second;
    }
    else {return std::nullopt;}
}

bool Sheet::TestingCicling (const Position& pos) const {
    std::set <Position> test_set;
    return Sheet::TestingCicling (pos, test_set); 
}


bool Sheet::TestingCicling (const Position& pos,  std::set <Position>& test_set) const {
    // auto* base_ptr = &base_.at({0,0});
    
   // auto cell_ptr = pos->get();
    //std::set <Cell*> test_set;
    test_set.insert(pos);
    const CellInterface* cast_ptr = GetCell(pos);
    const Cell* cell_ptr = dynamic_cast <const Cell*> (cast_ptr);
    if (cell_ptr != nullptr) {
        for (const auto& cell_ptr_new : cell_ptr->GetReferencedCells()) {
            if (test_set.find(cell_ptr_new) != test_set.end()) {
                return false;
            }
            else {
                if ( !TestingCicling (cell_ptr_new, test_set) ) {
                    return false;
                }
            }
        }
    }
    return true;
}

void Sheet::SetValuetoCache (const Position pos, Value val) const {
    cache_[pos] = std::move(val);
}