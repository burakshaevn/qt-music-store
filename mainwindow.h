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

#include "database_manager.h"
#include "user.h"
#include "table.h"
#include "instruments.h"
#include <unordered_set>

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

    // QMap<QString, QWidget*> instruments_cards_;
    std::unordered_map<QString, QWidget*> instruments_cards_;
    std::unique_ptr<QWidget> card_container_;
    std::unique_ptr<QVBoxLayout> layout_;

    DatabaseManager db_manager_;
    std::unique_ptr<Instruments> instruments_;
    std::unique_ptr<User> user_;
    std::unique_ptr<Table> table_;
    std::unique_ptr<QWidget> floating_menu_; // Плавающее меню

    // Боковое меню
    std::unique_ptr<QWidget> side_widget_;
    QListWidget* side_list_;

    // Корзина. Все инструменты, занесённые в корзину, можно впоследствие оплатить.
    // Корзина хранится в автоматической памяти.
    // Это значит, что после повторного запуска приложения текущая корзина пользователя будет очищена.
    // Сама хэш-таблица хранит «название товара, карточка» (указатель на класс QWidget)
    std::unordered_map<QString, QWidget*> cart_;
    int total_cost_ = 0;

    std::unordered_set<QString> hidden_to_cart_buttons_;
    void RestoreHiddenToCartButtons();

    void UpdateInstrumentsWidget(const QString& filter);
    void CreateInstrumentCart();

    // Обработка нажатий на кнопки "Ещё", "Профиль" и "Корзина" из плавающего меню
    void MoreClicked();
    void ProfileClicked();
    void CartClicked();

    // Функция очистки корзины
    void CleanCart();
    void ToPayCart();

    // Создать содержимое и настроить расположение для плавающего меню (главная, профиль, и др.)
    void SetupFloatingMenu();

    // Создать содержимое и настроить расположение для левого бокового меню
    void SetupSideMenu();

    void DrawItem(const InstrumentInfo& instrument);
    void DrawRelevantInstruments(const QString& term);

    void EnsureContainerInScrollArea(QScrollArea* targetScrollArea);

    // Спрятать старые карточки товаров.
    // Сначала все старые прячем, чтобы потом под определённые фильтры отобразить только нужные.
    // Они не удаляются, а просто скрываются, благодаря методу ->hide(),
    // потому что быстрее вызвать метод ->show() для необходимых карточек, чем собрать и отрисовать их по новой
    void HideOldCards();

    // Отформатировать число в формате 100000000 → 100 000 000
    QString FormatPrice(int price);

    // Кэширование инструментов из базы данных.
    // Загружаем в instruments_ как в основное хранилище
    void PullInstruments();

    // Получить список названий купленных товаров
    QList<QString> GetPurchasedInstruments(int user_id) const;

};

#endif // MAINWINDOW_H
