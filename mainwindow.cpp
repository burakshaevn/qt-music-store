#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 560);

    // Настраиваем автоматическое подключение к базе данных
    // при запуске приложения и открываем базу данных
    db_manager_.LoadDefault();

    // После запуска переносим пользователя на окно входа
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
    auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

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

                table_ = std::make_unique<Table>(&db_manager_, user_.get(), nullptr);
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
            auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

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
                        instruments_ = std::make_unique<Instruments>();
                        UpdateUser(user, this);
                        PullInstruments();
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

    if (user_->GetRole() == Role::User) {
        side_widget_.reset();
        floating_menu_.reset();
        layout_.reset();
        card_container_.reset();
        instruments_->Clear();
        instruments_cards_.clear();
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
    this->ui->stackedWidget->setCurrentWidget(this->ui->login);
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
            QString instrumentName = inputDialog.textValue(); // Имя инструмента

            // Проверяем успешность ввода
            if (!instrumentName.isEmpty()) {
                InstrumentInfo instrument_;
                instrument_.name_ = instrumentName;

                // Выполняем поиск инструмента
                DrawRelevantInstruments(instrument_.name_);
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
            UpdateInstrumentsWidget("Смотреть всё");
        }
        else {
            // Логика для остальных пунктов списка
            UpdateInstrumentsWidget(item->text());
        }
        ui->stackedWidget->setCurrentWidget(ui->main);
    });

    side_widget_->show();
}

void MainWindow::RestoreHiddenToCartButtons() {
    for (const auto& name : hidden_to_cart_buttons_) {
        auto to_cart_button = instruments_cards_[name]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
        if (to_cart_button) {
            to_cart_button->show();
        }
    }
    hidden_to_cart_buttons_.clear();
}

void MainWindow::UpdateInstrumentsWidget(const QString& filter) {
    int id = -1;
    if (filter != "Смотреть всё") {
        auto result = db_manager_.ExecuteSelectQuery(QString("SELECT id FROM instrument_types WHERE name = '%1'").arg(filter));
        if (result.canConvert<QSqlQuery>()) {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next()) {
                id = query.value("id").toInt();
            }
        }
    }

    HideOldCards();
    EnsureContainerInScrollArea(ui->scrollArea);

    // for (auto it = instruments_cards_.begin(); it != instruments_cards_.end(); ++it) {
    //     const QString& name = it.key();
    //     QWidget* card = it.value();
    //     const auto& instrument = instruments_->FindInstrument(name);

    //     bool shouldDisplay = (filter == "Смотреть всё" || instrument->type_id_ == id);
    //     if (shouldDisplay) {
    //         DrawItem(*instrument);
    //     }
    // }
    for (auto& [name, card] : instruments_cards_) {
        // const QString& name = it.key();
        // QWidget* card = it.value();
        const auto& instrument = instruments_->FindInstrument(name);

        bool shouldDisplay = (filter == "Смотреть всё" || instrument->type_id_ == id);
        if (shouldDisplay) {
            DrawItem(*instrument);
        }
    }
    card_container_->adjustSize();
}

void MainWindow::CreateInstrumentCart() {
    if (!card_container_) {
        // Создаем card_container_
        card_container_ = std::make_unique<QWidget>();
        card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        // Создаём layout для карточек
        layout_ = std::make_unique<QVBoxLayout>(card_container_.get());
        layout_->setAlignment(Qt::AlignTop);
        layout_->setSpacing(15);
        layout_->setContentsMargins(10, 10, 10, 10);
    }
}

void MainWindow::DrawItem(const InstrumentInfo& instrument) {
    if (instruments_cards_.contains(instrument.name_)) {
        QWidget* card = instruments_cards_[instrument.name_];
        if (!layout_->indexOf(card)) {
            card->setParent(card_container_.get());
            layout_->addWidget(card);
        }
        card->show();
        card->setFixedHeight(130);
        card->setVisible(true);
    }
    else {
        qDebug() << "Карточка не найдена: " << instrument.name_;
    }
}

QString MainWindow::FormatPrice(int price) {
    QString formattedPrice = QString::number(price);
    int len = formattedPrice.length();
    for (int i = len - 3; i > 0; i -= 3) {
        formattedPrice.insert(i, ' ');
    }
    return formattedPrice;
};

void MainWindow::PullInstruments() {
    if (!card_container_) {
        CreateInstrumentCart();
    }

    // Выполняем запрос к базе данных
    auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT * FROM public.instruments ORDER BY id ASC"));
    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();

        // Загружаем инструменты в instruments_
        instruments_->Clear();
        while (query.next()) {
            InstrumentInfo instrument;
            instrument.id_ = query.value("id").toInt();
            instrument.name_ = query.value("name").toString();
            instrument.type_id_ = query.value("type_id").toInt();
            instrument.price_ = query.value("price").toDouble();
            instrument.description_ = query.value("description").toString();

            QString image_path = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" +
                                                 "/../../resources/" + query.value("image_path").toString().replace("\\", "/"));
            image_path += "/" + instrument.name_ + ".png";
            instrument.image_path_ = image_path;

            instruments_->PushInstrument(instrument);
        }

        // Создаем карточки для инструментов
        for (const auto& [name, instrument_info] : instruments_->GetInstruments()) {
            if (!instruments_cards_.contains(instrument_info.name_)) {
                auto* card = new QWidget(card_container_.get());
                instruments_cards_[instrument_info.name_] = card;

                card->setStyleSheet("background-color: #fafafa; border-radius: 50px;");
                card->setFixedSize(644, 130);

                switch (instrument_info.type_id_) {
                case 1: case 2: {
                    QPixmap originalPixmap(instrument_info.image_path_);
                    if (!originalPixmap.isNull()) {
                        QTransform transform;
                        transform.rotate(90);
                        QPixmap rotatedPixmap = originalPixmap.transformed(transform);
                        QPixmap scaledPixmap = rotatedPixmap.scaled(210, 68, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        QLabel* instrument_image = new QLabel(card);
                        instrument_image->setPixmap(scaledPixmap);
                        instrument_image->setFixedSize(210, 68);
                        instrument_image->move(27, (card->height() - instrument_image->height()) / 2);
                    }
                    break;
                }
                case 3: case 4: case 5: case 6: case 7: {
                    QPixmap originalPixmap(instrument_info.image_path_);
                    if (!originalPixmap.isNull()) {
                        QPixmap scaledPixmap = originalPixmap.scaled(145, 113, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        QLabel* instrument_image = new QLabel(card);
                        instrument_image->setPixmap(scaledPixmap);
                        instrument_image->setFixedSize(145, 113);
                        instrument_image->move(62, (card->height() - instrument_image->height()) / 2);
                    }
                    break;
                }

                default:
                    break;
                }

                // Название инструмента
                QLineEdit* instrument_name = new QLineEdit(instrument_info.name_, card);
                instrument_name->setStyleSheet("font: 700 16pt 'Open Sans'; color: #1d1b20;");
                instrument_name->setAlignment(Qt::AlignLeft);
                instrument_name->setCursorPosition(0);
                instrument_name->setFixedSize(315, 26);
                instrument_name->move(272, 19);
                instrument_name->setReadOnly(true);

                // Описание
                QLabel* instrument_description = new QLabel(instrument_info.description_, card);
                instrument_description->setStyleSheet("font: 12pt 'JetBrains Mono'; color: #555555;");
                instrument_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                instrument_description->setFixedSize(315, 19);
                instrument_description->move(273, 50);

                // Цена
                QLabel* instrument_price = new QLabel(FormatPrice(instrument_info.price_) + " руб.", card);
                instrument_price->setStyleSheet("font: 700 16pt 'Open Sans'; color: #1d1b20;");
                instrument_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                instrument_price->setFixedSize(336, 26);
                instrument_price->move(273, 86);

                // Кнопка добавления в корзину
                QPushButton* to_cart_ = new QPushButton(card);
                to_cart_->setObjectName("to_cart_");
                to_cart_->setIcon(QIcon(":/bookmark.svg"));
                to_cart_->setIconSize(QSize(20, 20));
                to_cart_->setStyleSheet("border: none; outline: none;");

                connect(to_cart_, &QPushButton::clicked, this, [this, instrument_info, to_cart_]() {
                    if (cart_.find(instrument_info.name_) == cart_.end()) {
                        to_cart_->setIcon(QIcon(":/bookmark_filled.svg"));
                        cart_[instrument_info.name_] = instruments_cards_[instrument_info.name_];
                        total_cost_ += instrument_info.price_;
                    }
                    else {
                        to_cart_->setIcon(QIcon(":/bookmark.svg"));
                        cart_.erase(cart_.find(instrument_info.name_));
                        total_cost_ -= instrument_info.price_;
                    }
                });
                to_cart_->move(589, 13);

                layout_->addWidget(card); // Добавляем карточку в компоновку
            }
        }

        // Устанавливаем обновленную компоновку для контейнера
        card_container_->setLayout(layout_.get());
    }
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

void MainWindow::MoreClicked()
{
    if (user_.get()){
        if (!hidden_to_cart_buttons_.empty()){
            RestoreHiddenToCartButtons();
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

void MainWindow::EnsureContainerInScrollArea(QScrollArea* target_scroll_area) {
    if (!target_scroll_area) {
        qDebug() << "Target scroll area is null!";
        return;
    }

    if (!card_container_) {
        qDebug() << "Card container is null!";
        return;
    }

    // Лог текущего родителя
    QObject* current_parent = card_container_->parent();
    QString current_parent_name = current_parent ? current_parent->objectName() : QString();
    qDebug() << "Moving card_container_ from" << current_parent_name
             << "to" << target_scroll_area->objectName();

    // Удаляем текущий виджет из ScrollArea, если он есть
    if (auto current_widget = target_scroll_area->widget()) {
        target_scroll_area->takeWidget();
        current_widget->setParent(nullptr);
    }

    // Устанавливаем card_container_ в QScrollArea
    target_scroll_area->setWidget(card_container_.get());

    // Обновляем размеры и видимость
    card_container_->adjustSize();            // Обновляем размеры содержимого
    card_container_->show();                  // Убеждаемся, что виджет виден
    target_scroll_area->viewport()->update(); // Обновляем область прокрутки

    qDebug() << "Successfully reparented card_container_ to"
             << target_scroll_area->objectName();
}

void MainWindow::ProfileClicked() {
    if (user_->GetRole() == Role::User) {
        side_widget_->setVisible(false);
        side_list_->setVisible(false);

        HideOldCards();
        EnsureContainerInScrollArea(ui->scrollArea_2);

        const auto& purchases_instruments_ = GetPurchasedInstruments(user_->GetId());
        for (auto& name_ : purchases_instruments_) {
            DrawItem(*instruments_->FindInstrument(name_));
            auto to_cart_button = instruments_cards_[name_]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
                to_cart_button->hide();
                hidden_to_cart_buttons_.insert(name_);
            }
        }
        card_container_->adjustSize();

        ui->label_clientname->setText(user_->GetName() + " — профиль");
        floating_menu_->move(970, 125);
        ui->stackedWidget->setCurrentWidget(ui->user_page);
    }
}

void MainWindow::CartClicked(){
    if (user_.get()){
        if (!hidden_to_cart_buttons_.empty()){
            RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            side_widget_->setVisible(false);
            side_list_->setVisible(false);

            if (cart_.empty()){
                ui->label_cart_total->setText("Корзина");
                ui->label_cart_total_2->setText("Корзина пуста.");
            }
            else {
                ui->label_cart_total->setText("Корзина — " + FormatPrice(total_cost_) + " руб.");
                ui->label_cart_total_2->setText(QString::number(cart_.size()) + " шт.");

                HideOldCards();
                EnsureContainerInScrollArea(ui->scrollArea_3);

                for (const auto& [name_, card_] : cart_) {
                    const auto& instrument = instruments_->FindInstrument(name_);
                    DrawItem(*instrument);
                }
                card_container_->adjustSize();
            }

            floating_menu_->move(970, 105);
            ui->stackedWidget->setCurrentWidget(ui->cart);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::CleanCart() {
    for (auto& [name_, card_] : cart_) {
        if (card_) {
            auto to_cart_button = card_->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
                to_cart_button->setIcon(QIcon(":/bookmark.svg"));
            }
        }
    }
    HideOldCards();
    ui->label_cart_total->setText("Корзина");
    ui->label_cart_total_2->setText("Корзина пуста.");
    cart_.clear();
    total_cost_ = 0;
}

void MainWindow::ToPayCart()
{
    // Начало транзакции
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию: " + db.lastError().text());
        return;
    }

    int new_id = db_manager_.GetMaxOrMinValueFromTable("MAX", "id", "purchases") + 1;
    bool success = true; // Флаг для отслеживания статуса операций

    for (auto& [name_, card_] : cart_) {
        auto instrument = instruments_->FindInstrument(name_);
        if (!db_manager_.ExecuteQuery(QString("INSERT INTO public.purchases(id, client_id, instrument_id) VALUES (%1, %2, %3);")
                                          .arg(new_id).arg(user_->GetId()).arg(instrument->id_)))
        {
            success = false;
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить операцию: " + db_manager_.GetLastError());
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
            cart_.clear(); // Очищаем корзину после успешной транзакции
            total_cost_ = 0; // Сбрасываем общую стоимость
        }
    }
    else {
        // Если есть ошибки, откатываем транзакцию
        if (!db.rollback()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось откатить транзакцию: " + db.lastError().text());
        }
    }
}

void MainWindow::DrawRelevantInstruments(const QString& term) {
    QList<InstrumentInfo> relevant_instruments = instruments_->FindRelevantInstruments(term);
    if (!relevant_instruments.empty()){

        HideOldCards();
        EnsureContainerInScrollArea(ui->scrollArea);

        for(const auto& instrument : relevant_instruments){
            DrawItem(instrument);
        }
        card_container_->adjustSize();

        MoreClicked();

        QMessageBox::information(this, "", "Найдено " + QString::number(relevant_instruments.size()) + " результатов по запросу «" + term + "».");
    }
    else {
        QMessageBox::warning(this, "Результаты поиска", "Отсутствуют релевантные результаты.");
    }
}

void MainWindow::HideOldCards() {
    for (auto& [name, card] : instruments_cards_) {
        card->hide();
    }
    card_container_->adjustSize();
}

void MainWindow::on_pushButton_clean_cart_clicked()
{
    if (cart_.empty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно очистить корзину: отсутствует содержимое.");
        return;
    }
    CleanCart();
    QMessageBox::information(this, "", "Корзина очищена.");
}

void MainWindow::on_pushButton_submit_cart_clicked()
{
    if (cart_.empty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно выполить оплату: отсутствует содержимое.");
        return;
    }
    ToPayCart();
    CleanCart();
    QMessageBox::information(this, "", "Произведена оплата. Купленные инструменты добавлены в профиль.");
}
