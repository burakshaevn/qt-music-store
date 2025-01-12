#pragma once

#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QHash>
#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QCoreApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QMainWindow>

#include <unordered_map>

class DatabaseHandler;
class ProductCard;
class Cart;

struct InstrumentInfo
{
    InstrumentInfo() = default;
    explicit InstrumentInfo(const int id, const QString name, const int type_id, const int price, const QString description, const QString image_path)
        : id_(id)
        , name_(name)
        , type_id_(type_id)
        , price_(price)
        , description_(description)
        , image_path_(image_path)
    {}
    int id_ = 0;
    QString name_ = "";
    int type_id_ = 0;
    int price_ = 0;
    QString description_ = "";
    QString image_path_ = "";
};

struct InstrumentsHasher {
    std::size_t operator()(const QString& key) const {
        return qHash(key);
    }
};

class Instruments : public QObject
{
    Q_OBJECT
public:
    using Container = std::unordered_map<QString, InstrumentInfo, InstrumentsHasher>;

    explicit Instruments(std::shared_ptr<ProductCard> product_card,
                         std::shared_ptr<Cart> cart,
                         std::shared_ptr<DatabaseHandler> db_manager);

    void PushInstrument(const InstrumentInfo& instrument);

    void Clear();

    Container GetInstruments() const;

    const InstrumentInfo* FindInstrument(const QString& instrument_name) const;

    QList<InstrumentInfo> FindRelevantInstruments(const QString& term) const;

    // Кэширование инструментов из базы данных.
    // Загружаем данные об инструментах в instruments_ как в основное хранилище
    void PullInstruments();

signals:
    // Сигнал, который отправляет общую стоимость корзины
    void CartUpdated(double totalCost);

private:
    Container instruments_; // <Instrument Name, InstrumentInfo>
    
    std::shared_ptr<DatabaseHandler> db_manager_;
    std::weak_ptr<ProductCard> product_card_;
    std::shared_ptr<Cart> cart_;

    inline double ComputeTfIdf(const QString& document, const QString& term) const;

    inline int CountOccurrences(const QString& document, const QString& term) const;

    // Подсчет общего количества слов в документе.
    int CountTotalWords(const QString& document) const;
};

#endif // INSTRUMENTS_H
