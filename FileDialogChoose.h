#ifndef FILEDIALOGCHOOSE_H
#define FILEDIALOGCHOOSE_H
#include "mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <fstream>
#include <vector>
#include <llvm/Support/raw_ostream.h>

// Объявления функций
void processFiles(const QString& selectedPath, const QString& outputDir);
void processFile(const QString& filePath, const QString& outputDir);
void openFileDialogAndProcess(MainWindow *mainWindow);
void openFolder(MainWindow *mainWindow);
void SetprocessFile(const QString& filePath, const QString& outputDir);
void processFilesInDirectory(const QString& dirPath, const QString& outputDir);
void copyNonCppFiles(const QString& dirPath, const QString& outputDir);

#endif // FILEDIALOGCHOOSE_H
