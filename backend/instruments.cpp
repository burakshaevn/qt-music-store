#include "instruments.h"

#include "database_handler.h"
#include "cart.h"
#include "domain.h"
#include "product_card.h"

Instruments::Instruments(std::shared_ptr<ProductCard> product_card,
                         std::shared_ptr<Cart> cart,
                         std::shared_ptr<DatabaseHandler> db_manager)
    : product_card_(std::move(product_card))
    , cart_(std::move(cart))
    , db_manager_(std::move(db_manager))
{}

void Instruments::PushInstrument(const InstrumentInfo& instrument) {
    instruments_[instrument.name_] = instrument;
}

void Instruments::Clear() {
    instruments_.clear();
}

Instruments::Container Instruments::GetInstruments() const {
    return instruments_;
}

const InstrumentInfo* Instruments::FindInstrument(const QString& instrument_name) const {
    auto iter = instruments_.find(instrument_name);
    if (iter != instruments_.end()) {
        return &iter->second;
    }
    else {
        return nullptr;
    }
}

QList<InstrumentInfo> Instruments::FindRelevantInstruments(const QString& term) const {
    // Хранилище для всех инструментов и их релевантности
    QList<std::pair<InstrumentInfo, double>> scores;

    // Вычисляем TF-IDF для каждого инструмента
    for (const auto& [name, info] : instruments_) {
        double tf_idf = ComputeTfIdf(info.name_, term);
        scores.append({info, tf_idf});
    }

    // Сортируем результаты по убыванию TF-IDF
    std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
        return a.second > b.second; // Сортировка по убыванию релевантности
    });

    // Создаем результирующий список инструментов
    QList<InstrumentInfo> result;
    for (const auto& pair : scores) {
        if (pair.second > 0.0) { // Добавляем только инструменты с релевантностью > 0
            result.append(pair.first);
        }
    }
    return result;
}

void Instruments::PullInstruments() {
    // Выполняем запрос к базе данных
    auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.instruments ORDER BY id ASC"));
    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();

        // Загружаем инструменты в instruments_
        Clear();
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

            PushInstrument(instrument);
        }

        // Создаем карточки для инструментов
        for (const auto& [name, instrument_info] : GetInstruments()) {
            if (product_card_.lock()->FindInstrumentCard(instrument_info.name_) == nullptr) {
                QWidget* card = new QWidget(product_card_.lock()->GetCardContainer());
                product_card_.lock()->AddInstrumentCard(instrument_info.name_, card);

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
                default:
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
                    if (cart_->InstrumentInCart(instrument_info.name_)) {
                        to_cart_->setIcon(QIcon(":/bookmark_filled.svg"));
                        cart_->AddToCart(instrument_info.name_);
                        cart_->PlusToTotalCost(instrument_info.price_);
                    }
                    else {
                        to_cart_->setIcon(QIcon(":/bookmark.svg"));
                        cart_->DeleteFromCart(instrument_info.name_);
                        cart_->MinusToTotalCost(instrument_info.price_);
                    }
                    emit CartUpdated(cart_->GetTotalCost());
                });
                to_cart_->move(589, 13);

                // Добавляем карточку в компоновку
                product_card_.lock()->AddWidgetToLayout(card);
            }
        }

        // Устанавливаем обновленную компоновку для контейнера
        product_card_.lock()->UpdateCardContainer();
    }
}

double Instruments::ComputeTfIdf(const QString& document, const QString& term) const {
    // Подсчет частоты термина (TF — Term Frequency)
    int term_frequency = CountOccurrences(document, term);
    int total_terms = CountTotalWords(document);
    double tf = total_terms > 0 ? static_cast<double>(term_frequency) / total_terms : 0.0;

    // Подсчет обратной частотности (IDF — Inverse Document Frequency)
    int idf = 1.0;

    // Итоговое значение TF-IDF
    return tf * idf;
}

int Instruments::CountOccurrences(const QString& document, const QString& term) const {
    int count = 0;
    QRegularExpression regex("\\b" + QRegularExpression::escape(term) + "\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = regex.globalMatch(document);
    while (it.hasNext()) {
        it.next();
        ++count;
    }
    return count;
}

// Подсчет общего количества слов в документе.
int Instruments::CountTotalWords(const QString& document) const {
    QRegularExpression wordRegex("\\s+"); // Используем регулярное выражение для разделения по пробелам
    return document.split(wordRegex, Qt::SkipEmptyParts).size();
}
