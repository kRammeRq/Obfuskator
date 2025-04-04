#include "MyASTclass.h"

bool isUserCode(clang::SourceLocation Loc, clang::ASTContext *Context) {
    clang::SourceManager &SM = Context->getSourceManager();
    std::string fileName = SM.getFilename(Loc).str();
    std::string projectDir = "/home/krammer/test/"; // Укажите свою директорию проекта

    return fileName.rfind(projectDir, 0) == 0;
}

// Функция для преобразования числа в двоичное представление
QString toBinaryString(int value) {
    if (value == 0) return "0b0";
    std::string binary = std::bitset<32>(value).to_string();
    binary.erase(0, binary.find_first_not_of('0'));
    return "0b" + QString::fromStdString(binary);
}
