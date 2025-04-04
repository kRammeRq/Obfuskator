#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "mainwindow.h"
void SettingsWindow::setupListWidget() {
    // Преобразуем элементы в чекбоксы
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // Делаем элемент чекбоксом
        //item->setCheckState(Qt::Checked); // Устанавливаем состояние чекбокса
        // Проверяем, есть ли данный элемент в globalCheckedItems
        if (globalCheckedItems.contains(item->text())) {
            item->setCheckState(Qt::Checked);  // Если элемент есть в списке, ставим Checked
        } else {
            item->setCheckState(Qt::Unchecked);  // Если элемента нет, ставим Unchecked
        }
    }
    // Добавляем строки из globalListofPath в listWidgetPathStrings (без чекбоксов)
    for (const QString &path : globalListofPath) {
        ui->listWidgetPathStrings->addItem(path);  // Добавляем как строку, без чекбоксов
    }
}
SettingsWindow::SettingsWindow(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowTitle("Настройки");
    setupListWidget();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_pushButton_clicked()
{
    QStringList checkedItems;

    // Проходимся по всем элементам списка
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedItems.append(item->text()); // Добавляем текст отмеченных чекбоксов
        }
    }

    emit sendCheckedItems(checkedItems); // Отправляем список в главное окно
    this->close(); // Закрываем окно настроек
}

// Метод для добавления строки в список
void SettingsWindow::addStringToList(const QString &str)
{
    ui->listWidgetPathStrings->addItem(str);  // Добавляем строку в список
}

// Метод для удаления выбранной строки из списка
void SettingsWindow::removeStringFromList()
{
    QListWidgetItem *item = ui->listWidgetPathStrings->currentItem();
    if (item) {
        delete item;  // Удаляем выбранную строку
    }
}

void SettingsWindow::on_pushButtonAddString_clicked()
{
    QString selectedDir = QFileDialog::getExistingDirectory(nullptr, "Выберите папку для обработки");
    if (!selectedDir.isEmpty()) {
        // Добавляем строку в глобальный список
        globalListofPath.append(selectedDir);
        // Добавляем строку в список
        addStringToList(selectedDir);
    }
}

void SettingsWindow::on_pushButtonDeleteString_clicked()
{
    QListWidgetItem *item = ui->listWidgetPathStrings->currentItem();
    if (item) {
        // Получаем строку, которую нужно удалить
        QString pathToRemove = item->text();
        // Удаляем строку из глобального списка
        globalListofPath.removeAll(pathToRemove);
        // Удаляем выбранную строку из списка
        delete item;
    }
}

