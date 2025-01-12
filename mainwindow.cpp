#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 560);

    // Автоматически подключаемся к базе данных и открываем её
    db_manager_ = std::make_shared<DatabaseHandler>();
    db_manager_->LoadDefault();

    // Переключаем пользователя на экран логина после запуска
    ui->stackedWidget->setCurrentWidget(ui->login);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateUser(const UserInfo& user, QWidget* parent){
    user_ = std::make_unique<User>(user, parent);
}

void MainWindow::on_pushButton_login_clicked() {
    auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();
        if (query.next()) {
            UserInfo user;
            user.id_ = query.value("id").toInt();
            user.password_ = query.value("password").toString();
            user.role_ = Role::Admin;
            if (user.password_ == ui->lineEdit_password->text()) {
                ui->lineEdit_login->clear();
                ui->lineEdit_password->clear();
                QMessageBox::information(this, "Авторизация", "Выполнена авторизация как администратор.");
                UpdateUser(user, this);

                table_ = std::make_unique<Table>(db_manager_, user_.get(), nullptr);
                table_->BuildAdminTables();

                connect(table_.get(), &Table::Logout, this, &MainWindow::on_pushButton_logout_clicked);

                ui->stackedWidget->addWidget(table_.get());
                ui->stackedWidget->setCurrentWidget(table_.get());
            }
            else{
                QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
            }
        }
        else {
            auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

            if (queryResult.canConvert<QSqlQuery>()) {
                QSqlQuery query = queryResult.value<QSqlQuery>();
                if (query.next()) {
                    UserInfo user;
                    user.id_ = query.value("id").toInt();
                    user.full_name_ = query.value("first_name").toString();
                    user.full_name_ += " " + query.value("last_name").toString();
                    user.email_ = query.value("email").toString();
                    user.password_ = query.value("password").toString();
                    user.role_ = Role::User;
                    user.instruments_ = GetPurchasedInstruments(user.id_);

                    if (user.password_ == ui->lineEdit_password->text()) {
                        ui->lineEdit_login->clear();
                        ui->lineEdit_password->clear();
                        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

                        BuildDependencies();
                        UpdateUser(user, this);

                        instruments_->PullInstruments();

                        SetupFloatingMenu();
                        SetupSideMenu();

                        ui->stackedWidget->setCurrentWidget(ui->main);
                    }
                    else{
                        QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
                    }
                }
                else {
                    QMessageBox::critical(this, "Ошибка", "Пользователя с таким логином не существует.");
                }
            }
            else {
                QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
            }
        }
    }
    else {
        QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
    }
}

void MainWindow::on_pushButton_logout_clicked() {
    if (!this->ui->stackedWidget) {
        return;
    }

    if (user_  && user_->GetRole() == Role::User) {
        // side_widget_.reset();
        // floating_menu_.reset();
        floating_widgets_.reset();

        instruments_.reset();
        cart_.reset();
        product_card_.reset();
        db_manager_.reset();


        if (ui->scrollArea) {
            QWidget* oldWidget = ui->scrollArea->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }
        if (ui->scrollArea_2) {
            QWidget* oldWidget = ui->scrollArea_2->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }
    }
    user_.reset();
    table_.reset();

    // Переключение на экран логина
    if (this->ui->stackedWidget && this->ui->login) {
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }
}

void MainWindow::SetupFloatingMenu() {
    floating_widgets_->BuildFloatingMenu(
        // Обработка кнопки "More"
        [this]() { this->MoreClicked(); },

        // Обработка кнопки "Search"
        [this]() {
            bool ok;
            QString term = QInputDialog::getText(
                this,
                "Поиск",
                "Укажите название инструмента:",
                QLineEdit::Normal,
                "",
                &ok
                );

            if (ok && !term.isEmpty()) {
                InstrumentInfo instrument_;
                instrument_.name_ = term;

                int relevant_results = product_card_->DrawRelevantInstruments(ui->scrollArea, instrument_.name_);
                if (relevant_results > 0) {
                    QMessageBox::information(this, "", "Найдено " + QString::number(relevant_results) + " результатов по запросу «" + term + "».");
                    MoreClicked();
                } else {
                    QMessageBox::warning(this, "Результаты поиска", "Отсутствуют релевантные результаты.");
                }
            } else {
                QMessageBox::warning(
                    this,
                    "Предупреждение",
                    "Название инструмента не может быть пустым."
                    );
            }
        },

        // Обработка кнопки "Cart"
        [this]() { this->CartClicked(); },

        // Обработка кнопки "User Profile"
        [this]() { this->ProfileClicked(); }
        );

}

void MainWindow::SetupSideMenu() {
    floating_widgets_->BuildSideMenu(
        // Лямбда для обработки кликов по элементам списка
        [this](const QString& itemText) {
            product_card_->UpdateInstrumentsWidget(ui->scrollArea, itemText);
            ui->stackedWidget->setCurrentWidget(ui->main);
        },
        // Лямбда для обработки клика по "Смотреть всё"
        [this]() {
            product_card_->UpdateInstrumentsWidget(ui->scrollArea, "Смотреть всё");
            ui->stackedWidget->setCurrentWidget(ui->main);
        }
        );
}

QList<QString> MainWindow::GetPurchasedInstruments(int user_id) const {
    QList<QString> instruments_; // Список названий купленных инструментов

    QSqlQuery query;
    QString query_str = QString("select * from instruments where id in (select instrument_id from purchases where client_id = %1);").arg(user_id);
    if (!query.exec(query_str)) {
        qWarning() << "Failed to execute query:" << query.lastError().text();
        return instruments_;
    }

    while (query.next()) {
        instruments_.append(query.value(1).toString());
    }

    return instruments_;
}

void MainWindow::BuildDependencies() {
    if (!db_manager_) {
        db_manager_ = std::make_shared<DatabaseHandler>();
        db_manager_->LoadDefault();
    }

    if (floating_widgets_){
        SetupFloatingMenu();
        SetupSideMenu();
    }
    else {
        floating_widgets_ = std::make_unique<FloatingWidgets>(db_manager_, this);
        SetupFloatingMenu();
        SetupSideMenu();
    }

    if (!product_card_ && !cart_ && !instruments_) {
        product_card_ = std::make_shared<ProductCard>(db_manager_, nullptr, this);
        cart_ = std::make_shared<Cart>(product_card_, this);
        instruments_ = std::make_shared<Instruments>(product_card_, cart_, db_manager_);
        connect(instruments_.get(), &Instruments::CartUpdated, this, [this]{
            ui->label_cart_total->setText("Корзина — " + FormatPrice(cart_->GetTotalCost()) + " руб.");
        });

        // Устанавливаем связь между ProductCard и Instruments
        product_card_->SetInstrumentsPtr(instruments_);
    }
}

void MainWindow::MoreClicked()
{
    if (user_.get()){
        if (!product_card_->hidden_to_cart_buttons_IsEmpty()){
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            floating_widgets_->GetSideMenu()->setVisible(true);
            floating_widgets_->GetSideList()->setVisible(true);

            floating_widgets_->GetFloatingMenu()->move(970, 105);
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::ProfileClicked() {
    if (user_->GetRole() == Role::User) {
        floating_widgets_->GetSideMenu()->setVisible(false);
        floating_widgets_->GetSideList()->setVisible(false);

        product_card_->HideOldCards();
        product_card_->EnsureContainerInScrollArea(ui->scrollArea_2);

        const auto& purchases_instruments_ = GetPurchasedInstruments(user_->GetId());
        for (auto& name_ : purchases_instruments_) {
            product_card_->DrawItem(*instruments_->FindInstrument(name_));
            auto to_cart_button = product_card_->FindInstrumentCard(name_)->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
                to_cart_button->hide();
                product_card_->hidden_to_cart_buttons_Push(name_);
            }
        }
        product_card_->card_container_PerformAdjustSize();

        ui->label_clientname->setText(user_->GetName() + " — профиль");
        floating_widgets_->GetFloatingMenu()->move(970, 125);
        ui->stackedWidget->setCurrentWidget(ui->user_page);
    }
}

void MainWindow::CartClicked(){
    if (user_.get()){
        if (!product_card_->hidden_to_cart_buttons_IsEmpty()){
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            floating_widgets_->GetSideMenu()->setVisible(false);
            floating_widgets_->GetSideList()->setVisible(false);

            if (cart_->CartIsEmpty()){
                ui->label_cart_total->setText("Корзина");
                ui->label_cart_total_2->setText("Корзина пуста.");
            }
            else {
                ui->label_cart_total->setText("Корзина — " + FormatPrice(cart_->GetTotalCost()) + " руб.");
                ui->label_cart_total_2->setText(QString::number(cart_->GetCartSize()) + " шт.");

                product_card_->HideOldCards();
                product_card_->EnsureContainerInScrollArea(ui->scrollArea_3);

                for (const auto& [name_, card_] : cart_->GetCart()) {
                    const auto& instrument = instruments_->FindInstrument(name_);
                    product_card_->DrawItem(*instrument);
                }
                product_card_->card_container_PerformAdjustSize();
            }

            floating_widgets_->GetFloatingMenu()->move(970, 105);
            ui->stackedWidget->setCurrentWidget(ui->cart);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::CleanCart() {
    for (auto& [name_, card_] : cart_->GetCart()) {
        if (card_) {
            auto to_cart_button = card_->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
                to_cart_button->setIcon(QIcon(":/bookmark.svg"));
            }
        }
    }
    product_card_->HideOldCards();
    ui->label_cart_total->setText("Корзина");
    ui->label_cart_total_2->setText("Корзина пуста.");
    cart_->ClearCart();
}

void MainWindow::ToPayCart()
{
    // Начало транзакции
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию: " + db.lastError().text());
        return;
    }

    int new_id = db_manager_->GetMaxOrMinValueFromTable("MAX", "id", "purchases") + 1;
    bool success = true; // Флаг для отслеживания статуса операций

    for (auto& [name_, card_] : cart_->GetCart()) {
        auto instrument = instruments_->FindInstrument(name_);
        if (!db_manager_->ExecuteQuery(QString("INSERT INTO public.purchases(id, client_id, instrument_id) VALUES (%1, %2, %3);")
                                          .arg(new_id).arg(user_->GetId()).arg(instrument->id_)))
        {
            success = false;
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить операцию: " + db_manager_->GetLastError());
            break; // Прерываем цикл при ошибке
        }
        ++new_id;
    }

    // Если все операции успешны, фиксируем транзакцию
    if (success) {
        if (!db.commit()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать транзакцию: " + db.lastError().text());
        }
        else {
            // All succsess
        }
    }
    else {
        // Если есть ошибки, откатываем транзакцию
        if (!db.rollback()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось откатить транзакцию: " + db.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_clean_cart_clicked()
{
    if (cart_->CartIsEmpty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно очистить корзину: отсутствует содержимое.");
        return;
    }
    CleanCart();
    QMessageBox::information(this, "", "Корзина очищена.");
}

void MainWindow::on_pushButton_submit_cart_clicked()
{
    if (cart_->CartIsEmpty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно выполить оплату: отсутствует содержимое.");
        return;
    }
    ToPayCart();
    CleanCart();
    QMessageBox::information(this, "", "Произведена оплата. Купленные инструменты добавлены в профиль.");
}
