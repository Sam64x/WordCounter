#pragma once
#include <QObject>
#include <QHash>
#include <QMutex>
#include <QWaitCondition>

/**
 * Worker для асинхронного подсчёта слов в большом файле.
 * Использует чанковое чтение из файла (64 KiB)
 */
class WordCountWorker : public QObject
{
    Q_OBJECT
public:
    explicit WordCountWorker(QString filePath);
    ~WordCountWorker() override;

public slots:
    void process();
    void pause();
    void resume();
    void cancel();

signals:
    void resultsReady(const QStringList &topWords,
                      const QList<int> &topCounts);
    void progressUpdated(int percent);
    void finished();

private:
    void emitIntermediate(int percent);
    void computeTop15(QStringList &words,
                      QList<int>  &counts) const;

    QString              m_filePath;
    QHash<QString,int>   m_freqMap;

    mutable QMutex       m_mutex;
    QWaitCondition       m_pauseCond;
    bool                 m_pause  = false;
    bool                 m_cancel = false;
};
