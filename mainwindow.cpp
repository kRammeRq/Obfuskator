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
#include "FileDialogChoose.h"
#include "MyASTclass.h"
//clang++ main_preprocessedGadza.cpp -o main_preprocessed2 -I/usr/include/x86_64-linux-gnu/qt5 -lQt5Core
QStringList globalCheckedItems;
QStringList globalListofPath;
using namespace clang;
using namespace clang::tooling;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), settingsWindow(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::receiveCheckedItems(QStringList checkedItems) {
    globalCheckedItems = checkedItems;
    qDebug() << "Выбранные элементы из SettingsWindow:" << globalCheckedItems;
}

void MainWindow::setFilePath(const QString &path)
{
    ui->lineEditFilePath->setText(path);
}
void MainWindow::setFileSavePath(const QString &path)
{
    ui->lineEditSavePath->setText(path);
}
void MainWindow::on_pushButton_clicked()
{
    // Вызов функции для открытия диалога выбора файла или папки и обработки
    openFileDialogAndProcess(this);
}


void MainWindow::on_pushButton_2_clicked()
{
    openFolder(this);
}

void MainWindow::on_pushButton_3_clicked()
{
    if (!settingsWindow) {
        settingsWindow = new SettingsWindow(this);
        settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
        // Подключаем сигнал после создания окна!
        connect(settingsWindow, &SettingsWindow::sendCheckedItems,
                this, &MainWindow::receiveCheckedItems);
        // Подключаем сигнал закрытия окна к нашему слоту
        connect(settingsWindow, &SettingsWindow::destroyed,
                this, &MainWindow::handleSettingsWindowClosed);
    }

    settingsWindow->show();
}
void MainWindow::handleSettingsWindowClosed()
{
    settingsWindow = nullptr;  // Важно: обнуляем указатель
}

