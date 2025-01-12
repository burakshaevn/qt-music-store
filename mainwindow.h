#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QCalendarWidget>
#include <QDir>
#include <QListWidget>
#include <QDockWidget>
#include <QWidget>

#include <QProgressDialog>
#include <QThread>

#include "database_handler.h"
#include "product_card.h"
#include "floating_widgets.h"
#include "cart.h"
#include "user.h"
#include "table.h"
#include "instruments.h"

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

    void UpdateUser(const UserInfo& user, QWidget* parent);

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_logout_clicked();

    void on_pushButton_clean_cart_clicked();

    void on_pushButton_submit_cart_clicked();

private:
    Ui::MainWindow *ui;

    // Управление базой данных
    std::shared_ptr<DatabaseHandler> db_manager_;

    // Управляет информацией о каждом инструменте
    std::shared_ptr<Instruments> instruments_;

    // Управляет карточками товаров
    std::shared_ptr<ProductCard> product_card_;

    // Управляет корзиной.
    // Корзина хранится в статической памяти. Это значит, что после повторного запуска приложения
    // текущая корзина пользователя будет очищена.
    // Сама хэш-таблица хранит пару ключ-значение, в виде <название товара (QString), карточка (QWidget*)>
    std::shared_ptr<Cart> cart_;

    std::unique_ptr<User> user_;
    std::unique_ptr<Table> table_;

    // Управляет плавающими виджетами: левым боковым меню каталога и правым плавающим меню
    std::unique_ptr<FloatingWidgets> floating_widgets_;

    // Инициализировать зависимости
    void BuildDependencies();

    // Обработка нажатий на кнопки "Ещё", "Профиль" и "Корзина" из плавающего меню
    void MoreClicked();
    void ProfileClicked();
    void CartClicked();

    // Функции обработки кнопок оплаты/очистки корзины
    void CleanCart();
    void ToPayCart();

    // Создать содержимое и настроить расположение для плавающего меню (главная, профиль, и др.)
    void SetupFloatingMenu();

    // Создать содержимое и настроить расположение для левого бокового меню
    void SetupSideMenu();

    // Получить список названий купленных товаров
    QList<QString> GetPurchasedInstruments(int user_id) const;

};

#endif // MAINWINDOW_H
