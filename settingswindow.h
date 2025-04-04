#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QFileDialog>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    //explicit SettingsWindow(QWidget *parent = nullptr);
    explicit SettingsWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Window);
    ~SettingsWindow();
    void setupListWidget(); // Добавьте декларацию метода

signals:
    void sendCheckedItems(QStringList checkedItems); // Сигнал для отправки данных в основное окно
private slots:
    void on_pushButton_clicked();

    void on_pushButtonAddString_clicked();

    void on_pushButtonDeleteString_clicked();

private:
    Ui::SettingsWindow *ui;
    void addStringToList(const QString &str);  // Метод для добавления строки в список
    void removeStringFromList();               // Метод для удаления выбранной строки
};

#endif // SETTINGSWINDOW_H
