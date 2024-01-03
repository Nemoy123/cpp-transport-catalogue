#pragma once

#include "cell.h"
#include "common.h"


#include <functional>
#include <map>
#include <unordered_map>
#include <optional>

class Hasher {
public:
    size_t operator()(const Position& pos) const {
        return std::hash<int>{}(pos.col+17+17*17) + std::hash<int>{}(pos.row+43+43*43*43);
    }
};

class Sheet : public SheetInterface {
    using Value = std::variant<double, FormulaError>;
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами
    bool TestingCicling (const Position& pos) const;
    bool TestingCicling (const Position& pos,  std::set <Position>& test_set) const;

    std::optional<Value> GetValueFromCache (const Position& pos) const;
    void SetValuetoCache (const Position pos, Value val) const;

private:
	
    std::map <Position, std::unique_ptr<Cell>> base_ = {};
    mutable std::unordered_map <Position, Value, Hasher> cache_;
   // mutable std::map <Position, Value> cache_;
    
};

