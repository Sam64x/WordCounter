#include "wordcountworker.h"

#include <QFile>
#include <QStringDecoder>
#include <QRegularExpression>
#include <algorithm>

WordCountWorker::WordCountWorker(QString filePath)
    : m_filePath(std::move(filePath)) {}

WordCountWorker::~WordCountWorker() {}

void WordCountWorker::process()
{
    static const int  CHUNK_SIZE = 64 * 1024;
    static const QRegularExpression WS_RE(
        QStringLiteral("[^\\p{L}\\p{Nd}']+"),
        QRegularExpression::UseUnicodePropertiesOption);

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit finished();
        return;
    }

    const qint64 totalBytes = file.size();
    qint64 bytesRead = 0; // сколько уже прочитали
    int chunkCounter = 0; // для периодичности emit'ов

    QStringDecoder decoder(QStringConverter::System);
    QString pending; // хвост незаконченного слова

    while (!file.atEnd()) {
        {
            QMutexLocker locker(&m_mutex);
            if (m_cancel) break;
            while (m_pause && !m_cancel)
                m_pauseCond.wait(&m_mutex);
            if (m_cancel) break;
        }

        QByteArray buffer = file.read(CHUNK_SIZE);
        bytesRead += buffer.size();
        ++chunkCounter;
        QString chunk = decoder(buffer);      // stateful: «склеит» разорванный символ
        QString data = pending + chunk;      // плюс хвост с прошлого прохода
        pending.clear();

        /* --- Токенизация --------------------------------------- */
        bool endsWithSpace = !data.isEmpty() && data.at(data.size()-1).isSpace();

        QStringList tokens = data.split(WS_RE, Qt::SkipEmptyParts);

        // Если конец куска не whitespace — последний токен может быть неполным
        if (!endsWithSpace && !tokens.isEmpty()) {
            pending = tokens.takeLast();      // откладываем до следующего чанка
        }

        for (const QString &t : std::as_const(tokens))
            ++m_freqMap[t];

        // Периодически шлём результаты
        if (chunkCounter % 4 == 0 || file.atEnd()) { // каждые 256 KiB
            int percent = totalBytes
                          ? int(bytesRead * 100 / totalBytes)
                          : 0;
            if (percent > 100) percent = 100;
            emitIntermediate(percent);
            chunkCounter = 0;
        }
    }

    // учесть последнее слово, если файл не оканчивался whitespace
    if (!pending.isEmpty())
        ++m_freqMap[pending];

    // финальный отчёт (учитываем отмену)
    const bool wasCanceled = [this]{
        QMutexLocker locker(&m_mutex);
        return m_cancel;
    }();

    int percent = totalBytes ? int(bytesRead * 100 / totalBytes) : 0;
    if (!wasCanceled) percent = 100;
    if (percent > 100) percent = 100;

    emitIntermediate(percent);
    emit finished();
}

void WordCountWorker::emitIntermediate(int percent)
{
    QStringList words;
    QList<int>  counts;
    computeTop15(words, counts);

    emit resultsReady(words, counts);
    emit progressUpdated(percent);
}

void WordCountWorker::computeTop15(QStringList &words,
                                   QList<int>  &counts) const
{
    QVector<QPair<QString,int>> vec;
    vec.reserve(m_freqMap.size());
    for (auto it = m_freqMap.constBegin(); it != m_freqMap.constEnd(); ++it)
        vec.append({it.key(), it.value()});

    std::sort(vec.begin(), vec.end(),
              [](auto &a, auto &b){ return a.second > b.second; });

    int n = qMin(15, vec.size());
    words.clear(); counts.clear();
    for (int i = 0; i < n; ++i) {
        words  << vec[i].first;
        counts << vec[i].second;
    }
}

void WordCountWorker::pause()
{
    QMutexLocker locker(&m_mutex);
    m_pause = true;
}
void WordCountWorker::resume()
{
    QMutexLocker locker(&m_mutex);
    m_pause = false;
    m_pauseCond.wakeAll();
}
void WordCountWorker::cancel()
{
    QMutexLocker locker(&m_mutex);
    m_cancel = true;
    m_pauseCond.wakeAll();
}
