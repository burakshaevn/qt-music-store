#include "product_card.h"

ProductCard::ProductCard(std::shared_ptr<DatabaseHandler> db_manager, std::shared_ptr<Instruments> instruments, QObject *parent)
    : db_manager_(std::move(db_manager))
    , instruments_(std::move(instruments))
    , QObject{parent}
{
    // Создаем card_container_
    card_container_ = new QWidget();
    card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для карточек
    layout_ = new QVBoxLayout(card_container_);
    layout_->setAlignment(Qt::AlignTop);
    layout_->setSpacing(15);
    layout_->setContentsMargins(10, 10, 10, 10);
}

void ProductCard::SetInstrumentsPtr(std::shared_ptr<Instruments> instruments) {
    instruments_ = std::move(instruments);
}

void ProductCard::DrawItem(const InstrumentInfo& instrument) {
    if (instruments_cards_.contains(instrument.name_)) {
        QWidget* card = instruments_cards_[instrument.name_];
        if (!layout_->indexOf(card)) {
            card->setParent(card_container_);
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

int ProductCard::DrawRelevantInstruments(QScrollArea* scrollArea,const QString& term) {
    QList<InstrumentInfo> relevant_instruments = instruments_.lock()->FindRelevantInstruments(term);
    if (!relevant_instruments.empty()){

        HideOldCards();
        EnsureContainerInScrollArea(scrollArea);

        for(const auto& instrument : relevant_instruments){
            DrawItem(instrument);
        }
        card_container_->adjustSize();

        return relevant_instruments.size();
    }
    return 0;
}

void ProductCard::RestoreHiddenToCartButtons() {
    for (const auto& name : hidden_to_cart_buttons_) {
        auto to_cart_button = instruments_cards_[name]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
        if (to_cart_button) {
            to_cart_button->show();
        }
    }
    hidden_to_cart_buttons_.clear();
}

void ProductCard::UpdateInstrumentsWidget(QScrollArea* scrollArea, const QString& filter) {
    int id = -1;
    if (filter != "Смотреть всё") {
        auto result = db_manager_.lock()->ExecuteSelectQuery(QString("SELECT id FROM instrument_types WHERE name = '%1'").arg(filter));
        if (result.canConvert<QSqlQuery>()) {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next()) {
                id = query.value("id").toInt();
            }
        }
    }

    HideOldCards();
    EnsureContainerInScrollArea(scrollArea);

    for (auto& [name, card] : instruments_cards_) {
        const auto& instrument = instruments_.lock()->FindInstrument(name);

        bool shouldDisplay = (filter == "Смотреть всё" || instrument->type_id_ == id);
        if (shouldDisplay) {
            DrawItem(*instrument);
        }
    }
    card_container_->adjustSize();
}

void ProductCard::EnsureContainerInScrollArea(QScrollArea* target_scroll_area) {
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
    // qDebug() << "Moving card_container_ from" << current_parent_name
    //          << "to" << target_scroll_area->objectName();

    // Удаляем текущий виджет из ScrollArea, если он есть
    if (auto current_widget = target_scroll_area->widget()) {
        target_scroll_area->takeWidget();
        current_widget->setParent(nullptr);
    }

    // Устанавливаем card_container_ в QScrollArea
    target_scroll_area->setWidget(card_container_);

    // Обновляем размеры и видимость
    card_container_->adjustSize();            // Обновляем размеры содержимого
    card_container_->show();                  // Убеждаемся, что виджет виден
    target_scroll_area->viewport()->update(); // Обновляем область прокрутки

    // qDebug() << "Successfully reparented card_container_ to" << target_scroll_area->objectName();
}

void ProductCard::HideOldCards() {
    for (auto& [name, card] : instruments_cards_) {
        card->hide();
    }
    card_container_->adjustSize();
}

QWidget* ProductCard::FindInstrumentCard(const QString& instrument_name) {
    if (!instruments_cards_.empty()){
        if (instruments_cards_.contains(instrument_name)){
            return instruments_cards_[instrument_name];
        }
    }
    return nullptr;
}

void ProductCard::AddInstrumentCard(const QString& instrument_name, QWidget* instrument_card){
    instruments_cards_[instrument_name] = instrument_card;
}
