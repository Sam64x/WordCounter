#pragma once

#include <QObject>
#include <QUrl>
#include <QThread>

class WordCountWorker;
class WordFrequencyModel;

/**
 * Класс контроллера, который является посредником между UI и фоновым рабочим процессом.
 * Предоставляет свойства и методы QML для управления обработкой файлов.
 */
class Controller : public QObject {
    Q_OBJECT

    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    void setModel(WordFrequencyModel *model) { m_model = model; }

    QUrl fileUrl() const { return m_fileUrl; }
    double progress() const { return m_progress; }
    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }

    void setFileUrl(const QUrl &url) {
        if (m_fileUrl != url) {
            m_fileUrl = url;
            emit fileUrlChanged();
        }
    }

    Q_INVOKABLE void startProcessing();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void cancel();

signals:
    void fileUrlChanged();
    void progressChanged();
    void runningChanged();
    void pausedChanged();

private slots:
    void onWorkerProgress(int percent);
    void onWorkerResults(const QStringList &words, const QList<int> &counts);
    void onWorkerFinished();

private:
    WordFrequencyModel *m_model    = nullptr;
    QThread *m_thread   = nullptr;
    WordCountWorker *m_worker   = nullptr;
    QUrl m_fileUrl;
    double m_progress = 0.0;
    bool m_running  = false;
    bool m_paused   = false;
    bool m_canceled = false;
};
