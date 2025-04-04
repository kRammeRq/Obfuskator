#ifndef MYASTCLASS_H
#define MYASTCLASS_H
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <fstream>
#include <iostream>
#include <QDebug>
#include <unordered_map>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Parse/ParseAST.h>
#include <llvm/Support/CommandLine.h>
#include "clang/Rewrite/Core/Rewriter.h"
#include <clang/AST/ParentMapContext.h>
using namespace clang;
using namespace clang::tooling;
//QStringList globalCheckedItems;

QString toBinaryString(int value);

// Функция для проверки, находится ли код в проекте
// bool isUserCode(SourceLocation Loc, ASTContext *Context) {
//     SourceManager &SM = Context->getSourceManager();
//     std::string fileName = SM.getFilename(Loc).str();
//     std::string projectDir = "/home/krammer/test/"; // Укажите свою директорию проекта

//     return fileName.rfind(projectDir, 0) == 0;
// }
bool isUserCode(clang::SourceLocation Loc, clang::ASTContext *Context);

// Класс для обхода AST
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
    //explicit MyASTVisitor(ASTContext *Context) : Context(Context) {}
    explicit MyASTVisitor(ASTContext *Context, Rewriter &codeRewriter, std::unordered_map<std::string, QString> &obfuscationMap)
        : Context(Context), codeRewriter(codeRewriter), obfuscationMap(obfuscationMap), counter(0) {}

    // Добавляем обработку числовых литералов
    bool VisitIntegerLiteral(IntegerLiteral *literal) {
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(literal->getBeginLoc());
        if (!globalCheckedItems.contains("Numbers")) {
            return true;
        }
        if (!loc.isValid()) {
            qWarning() << "Invalid location for integer literal!";
            return true;
        }

        // Получаем значение литерала
        llvm::APInt value = literal->getValue();
        int intValue = value.getSExtValue();

        // Преобразуем в двоичное представление
        QString binaryStr = toBinaryString(intValue);

        // Заменяем в коде
        codeRewriter.ReplaceText(loc, binaryStr.toStdString());
        qDebug() << "Replaced integer literal:" << intValue << "->" << binaryStr;

        return true;
    }

    // void RemoveComments() {
    //     SourceManager &SM = Context->getSourceManager();
    //     LangOptions LangOpts = Context->getLangOpts();

    //     for (auto It = SM.fileinfo_begin(); It != SM.fileinfo_end(); ++It) {
    //         //const FileEntry *FE = It->first.getFileEntry();
    //         const FileEntry *FE = &It->first.getFileEntry();
    //         if (!FE) continue;

    //         FileID FID = SM.translateFile(FE);
    //         if (!FID.isValid()) continue;

    //         // Проверяем, загружен ли буфер
    //         //const llvm::MemoryBuffer *MB = SM.getBuffer(FID);
    //         //const llvm::MemoryBuffer *MB = SM.getBufferOrFake(FID);
    //         llvm::MemoryBufferRef MB = SM.getBufferOrFake(FID);
    //         llvm::StringRef BufferData = MB.getBuffer();

    //         if (!MB) {
    //             qWarning() << "Buffer not found for file!";
    //             continue;
    //         }

    //         Lexer Lex(FID, MB->getMemBufferRef(), SM, LangOpts);
    //         Token Tok;

    //         while (!Lex.LexFromRawLexer(Tok)) {
    //             if (Tok.is(tok::comment)) {
    //                 SourceLocation StartLoc = Tok.getLocation();
    //                 SourceLocation EndLoc = Lexer::getLocForEndOfToken(StartLoc, 1, SM, LangOpts);

    //                 qDebug() << "Removing comment at" << SM.getFilename(StartLoc).str().c_str();
    //                 codeRewriter.RemoveText(SourceRange(StartLoc, EndLoc));
    //             }
    //         }
    //     }
    // }
    bool VisitMemberExpr(MemberExpr *member) {
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(member->getBase()->getBeginLoc());
        if (!loc.isValid()) {
            qWarning() << "Invalid location!";
            return true;
        }
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }

        if (!loc.isValid()) return true;

        QString baseName = QString::fromStdString(member->getBase()->IgnoreImpCasts()->getStmtClassName());
        if (auto *declRef = dyn_cast<DeclRefExpr>(member->getBase()->IgnoreImpCasts())) {
            baseName = QString::fromStdString(declRef->getNameInfo().getAsString());

            // Если переменная переименована, заменяем её
            if (obfuscationMap.find(baseName.toStdString()) != obfuscationMap.end()) {
                QString obfuscatedName = obfuscationMap[baseName.toStdString()];
                qDebug() << "Replacing object usage in method call: " << baseName << " -> " << obfuscatedName;
                codeRewriter.ReplaceText(loc, obfuscatedName.toStdString());
            }
        }

        return true;
    }

    bool VisitVarDecl(VarDecl *var) {
        //if (!isUserCode(var->getLocation(), Context)) return true; // Пропускаем системные файлы
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(var->getLocation());

        if (!loc.isValid()) {
            qWarning() << "Invalid location!";
            return true;
        }
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        QString name = QString::fromStdString(var->getNameAsString());
        // Пропускаем переменные с именами "argc", "argv", "main"
        if (name == "argc" || name == "argv" || name == "main") {
            qDebug() << "Skipping variable:" << name;
            return true; // Пропускаем эту переменную
        }
        QString type = QString::fromStdString(var->getType().getAsString());
        QString obfuscatedName = getObfuscatedName(name);
        //qDebug() << "Found variable:" << name << "of type" << type;
        qDebug() << "Found variable:" << name << "->" << getObfuscatedName(name);
        // Здесь заменяем имя в исходном коде
        ReplaceName(var, obfuscatedName);
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *func) {
        //if (!isUserCode(func->getLocation(), Context)) return true;
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(func->getLocation());

        if (!loc.isValid()) {
            qWarning() << "Invalid location!";
            return true;
        }
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        QString name = QString::fromStdString(func->getNameAsString());
        // Пропускаем замену имени функции, если она называется "main"
        if (name == "main") {
            return true;
        }
        QString returnType = QString::fromStdString(func->getReturnType().getAsString());
        QString obfuscatedName = getObfuscatedName(name);
        //qDebug() << "Found function:" << name << "with return type" << returnType;
        qDebug() << "Found function:" << name << "->" << getObfuscatedName(name);

        // Заменяем имя функции в исходном коде
        ReplaceName(func, obfuscatedName);
        for (unsigned i = 0; i < func->getNumParams(); ++i) {
            ParmVarDecl *param = func->getParamDecl(i);
            QString paramName = QString::fromStdString(param->getNameAsString());
            // Пропускаем argc и argv
            if (paramName == "argc" || paramName == "argv") {
                qDebug() << "Skipping parameter:" << paramName;
                continue; // Пропускаем параметр argc и argv
            }
            QString paramType = QString::fromStdString(param->getType().getAsString());
            //qDebug() << "  Parameter:" << paramName << "of type" << paramType;
            qDebug() << "  Parameter:" << paramName << "->" << getObfuscatedName(name);
        }

        return true;
    }

    bool VisitCXXMethodDecl(CXXMethodDecl *method) {
        //if (!isUserCode(method->getLocation(), Context)) return true;
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        QString name = QString::fromStdString(method->getNameAsString());
        QString returnType = QString::fromStdString(method->getReturnType().getAsString());
        QString obfuscatedName = getObfuscatedName(name);
        //qDebug() << "Found method:" << name << "with return type" << returnType;
        qDebug() << "Found method:" << name << "->" << getObfuscatedName(name);

        // Заменяем имя метода
        ReplaceName(method, obfuscatedName);
        for (unsigned i = 0; i < method->getNumParams(); ++i) {
            ParmVarDecl *param = method->getParamDecl(i);
            QString paramName = QString::fromStdString(param->getNameAsString());
            QString paramType = QString::fromStdString(param->getType().getAsString());
            //qDebug() << "  Parameter:" << paramName << "of type" << paramType;
            qDebug() << "  Parameter:" << paramName << "->" << getObfuscatedName(name);
        }

        return true;
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *record) {
        if (record->isClass() || record->isStruct()) {
            //if (!isUserCode(record->getLocation(), Context)) return true;
            if (!globalCheckedItems.contains("Name variable")) {
                return true;
            }
            QString name = QString::fromStdString(record->getNameAsString());
            QString obfuscatedName = getObfuscatedName(name);  // Обфусцируем имя
            //qDebug() << "Found class/struct:" << name;
            qDebug() << "Found class/struct:" << name << "->" << getObfuscatedName(name);
            // Заменяем имя в коде
            ReplaceName(record, obfuscatedName);
        }
        return true;
    }


    bool VisitEnumDecl(EnumDecl *E) {
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        QString name = QString::fromStdString(E->getNameAsString());
        QString obfuscatedName = getObfuscatedName(name);  // Обфусцируем имя
        //qDebug() << "Enum:" << name;
        qDebug() << "Enum:" << name << "->" << getObfuscatedName(name);
        // Заменяем имя в коде
        ReplaceName(E, obfuscatedName);
        return true;
    }

    bool VisitEnumConstantDecl(EnumConstantDecl *E) {
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        QString name = QString::fromStdString(E->getNameAsString());
        QString obfuscatedName = getObfuscatedName(name);  // Обфусцируем имя
        //qDebug() << "  Enum Constant:" << name;
        qDebug() << "  Enum Constant:" << name << "->" << getObfuscatedName(name);
        // Заменяем имя в коде
        ReplaceName(E, obfuscatedName);
        return true;
    }

    bool VisitCallExpr(CallExpr *call) {
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(call->getExprLoc());
        if (!loc.isValid()) {
            qWarning() << "Invalid location!";
            return true;
        }
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        if (!loc.isValid()) return true;



        FunctionDecl *func = call->getDirectCallee();
        if (!func) return true;

        QString name = QString::fromStdString(func->getNameAsString());
        // Проверяем, было ли имя уже обфусцировано
        if (obfuscationMap.find(name.toStdString()) != obfuscationMap.end()) {
            return true; // Пропускаем, если имя уже обфусцировано
        }
        qDebug() << "VisitCallExpr: function=" << name;

        if (obfuscationMap.find(name.toStdString()) != obfuscationMap.end()) {
            QString obfuscatedName = obfuscationMap[name.toStdString()];
            qDebug() << "Replacing function call: " << name << " -> " << obfuscatedName;
            codeRewriter.ReplaceText(loc, obfuscatedName.toStdString());
        }

        // Проверяем, вызывается ли метод через объект (a.exec();)
        if (MemberExpr *member = dyn_cast<MemberExpr>(call->getCallee())) {
            Expr *baseExpr = member->getBase()->IgnoreImpCasts();
            QString baseName;

            if (auto *declRef = dyn_cast<DeclRefExpr>(baseExpr)) {
                baseName = QString::fromStdString(declRef->getNameInfo().getAsString());
            } else {
                baseName = "UNKNOWN";
            }

            QString methodName = QString::fromStdString(member->getMemberNameInfo().getAsString());
            qDebug() << "VisitCallExpr (object method): base=" << baseName << ", method=" << methodName;

            if (obfuscationMap.find(baseName.toStdString()) != obfuscationMap.end()) {
                QString obfuscatedBase = obfuscationMap[baseName.toStdString()];
                qDebug() << "Replacing object name: " << baseName << " -> " << obfuscatedBase;
                SourceLocation baseLoc = SM.getSpellingLoc(member->getBeginLoc());
                codeRewriter.ReplaceText(baseLoc, obfuscatedBase.toStdString());
            }
        }

        return true;
    }
    bool VisitDeclRefExpr(DeclRefExpr *ref) {
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = SM.getSpellingLoc(ref->getLocation());

        if (!loc.isValid()) {
            qWarning() << "Invalid location!";
            return true;
        }
        if (!globalCheckedItems.contains("Name variable")) {
            return true;
        }
        // Проверяем, является ли родительский узел CallExpr
        // if (auto *parent = dyn_cast_or_null<CallExpr>(ref->getParent())) {
        //     // Если это вызов функции, пропускаем обработку
        //     return true;
        // }
        // Используем ParentMapContext для получения родительских узлов
        auto &parentMap = Context->getParentMapContext();
        auto parents = parentMap.getParents(*ref);

        // Проверяем, является ли родительский узел CallExpr
        for (const auto &parent : parents) {
            if (parent.get<CallExpr>()) {
                // Если это вызов функции, пропускаем обработку
                return true;
            }
        }

        QString name = QString::fromStdString(ref->getNameInfo().getAsString());

        // Проверяем, есть ли обфусцированное имя
        if (obfuscationMap.find(name.toStdString()) != obfuscationMap.end()) {
            QString obfuscatedName = obfuscationMap[name.toStdString()];
            qDebug() << "Replacing usage:" << name << "->" << obfuscatedName;
            codeRewriter.ReplaceText(loc, obfuscatedName.toStdString());
        }

        return true;
    }

private:
    ASTContext *Context;
    std::unordered_map<std::string, QString>& obfuscationMap;
    clang::Rewriter &codeRewriter;
    int counter;
    QString getObfuscatedName(const QString &original) {
        std::string key = original.toStdString();
        if (obfuscationMap.find(key) == obfuscationMap.end()) {
            obfuscationMap[key] = "X" + QString::number(counter++);
        }
        return obfuscationMap[key];
    }
    void ReplaceName(const NamedDecl *decl, const QString &newName) {
        SourceManager &SM = Context->getSourceManager();
        SourceLocation loc = decl->getLocation();
        loc = SM.getSpellingLoc(loc);  // Получаем реальное положение имени в коде

        if (loc.isValid()) {
            codeRewriter.ReplaceText(loc, newName.toStdString());
            qDebug() << "Replaced:" << QString::fromStdString(decl->getNameAsString()) << "->" << newName;
        }
    }
    void ReplaceTextAtLocation(SourceLocation loc, const QString &newName) {
        if (!loc.isValid()) return;
        SourceManager &SM = Context->getSourceManager();
        codeRewriter.ReplaceText(SM.getExpansionLoc(loc), newName.toStdString());
    }
};

// Класс для анализа AST
class MyASTConsumer : public ASTConsumer {
public:
    //explicit MyASTConsumer(ASTContext *Context, clang::Rewriter &codeRewriter)
    //: Visitor(Context, codeRewriter) {}
    explicit MyASTConsumer(ASTContext *Context, clang::Rewriter &codeRewriter, std::unordered_map<std::string, QString> &obfuscationMap)
        : Visitor(Context, codeRewriter, obfuscationMap), Context(Context), codeRewriter(codeRewriter) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    MyASTVisitor Visitor;
    ASTContext *Context;
    clang::Rewriter &codeRewriter;
};

// Класс для создания ASTConsumer
class MyFrontendAction : public ASTFrontendAction {
public:
    // Конструктор, принимающий карту обфускации
    explicit MyFrontendAction(std::unordered_map<std::string, QString> &obfuscationMap)
        : obfuscationMap(obfuscationMap) {}
    clang::Rewriter rewriter;  // Добавляем rewriter как член класса
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        return std::make_unique<MyASTConsumer>(&CI.getASTContext(), rewriter, obfuscationMap);
    }
private:
    std::unordered_map<std::string, QString> &obfuscationMap; // Карта обфускации
};
#endif // MYASTCLASS_H
