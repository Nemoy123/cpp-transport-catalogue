#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <set>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
                    : ast_(ParseFormulaAST(std::move(expression)))
                    {}
    Value Evaluate(const SheetInterface& sheet) const override {
        //const Sheet* sheet = dynamic_cast <const Sheet*> (&sheet);
        
        try {
            const auto* ptr_sheet = &sheet;
            return ast_.Execute(ptr_sheet);
        }
        catch (FormulaError& ex) {
            return ex.GetCategory();
        }
        
        

    }
    std::string GetExpression() const override {
        std::stringstream stroke;
        ast_.PrintFormula(stroke);
        return stroke.str();
    }
    std::vector<Position> GetReferencedCells() const override {
        std::set <Position> result {ast_.GetCells().begin(), ast_.GetCells().end()};
        return std::vector <Position> {result.begin(), result.end()};
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}