#pragma once

#include "common.h"
#include "formula.h"
#include <memory>
#include <set>
//#include "sheet.h"

class Cell : public CellInterface {
public:
    //Cell();
    Cell(std::string& text, const SheetInterface* sheet, Position pos);
   // Cell(Sheet& sheet); // новый
    std::vector<Position> GetReferencedCells() const override; // новый
    ~Cell();

    void Set(std::string text);
    void Clear();
    void SetSheetPtr (const SheetInterface* sheet) {sheet_ = sheet;}
    void SetThisCellPosition (const Position& pos) {this_cell_position_ = pos;}
    void SetForwardRelation (const Position& pos) {forward_relation_.insert(pos);}
    void SetBackwardRelation (const Position& pos) {backward_relation_.insert(pos);}

    const SheetInterface* GetSheetPtr () {return sheet_;}
    Value GetValue() const override;
    std::string GetText() const override;
    const std::set <Position> GetForwardRelation () const {return forward_relation_;}

    bool Empty () const {
        if (impl_ == nullptr) return true;
        else  {return impl_.get()->Empty(); }
    }



private:
    std::set <Position> forward_relation_; // на кого ссылаетс
    std::set <Position> backward_relation_; // кто ссылается
    const SheetInterface* sheet_ = nullptr;
    Position this_cell_position_;
    
    class Impl {
        public:
            Impl (const SheetInterface* ptr, Position& pos) 
                    : ptr_(ptr)
                    , cell_pos (pos) 
                    {}
            virtual std::string GetTextImpl() = 0;
            virtual CellInterface::Value GetValueImpl() = 0;
            virtual bool Empty() const = 0;
            void SetPtr (SheetInterface* ptr) {ptr_ = ptr;}
            void SetPos (Position pos) {cell_pos = pos;}
            const SheetInterface* GetPtr () {return ptr_;}
            const Position& GetPos () const {return cell_pos;}
            
        private:
            const SheetInterface* ptr_;
            Position cell_pos;
            
    };

     class EmptyImpl : public Impl  {
        public:
            EmptyImpl (std::string& cont, const SheetInterface* ptr, Position& pos)
                        : Impl (ptr, pos)
                        , val_(cont)
                        , empty_(true)
                        {}
            EmptyImpl (const SheetInterface* ptr, Position& pos)
                        : Impl (ptr, pos)
                        , empty_(true)
                        {}

            std::string GetTextImpl() override {return "";}
            CellInterface::Value GetValueImpl() override {return {};}
            bool Empty() const override {return empty_;}
            //void SetPtr (SheetInterface* ptr) {ptr_ = ptr;}

        private:
            CellInterface::Value val_;
            bool empty_ = true;
            
            
            
     };
    class TextImpl : public Impl  {
        public:
            //TextImpl (std::string& cont):val_(cont), empty_(false){}
            TextImpl (std::string& cont, const SheetInterface* ptr, Position& pos)
                      : Impl (ptr, pos)
                      , val_(cont)
                      , empty_(false)
                      {}

            std::string GetTextImpl() override {return std::get<std::string>(val_);}
            CellInterface::Value GetValueImpl() override {
                auto temp = std::get<std::string>(val_);
                if (temp[0] == 39) {        // апостроф '
                     return temp.substr(1);           
                } else { return temp; }
            }
            bool Empty() const override {return empty_;}
                
        private:
            CellInterface::Value val_;
            bool empty_ = true;
            //SheetInterface* ptr_;
            
            
    };
    class FormulaImpl: public Impl  {
        public:
            
            // FormulaImpl (std::string& cont) 
            //             : formula_(ParseFormula(cont.substr(1)))
            //             , empty_(false)
                        
            // {
            //     // auto temp = formula_.get()->Evaluate(*ptr_); //this->GetPtr()
            //     // if ( std::holds_alternative<double>(temp) ) { val_ = std::get<double>(temp);  }
            //     //  else { val_ = std::get<FormulaError>(temp); }
            // }
            FormulaImpl (std::string& cont, const SheetInterface* ptr, Position& pos) 
                        : Impl (ptr, pos)
                         
                        , empty_(false)
                        {   
                            try {
                                formula_ = ParseFormula(cont.substr(1)); 
                            }   
                            catch (...) {
                                throw FormulaException ("Formula error");
                            }
                             auto temp_list = formula_.get()->GetReferencedCells();
                           
                        }

            std::string GetTextImpl() override {return "=" + formula_.get()->GetExpression();} 
            CellInterface::Value GetValueImpl() override { 
                
                auto temp = formula_.get()->Evaluate(*GetPtr()); //this->GetPtr()
                if ( std::holds_alternative<double>(temp) ) { val_ = std::get<double>(temp);  }
                else { val_ = std::get<FormulaError>(temp); }

                return val_;
            }  
            bool Empty() const override {return empty_;}
            std::vector<Position> GetReferencedCells() const {return formula_.get()->GetReferencedCells();}    
        private:
            std::unique_ptr<FormulaInterface> formula_;
            CellInterface::Value val_;
            bool empty_ = true;
            //SheetInterface* ptr_;
           

            
    };

    std::unique_ptr<Impl> impl_;


};