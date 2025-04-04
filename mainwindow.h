#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingswindow.h" // Подключаем заголовок второго окна
extern QStringList globalCheckedItems;
extern QStringList globalListofPath;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setFilePath(const QString &path);
    void setFileSavePath(const QString &path);

private slots:
    void on_pushButton_clicked();
    void receiveCheckedItems(QStringList checkedItems); // Обработчик данных из окна настроек
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();
    void handleSettingsWindowClosed();  // Добавляем новый слот

private:
    Ui::MainWindow *ui;
    SettingsWindow *settingsWindow; // Указатель на второе окно
};
#endif // MAINWINDOW_H
