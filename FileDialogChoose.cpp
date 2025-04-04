#include "FileDialogChoose.h"
#include <QCoreApplication>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Basic/Diagnostic.h>
#include <QRegularExpression>
#include <llvm/Support/raw_ostream.h>
#include "MyASTclass.h"
using namespace clang;  // Подключаем пространство имён clang
// Глобальная карта обфускации
std::unordered_map<std::string, QString> globalObfuscationMap;

QStringList readLinesFromFile(const QString& filePath) {
    QStringList lines;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return lines;  // Возвращаем пустой список в случае ошибки
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        lines.append(line);  // Добавляем строку в QStringList
    }

    file.close();
    return lines;
}

QString removeComments(const QString &code) {
    QString result = code;
    QStringList lines = code.split("\n");

    bool inMultiLineComment = false; // Флаг нахождения в многострочном комментарии
    int multiLineStart = -1; // Переменная для запоминания начала многострочного комментария

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];

        // Ищем однострочные комментарии
        QRegularExpression singleLineRegex("//.*");
        QRegularExpressionMatch singleMatch = singleLineRegex.match(line);
        if (singleMatch.hasMatch()) {
            qDebug() << "Удалён однострочный комментарий с линии" << (i + 1)
                     << ": " << singleMatch.captured(0);
            line.remove(singleMatch.captured(0)); // Удаляем однострочный комментарий
        }

        // Ищем начало многострочного комментария
        int multiStart = line.indexOf("/*");
        if (multiStart != -1 && !inMultiLineComment) {
            inMultiLineComment = true;
            multiLineStart = i + 1; // Запоминаем номер строки, где начинается многострочный комментарий
            qDebug() << "Удалён многострочный комментарий с линии" << (i + 1)
                     << ": " << line.mid(multiStart);
            line.remove(multiStart, line.length() - multiStart); // Удаляем всё после "/*"
        }

        // Ищем конец многострочного комментария
        int multiEnd = line.indexOf("*/");
        if (multiEnd != -1 && inMultiLineComment) {
            qDebug() << "Удалён многострочный комментарий с линии" << multiLineStart
                     << "по" << (i + 1)
                     << ": " << line.mid(0, multiEnd + 2);
            inMultiLineComment = false; // Завершаем многострочный комментарий
            line.remove(0, multiEnd + 2); // Удаляем до "*/"
        }

        // Если мы внутри многострочного комментария, продолжаем его удаление
        if (inMultiLineComment) {
            qDebug() << "Удалён многострочный комментарий с линии" << (i + 1)
                     << ": " << line;
            line.clear(); // Очищаем строку, так как она внутри многострочного комментария
        }

        // Обновляем строку в списке
        lines[i] = line.trimmed(); // Удаляем лишние пробелы и пустые строки
    }

    // Удаляем пустые строки
    lines.removeAll("");

    // Собираем строки обратно в один текст
    result = lines.join("\n");
    return result;
}


void processFile(const QString& filePath, const QString& outputDir) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, QCoreApplication::translate("context", "Ошибка"), QCoreApplication::translate("context", "Не удалось открыть файл"));
        return;
    }

    QTextStream in(&file);
    QString code = in.readAll();
    if (globalCheckedItems.contains("Delete comments")) {
        code = removeComments(code);
    }

    qDebug() << "Обрабатываем файл:" << filePath;

    QFileInfo fileInfo(filePath);
    // Генерируем имя выходного файла, используя имя исходного файла и папку для сохранения
    QString outputFile = outputDir + "/" + fileInfo.fileName();
    QString filePathfromfile = "/home/krammer/untitled/build/Gadza-Debug/PathToArgs";
    QStringList linesofpath = readLinesFromFile(filePathfromfile);
    clang::Rewriter rewriter;
    std::vector<std::string> extraArgs;
    extraArgs.push_back("-std=c++17");
    // for (QString &line : linesofpath) {
    //     //line = "-I" + line;
    //     //extraArgs.push_back(line);
    //     extraArgs.push_back(("-I" + line).toStdString());
    // }
    extraArgs.push_back("-I/usr/include/x86_64-linux-gnu/qt5");
    extraArgs.push_back("-I/home/krammer/test");
    extraArgs.push_back("-fsyntax-only");

    std::unique_ptr<ASTUnit> AST = tooling::buildASTFromCodeWithArgs(code.toStdString(), extraArgs);

    if (!AST) {
        qWarning() << "Не удалось построить AST";
        return;
    }

    rewriter.setSourceMgr(AST->getASTContext().getSourceManager(), AST->getASTContext().getLangOpts());
    MyASTConsumer consumer(&AST->getASTContext(), rewriter, globalObfuscationMap);
    consumer.HandleTranslationUnit(AST->getASTContext());

    std::string modifiedCode;
    llvm::raw_string_ostream modifiedStream(modifiedCode);
    rewriter.getEditBuffer(AST->getASTContext().getSourceManager().getMainFileID()).write(modifiedStream);
    modifiedStream.flush();

    // Открываем файл для записи в указанную папку
    std::ofstream outFile(outputFile.toStdString());
    if (outFile.is_open()) {
        outFile << modifiedCode;
        outFile.close();
        std::cout << "Файл сохранён как " << outputFile.toStdString() << std::endl;
    } else {
        std::cerr << "Ошибка открытия файла для записи: " << outputFile.toStdString() << std::endl;
    }
}

void openFileDialogAndProcess(MainWindow *mainWindow) {
    // Даем пользователю выбрать файл для обработки
    QString selectedFile = QFileDialog::getOpenFileName(nullptr, QCoreApplication::translate("context", "Выберите файл для обработки"));

    if (selectedFile.isEmpty()) {
        return;
    }

    // Выбор папки для сохранения обработанного файла
    QString outputDir = QFileDialog::getExistingDirectory(nullptr, QCoreApplication::translate("context", "Выберите папку для сохранения файла"));

    if (outputDir.isEmpty()) {
        return;
    }
    mainWindow->setFilePath(selectedFile);
    mainWindow->setFileSavePath(outputDir);
    // Обрабатываем выбранный файл и сохраняем его в выбранную папку
    processFile(selectedFile, outputDir);
}
////////////////////////////////////////////////////////////////////////
void SetprocessFile(const QString& filePath, const QString& outputDir) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream in(&file);
    QString code = in.readAll();
    if (globalCheckedItems.contains("Delete comments")) {
        code = removeComments(code);
    }
    qDebug() << "Обрабатываем файл:" << filePath;

    QFileInfo fileInfo(filePath);
    QString outputFile = outputDir + "/" + fileInfo.fileName();

    clang::Rewriter rewriter;
    std::vector<std::string> extraArgs;
    // extraArgs.push_back("-std=c++17");
    // //extraArgs.push_back("-I/usr/include/x86_64-linux-gnu/qt5");
    // extraArgs.push_back("-I/usr/include/x86_64-linux-gnu/qt5/QtCore");
    // extraArgs.push_back("-I/home/krammer/test");
    // extraArgs.push_back("-fsyntax-only");
    QString filePathfromfile = "/home/krammer/untitled/build/Gadza-Debug/PathToArgs";
    QStringList linesofpath = readLinesFromFile(filePathfromfile);
    extraArgs.push_back("-std=c++17");
    for (QString &line : linesofpath) {
        //line = "-I" + line;
        //extraArgs.push_back(line);
        extraArgs.push_back(("-I" + line).toStdString());
    }
    //extraArgs.push_back("-I/usr/include/x86_64-linux-gnu/qt5");
    //extraArgs.push_back("-I/home/krammer/test");
    extraArgs.push_back("-fsyntax-only");

    std::unique_ptr<ASTUnit> AST = tooling::buildASTFromCodeWithArgs(code.toStdString(), extraArgs);

    if (!AST) {
        qWarning() << "Не удалось построить AST";
        return;
    }

    rewriter.setSourceMgr(AST->getASTContext().getSourceManager(), AST->getASTContext().getLangOpts());
    MyASTConsumer consumer(&AST->getASTContext(), rewriter, globalObfuscationMap);
    consumer.HandleTranslationUnit(AST->getASTContext());

    // Здесь добавьте логику обработки AST
    // Например: MyASTConsumer consumer(&AST->getASTContext(), rewriter);
    // consumer.HandleTranslationUnit(AST->getASTContext());

    std::string modifiedCode;
    llvm::raw_string_ostream modifiedStream(modifiedCode);
    rewriter.getEditBuffer(AST->getASTContext().getSourceManager().getMainFileID()).write(modifiedStream);
    modifiedStream.flush();

    std::ofstream outFile(outputFile.toStdString());
    if (outFile.is_open()) {
        outFile << modifiedCode;
        outFile.close();
        std::cout << "Файл сохранён как " << outputFile.toStdString() << std::endl;
    } else {
        std::cerr << "Ошибка открытия файла для записи: " << outputFile.toStdString() << std::endl;
    }
}

void processFilesInDirectory(const QString& dirPath, const QString& outputDir) {
    QDir dir(dirPath);
    QStringList filters;
    //filters << "*.cpp";  // Ищем только .cpp файлы
    filters << "*.cpp" << "*.h";  // Ищем .cpp и .h файлы
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    // Создаем директорию, если она не существует
    if (!QDir(outputDir).exists()) {
        QDir().mkdir(outputDir);
    }

    // Обрабатываем все найденные файлы .cpp
    for (const QFileInfo& file : files) {
        QString filePath = file.absoluteFilePath();
        SetprocessFile(filePath, outputDir);
    }

    // Рекурсивно обрабатываем все подпапки
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& subdir : subdirs) {
        QString subDirPath = subdir.absoluteFilePath();
        QString outputSubDir = outputDir + "/" + subdir.fileName();
        processFilesInDirectory(subDirPath, outputSubDir);  // Рекурсивно обрабатываем подпапки
    }

    // Копируем все файлы, которые не являются .cpp в новую папку
    copyNonCppFiles(dirPath, outputDir);
}

void copyNonCppFiles(const QString& dirPath, const QString& outputDir) {
    QDir dir(dirPath);

    // Копируем все файлы, которые не являются .cpp
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
        if (!file.suffix().compare("cpp", Qt::CaseInsensitive)) {
            continue; // Пропускаем .cpp файлы
        }

        QString srcFilePath = file.absoluteFilePath();
        QString dstFilePath = outputDir + "/" + file.fileName();

        if (!QFile::copy(srcFilePath, dstFilePath)) {
            std::cerr << "Ошибка копирования файла: " << srcFilePath.toStdString() << std::endl;
        } else {
            std::cout << "Файл скопирован: " << dstFilePath.toStdString() << std::endl;
        }
    }

    // Копируем подпапки
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& subdir : subdirs) {
        QString subDirPath = subdir.absoluteFilePath();
        QString outputSubDir = outputDir + "/" + subdir.fileName();
        QDir().mkpath(outputSubDir);  // Создаем структуру папок в целевой директории

        // Рекурсивно копируем содержимое подпапки
        copyNonCppFiles(subDirPath, outputSubDir);
    }
}


void openFolder(MainWindow *mainWindow) {
    // Даем пользователю выбрать папку с .cpp файлами
    QString selectedDir = QFileDialog::getExistingDirectory(nullptr, "Выберите папку для обработки");

    if (selectedDir.isEmpty()) {
        return;  // Если папка не выбрана, выходим
    }

    // Папка для сохранения обработанных файлов
    QString outputDir = QFileDialog::getExistingDirectory(nullptr, "Выберите папку для сохранения файлов");

    if (outputDir.isEmpty()) {
        return;  // Если папка для сохранения не выбрана, выходим
    }
    //mainWindow->ui->lineEditFilePath->setText(outputDir);
    // Обновляем text в главном окне через публичный метод
    mainWindow->setFilePath(selectedDir);
    mainWindow->setFileSavePath(outputDir);
    processFilesInDirectory(selectedDir, outputDir);  // Запускаем обработку
}
