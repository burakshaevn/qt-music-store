#include "floating_widgets.h"

FloatingWidgets::FloatingWidgets(std::shared_ptr<DatabaseHandler> db_manager, QWidget* parent)
    : db_manager_(std::move(db_manager))
    , QWidget{parent}
{
    floating_menu_ = std::make_unique<QWidget>(parent);
    side_menu_ = std::make_unique<QWidget>(parent);
}

void FloatingWidgets::BuildFloatingMenu(
    const std::function<void()>& onMoreClicked,
    const std::function<void()>& onSearchClicked,
    const std::function<void()>& onCartClicked,
    const std::function<void()>& onUserProfileClicked
    ) {

    floating_menu_->setStyleSheet("background-color: #fafafa; border-radius: 29px;");
    floating_menu_->setFixedSize(74, 349);
    floating_menu_->setGeometry(970, 105, 74, 349);

    QVBoxLayout* menuLayout = new QVBoxLayout(floating_menu_.get());
    menuLayout->setContentsMargins(10, 20, 10, 20);
    menuLayout->setSpacing(20);

    // Кнопка More
    QPushButton* more_ = new QPushButton(floating_menu_.get());
    more_->setIcon(QIcon(":/more_horiz.svg"));
    more_->setIconSize(QSize(35, 35));
    more_->setStyleSheet("border: none; outline: none;");
    connect(more_, &QPushButton::clicked, this, [onMoreClicked]() {
        if (onMoreClicked) onMoreClicked();
    });

    // Кнопка Search
    QPushButton* searchButton = new QPushButton(floating_menu_.get());
    searchButton->setIcon(QIcon(":/search.svg"));
    searchButton->setIconSize(QSize(35, 35));
    searchButton->setStyleSheet("border: none; outline: none;");
    connect(searchButton, &QPushButton::clicked, this, [onSearchClicked]() {
        if (onSearchClicked) onSearchClicked();
    });

    // Кнопка Cart
    QPushButton* cart = new QPushButton(floating_menu_.get());
    cart->setIcon(QIcon(":/Shopping cart.svg"));
    cart->setIconSize(QSize(32, 32));
    cart->setStyleSheet("border: none; outline: none;");
    connect(cart, &QPushButton::clicked, this, [onCartClicked]() {
        if (onCartClicked) onCartClicked();
    });

    // Кнопка User Profile
    QPushButton* userButton = new QPushButton(floating_menu_.get());
    userButton->setIcon(QIcon(":/person.svg"));
    userButton->setIconSize(QSize(35, 35));
    userButton->setStyleSheet("border: none; outline: none;");
    connect(userButton, &QPushButton::clicked, this, [onUserProfileClicked]() {
        if (onUserProfileClicked) onUserProfileClicked();
    });

    // Добавляем кнопки в макет
    menuLayout->addWidget(more_);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(cart);
    menuLayout->addWidget(userButton);

    // Устанавливаем позицию и показываем меню
    floating_menu_->move(970, 105);
    floating_menu_->show();
}

void FloatingWidgets::BuildSideMenu(
    const std::function<void(const QString&)>& onItemClicked,
    const std::function<void()>& onViewAllClicked
    ) {
    // side_menu_ = std::make_unique<QWidget>(this);
    side_menu_->setStyleSheet("background-color: #fafafa;");
    side_menu_->setGeometry(0, 0, 224, 560);

    QLabel* title = new QLabel("Каталог", side_menu_.get());
    title->setStyleSheet("background-color: #fafafa; color: #140f10; font: 700 14pt 'Open Sans';");
    title->setGeometry(28, 87, 172, 19);

    side_list_ = new QListWidget(side_menu_.get());
    side_list_->setStyleSheet("background-color: #fafafa; color: #140f10; font: 12pt 'Open Sans'; border: 0px;");
    side_list_->setGeometry(22, 111, 224, 449);
    side_list_->setSpacing(3);

    // Заполнение данных из базы данных
    QSqlQuery query("SELECT name FROM instrument_types");
    while (query.next()) {
        QString instrumentTypeName = query.value("name").toString();
        side_list_->addItem(instrumentTypeName);
    }

    // Добавляем элемент "Смотреть всё"
    QListWidgetItem* viewAllItem = new QListWidgetItem("Смотреть всё", side_list_);
    viewAllItem->setTextAlignment(Qt::AlignLeft);
    viewAllItem->setFont(QFont("Open Sans", 12));
    viewAllItem->setForeground(QColor("#9b9c9c"));

    // Обработка сигналов кликов
    connect(side_list_, &QListWidget::itemClicked, this, [onItemClicked, onViewAllClicked](QListWidgetItem* item) {
        if (item->text() == "Смотреть всё") {
            if (onViewAllClicked) onViewAllClicked();
        } else {
            if (onItemClicked) onItemClicked(item->text());
        }
    });

    side_menu_->show();
}
