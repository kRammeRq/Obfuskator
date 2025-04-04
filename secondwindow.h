#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H
#include <QWidget>
#include "ui_secondwindow.h"  // Убедитесь, что этот заголовочный файл подключён

namespace Ui {
class SecondWindow;
}

class SecondWindow : public QWidget {
    Q_OBJECT

public:
    explicit SecondWindow(QWidget *parent = nullptr);
    ~SecondWindow();

private:
    Ui::SecondWindow *ui;
};
#endif // SECONDWINDOW_H
