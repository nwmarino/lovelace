//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_VISITOR_H_
#define LOVELACE_VISITOR_H_

namespace lace {

class AST;

class AliasDefn;
class EnumDefn;
class FieldDefn;
class FunctionDefn;
class LoadDefn;
class ParameterDefn;
class StructDefn;
class VariableDefn;
class VariantDefn;

class AdapterStmt;
class BlockStmt;
class IfStmt;
class RestartStmt;
class RetStmt;
class StopStmt;
class UntilStmt;
class RuneStmt;

class BoolLiteral;
class CharLiteral;
class IntegerLiteral;
class FloatLiteral;
class NullLiteral;
class StringLiteral;

class BinaryOp;
class UnaryOp;

class AccessExpr;
class CallExpr;
class CastExpr;
class ParenExpr;
class RefExpr;
class SizeofExpr;
class SubscriptExpr;

/// Abstract definition for a visitor pattern over the abstract syntax tree.
class Visitor {
protected:
    Visitor() = default;

public:
    virtual ~Visitor() = default;

    Visitor(const Visitor&) = delete;
    void operator=(const Visitor&) = delete;

    Visitor(Visitor&&) noexcept = delete;
    void operator=(Visitor&&) noexcept = delete;
    
    virtual void visit(AST& ast) {}

    virtual void visit(AliasDefn& node) {}
    virtual void visit(EnumDefn& node) {}
    virtual void visit(FieldDefn& node) {}
    virtual void visit(FunctionDefn& node) {}
    virtual void visit(LoadDefn& node) {}
    virtual void visit(ParameterDefn& node) {}
    virtual void visit(StructDefn& node) {}
    virtual void visit(VariableDefn& node) {}
    virtual void visit(VariantDefn& node) {}

    virtual void visit(AdapterStmt& node) {}
    virtual void visit(BlockStmt& node) {}
    virtual void visit(IfStmt& node) {}
    virtual void visit(RestartStmt& node) {}
    virtual void visit(RetStmt& node) {}
    virtual void visit(StopStmt& node) {}
    virtual void visit(UntilStmt& node) {}
    virtual void visit(RuneStmt& node) {}

    virtual void visit(BoolLiteral& node) {}
    virtual void visit(CharLiteral& node) {}
    virtual void visit(IntegerLiteral& node) {}
    virtual void visit(FloatLiteral& node) {}
    virtual void visit(NullLiteral& node) {}
    virtual void visit(StringLiteral& node) {}
    
    virtual void visit(BinaryOp& node) {}
    virtual void visit(UnaryOp& node) {}

    virtual void visit(AccessExpr& node) {}
    virtual void visit(CallExpr& node) {}
    virtual void visit(CastExpr& node) {}
    virtual void visit(SizeofExpr& node) {}
    virtual void visit(SubscriptExpr& node) {}
    virtual void visit(ParenExpr& node) {}
    virtual void visit(RefExpr& node) {}
};

} // namespace lace

#endif // LOVELACE_VISITOR_H_
