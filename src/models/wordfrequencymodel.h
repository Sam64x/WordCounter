#pragma once
#include <QAbstractListModel>
#include <QString>
#include <QList>

/**
 * Модель, предоставляющая топ-15 слов и их количество в QML UI.
 * Наследует QAbstractListModel, поэтому может быть использована как модель в QML (например, с помощью Repeater)
 */

class WordFrequencyModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int maxCount READ maxCount NOTIFY maxCountChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum RoleNames { WordRole = Qt::UserRole + 1, CountRole };

    explicit WordFrequencyModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int maxCount() const { return m_maxCount; }
    void updateData(const QStringList &words, const QList<int> &counts);

signals:
    void maxCountChanged();
    void countChanged();

private:
    struct Entry { QString word; int count; };
    QList<Entry> m_entries;
    int m_maxCount = 0;
};
