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
        side_widget_.reset();
        floating_menu_.reset(); 

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
    floating_menu_ = std::make_unique<QWidget>(this);
    floating_menu_->setStyleSheet("background-color: #fafafa; border-radius: 29px;");
    floating_menu_->setFixedSize(74, 349);

    QVBoxLayout* menuLayout = new QVBoxLayout(floating_menu_.get());
    menuLayout->setContentsMargins(10, 20, 10, 20);
    menuLayout->setSpacing(20);

    QPushButton* more_ = new QPushButton(floating_menu_.get());
    more_->setIcon(QIcon(":/more_horiz.svg"));
    more_->setIconSize(QSize(35, 35));
    more_->setStyleSheet("border: none; outline: none;");
    connect(more_, &QPushButton::clicked, this, &MainWindow::MoreClicked);

    QPushButton* searchButton = new QPushButton(floating_menu_.get());
    searchButton->setIcon(QIcon(":/search.svg"));
    searchButton->setIconSize(QSize(35, 35));
    searchButton->setStyleSheet("border: none; outline: none;");
    connect(searchButton, &QPushButton::clicked, this, [this]() {
        bool ok;

        // Создаём QInputDialog
        QInputDialog inputDialog(this);
        inputDialog.setWindowTitle("Поиск");
        inputDialog.setLabelText("Укажите название инструмента:");
        inputDialog.setInputMode(QInputDialog::TextInput); // Ввод текста
        inputDialog.setTextValue(""); // Стартовое значение

        // Устанавливаем минимальные и максимальные размеры
        inputDialog.setMinimumSize(300, 150); // Минимальные размеры
        inputDialog.setMaximumSize(600, 400); // Разумные ограничения максимума

        // Центрируем диалог
        inputDialog.setGeometry(QStyle::alignedRect(
            Qt::LeftToRight,                      // Направление текста/интерфейса
            Qt::AlignCenter,                      // Центрирование
            inputDialog.size(),                   // Размер диалога
            qApp->primaryScreen()->availableGeometry() // Доступная геометрия экрана
            ));

        // Если пользователь нажал OK
        if (inputDialog.exec() == QDialog::Accepted) {
            QString term = inputDialog.textValue();

            // Проверяем успешность ввода
            if (!term.isEmpty()) {
                InstrumentInfo instrument_;
                instrument_.name_ = term;

                // Выполняем поиск инструмента
                int relevant_results = product_card_->DrawRelevantInstruments(ui->scrollArea, instrument_.name_);
                if (relevant_results > 0){
                    QMessageBox::information(this, "", "Найдено " + QString::number(relevant_results) + " результатов по запросу «" + term + "».");
                    MoreClicked();
                }
                else{
                    QMessageBox::warning(this, "Результаты поиска", "Отсутствуют релевантные результаты.");
                }
            }
            else {
                QMessageBox::warning(
                    this,
                    "Предупреждение",
                    "Название инструмента не может быть пустым."
                );
            }
        }
    });

    QPushButton* cart = new QPushButton(floating_menu_.get());
    cart->setIcon(QIcon(":/Shopping cart.svg"));
    cart->setIconSize(QSize(32, 32));
    cart->setStyleSheet("border: none; outline: none;");
    connect(cart, &QPushButton::clicked, this, &MainWindow::CartClicked);

    QPushButton* userButton = new QPushButton(floating_menu_.get());
    userButton->setIcon(QIcon(":/person.svg"));
    userButton->setIconSize(QSize(35, 35));
    userButton->setStyleSheet("border: none; outline: none;");
    connect(userButton, &QPushButton::clicked, this, &MainWindow::ProfileClicked);

    menuLayout->addWidget(more_);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(cart);
    menuLayout->addWidget(userButton);

    floating_menu_->move(970, 105);
    floating_menu_->show();
}

void MainWindow::SetupSideMenu() {
    // Создаём боковое меню как QWidget
    side_widget_ = std::make_unique<QWidget>(this);
    side_widget_->setStyleSheet("background-color: #fafafa;");

    side_widget_->setGeometry(0, 0, 224, 560);

    // QLabel* logo = new QLabel(side_widget_.get());
    // logo->setPixmap(QPixmap("://logo.svg").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // logo->setGeometry(19, 38, 18, 18);

    // QLabel* logo_words = new QLabel(side_widget_.get());
    // logo_words->setPixmap(QPixmap("://mercedez-benz.svg").scaled(160, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // logo_words->setGeometry(45, 38, 160, 18);

    // заголовок
    QLabel* title = new QLabel("Каталог", side_widget_.get());
    title->setStyleSheet("background-color: #fafafa; color: #140f10; font: 700 14pt 'Open Sans';");
    title->setGeometry(28, 87, 172, 19);

    // Создаём QListWidget
    side_list_ = new QListWidget(side_widget_.get());
    side_list_->setStyleSheet("background-color: #fafafa; color: #140f10; font: 12pt 'Open Sans'; border: 0px;");
    side_list_->setGeometry(22, 111, 224, 449);
    side_list_->setSpacing(3);

    // Наполняем список моделей
    QSqlQuery query("SELECT name FROM instrument_types");
    while (query.next()) {
        QString carTypeName = query.value("name").toString();
        side_list_->addItem(carTypeName);
    }

    // Добавляем кнопку "Смотреть все" как элемент списка
    QListWidgetItem* viewAllItem = new QListWidgetItem("Смотреть всё", side_list_);
    viewAllItem->setTextAlignment(Qt::AlignLeft); // Выравнивание текста по центру
    viewAllItem->setFont(QFont("Open Sans", 12));
    viewAllItem->setForeground(QColor("#9b9c9c"));

    // Обработка кликов по элементам меню
    connect(side_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item->text() == "Смотреть всё") {
            product_card_->UpdateInstrumentsWidget(ui->scrollArea, "Смотреть всё");
        }
        else {
            // Логика для остальных пунктов списка
            product_card_->UpdateInstrumentsWidget(ui->scrollArea, item->text());
        }
        ui->stackedWidget->setCurrentWidget(ui->main);
    });

    side_widget_->show();
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
    if (!product_card_ && !cart_ && !instruments_) {
        product_card_ = std::make_shared<ProductCard>(db_manager_, nullptr, this);
        cart_ = std::make_shared<Cart>(product_card_, this);
        instruments_ = std::make_shared<Instruments>(product_card_, cart_, db_manager_);

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
            side_widget_->setVisible(true);
            side_list_->setVisible(true);

            floating_menu_->move(970, 105);
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::ProfileClicked() {
    if (user_->GetRole() == Role::User) {
        side_widget_->setVisible(false);
        side_list_->setVisible(false);

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
        floating_menu_->move(970, 125);
        ui->stackedWidget->setCurrentWidget(ui->user_page);
    }
}

void MainWindow::CartClicked(){
    if (user_.get()){
        if (!product_card_->hidden_to_cart_buttons_IsEmpty()){
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            side_widget_->setVisible(false);
            side_list_->setVisible(false);

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

            floating_menu_->move(970, 105);
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
            cart_->ClearCart(); // Очищаем корзину после успешной транзакции
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
