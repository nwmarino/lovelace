//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_VISITOR_H_
#define SCC_VISITOR_H_

//
// This header file declares a class that provides virtual functions to 
// implement a visitor design pattern over the abstract syntax tree.
//

namespace scc {

class TranslationUnitDecl;
class VariableDecl;
class ParameterDecl;
class FunctionDecl;
class FieldDecl;
class TypedefDecl;
class RecordDecl;
class EnumVariantDecl;
class EnumDecl;

class CompoundStmt;
class DeclStmt;
class ExprStmt;
class IfStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class WhileStmt;
class ForStmt;
class CaseStmt;
class SwitchStmt;

class IntegerLiteral;
class FPLiteral;
class CharLiteral;
class StringLiteral;
class BinaryExpr;
class UnaryExpr;
class ParenExpr;
class RefExpr;
class CallExpr;
class CastExpr;
class SizeofExpr;
class SubscriptExpr;
class MemberExpr;
class TernaryExpr;

class Visitor {
protected:
    Visitor() = default;

public:
    virtual ~Visitor() = default;

    virtual void visit(TranslationUnitDecl &node) {}
    virtual void visit(VariableDecl &node) {}
    virtual void visit(ParameterDecl &node) {}
    virtual void visit(FunctionDecl &node) {}
    virtual void visit(FieldDecl &node) {}
    virtual void visit(TypedefDecl &node) {}
    virtual void visit(RecordDecl &node) {}
    virtual void visit(EnumVariantDecl &node) {}
    virtual void visit(EnumDecl &node) {}

    virtual void visit(CompoundStmt &node) {}
    virtual void visit(DeclStmt &node) {}
    virtual void visit(ExprStmt &node) {}
    virtual void visit(IfStmt &node) {}
    virtual void visit(ReturnStmt &node) {}
    virtual void visit(BreakStmt &node) {}
    virtual void visit(ContinueStmt &node) {}
    virtual void visit(WhileStmt &node) {}
    virtual void visit(ForStmt &node) {}
    virtual void visit(CaseStmt &node) {}
    virtual void visit(SwitchStmt &node) {}

    virtual void visit(IntegerLiteral &node) {}
    virtual void visit(FPLiteral &node) {}
    virtual void visit(CharLiteral &node) {}
    virtual void visit(StringLiteral &node) {}
    virtual void visit(BinaryExpr &node) {}
    virtual void visit(UnaryExpr &node) {}
    virtual void visit(ParenExpr &node) {}
    virtual void visit(RefExpr &node) {}
    virtual void visit(CallExpr &node) {}
    virtual void visit(CastExpr &node) {}
    virtual void visit(SizeofExpr &node) {}
    virtual void visit(SubscriptExpr &node) {}
    virtual void visit(MemberExpr &node) {}
    virtual void visit(TernaryExpr &node) {}
};

} // namespace scc

#endif // SCC_VISITOR_H_
