#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QHash>

#include <unordered_map>

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
    int id_;
    QString name_;
    int type_id_;
    int price_;
    QString description_;
    QString image_path_;
};

struct InstrumentsHasher {
    std::size_t operator()(const QString& key) const {
        return qHash(key);
    }
};

class Instruments
{
public:
    using Container = std::unordered_map<QString, InstrumentInfo, InstrumentsHasher>;

    Instruments() = default;

    inline void PushInstrument(const InstrumentInfo& instrument) {
        instruments_[instrument.name_] = instrument;
    }

    inline void Clear() {
        instruments_.clear();
    }

    inline Container GetInstruments() const {
        return instruments_;
    }

    inline const InstrumentInfo* FindInstrument(const QString& instrument_name) const {
        auto iter = instruments_.find(instrument_name);
        if (iter != instruments_.end()) {
            return &iter->second;
        }
        else {
            return nullptr;
        }
    }

    QList<InstrumentInfo> FindRelevantInstruments(const QString& term) const {
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

private:
    Container instruments_; // <Instrument Name, InstrumentInfo>

    double ComputeTfIdf(const QString& document, const QString& term) const {
        // Подсчет частоты термина (TF — Term Frequency)
        int term_frequency = CountOccurrences(document, term);
        int total_terms = CountTotalWords(document);
        double tf = total_terms > 0 ? static_cast<double>(term_frequency) / total_terms : 0.0;

        // Подсчет обратной частотности (IDF — Inverse Document Frequency)
        int idf = 1.0;

        // Итоговое значение TF-IDF
        return tf * idf;
    }

    int CountOccurrences(const QString& document, const QString& term) const {
        int count = 0;
        QRegularExpression regex("\\b" + QRegularExpression::escape(term) + "\\b", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = regex.globalMatch(document);
        while (it.hasNext()) {
            it.next();
            ++count;
        }
        return count;
    }

    /**
     * Подсчет общего количества слов в документе.
     */
    int CountTotalWords(const QString& document) const {
        QRegularExpression wordRegex("\\s+"); // Используем регулярное выражение для разделения по пробелам
        return document.split(wordRegex, Qt::SkipEmptyParts).size();
    }
};

#endif // INSTRUMENTS_H
