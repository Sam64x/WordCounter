#include "wordfrequencymodel.h"

int WordFrequencyModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_entries.size();
}

QVariant WordFrequencyModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    const Entry &entry = m_entries.at(index.row());
    if (role == WordRole) {
        return entry.word;
    } else if (role == CountRole) {
        return entry.count;
    }
    return {};
}

QHash<int, QByteArray> WordFrequencyModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[WordRole] = "word";
    roles[CountRole] = "count";
    return roles;
}

void WordFrequencyModel::updateData(const QStringList &words, const QList<int> &counts) {
    beginResetModel();
    m_entries.clear();
    m_maxCount = 0;
    int n = qMin(words.size(), counts.size());
    m_entries.reserve(n);
    for (int i = 0; i < n; ++i) {
        Entry e { words[i], counts[i] };
        m_entries.append(e);
        if (e.count > m_maxCount) {
            m_maxCount = e.count;
        }
    }
    endResetModel();
    emit maxCountChanged();
    emit countChanged();
}
