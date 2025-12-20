//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_VISITOR_H_
#define STATIM_VISITOR_H_

namespace stm {

class LoadDecl;
class TranslationUnitDecl;
class VariableDecl;
class ParameterDecl;
class FunctionDecl;
class FieldDecl;
class VariantDecl;
class AliasDecl;
class StructDecl;
class EnumDecl;

class AsmStmt;
class BlockStmt;
class DeclStmt;
class RetStmt;
class IfStmt;
class WhileStmt;
class BreakStmt;
class ContinueStmt;

class BoolLiteral;
class IntegerLiteral;
class FPLiteral;
class CharLiteral;
class StringLiteral;
class NullLiteral;
class BinaryOp;
class UnaryOp;
class CastExpr;
class ParenExpr;
class SizeofExpr;
class AccessExpr;
class SubscriptExpr;
class DeclRefExpr;
class CallExpr;

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
    
    virtual void visit(LoadDecl& node) {}
    virtual void visit(TranslationUnitDecl& node) {}
    virtual void visit(VariableDecl& node) {}
    virtual void visit(ParameterDecl& node) {}
    virtual void visit(FunctionDecl& node) {}
    virtual void visit(FieldDecl& node) {}
    virtual void visit(VariantDecl& node) {}
    virtual void visit(AliasDecl& node) {}
    virtual void visit(StructDecl& node) {}
    virtual void visit(EnumDecl& node) {}

    virtual void visit(AsmStmt& node) {}
    virtual void visit(BlockStmt& node) {}
    virtual void visit(DeclStmt& node) {}
    virtual void visit(RetStmt& node) {}
    virtual void visit(IfStmt& node) {}
    virtual void visit(WhileStmt& node) {}
    virtual void visit(BreakStmt& node) {}
    virtual void visit(ContinueStmt& node) {}

    virtual void visit(BoolLiteral& node) {}
    virtual void visit(IntegerLiteral& node) {}
    virtual void visit(FPLiteral& node) {}
    virtual void visit(CharLiteral& node) {}
    virtual void visit(StringLiteral& node) {}
    virtual void visit(NullLiteral& node) {}
    virtual void visit(BinaryOp& node) {}
    virtual void visit(UnaryOp& node) {}
    virtual void visit(CastExpr& node) {}
    virtual void visit(ParenExpr& node) {}
    virtual void visit(SizeofExpr& node) {}
    virtual void visit(AccessExpr& node) {}
    virtual void visit(SubscriptExpr& node) {}
    virtual void visit(DeclRefExpr& node) {}
    virtual void visit(CallExpr& node) {}
};

} // namespace stm

#endif // STATIM_VISITOR_H_
