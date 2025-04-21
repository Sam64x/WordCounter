#include "controller.h"
#include "workers/wordcountworker.h"
#include "models/wordfrequencymodel.h"

#include <QFileInfo>
#include <QMetaObject>
#include <QDebug>

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    // Необходим для того, чтобы QStringList и QList<int> могли проходить через сигналы/слоты
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType<QList<int>>("QList<int>");
}

void Controller::startProcessing()
{
    if (m_running || m_fileUrl.isEmpty())
        return;

    QString path = m_fileUrl.toLocalFile();
    if (!QFileInfo(path).isFile()) {
        qWarning() << "Invalid file:" << path;
        return;
    }

    m_canceled = false;

    m_thread = new QThread(this);
    m_worker = new WordCountWorker(path);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started,
            m_worker, &WordCountWorker::process);
    connect(m_worker, &WordCountWorker::progressUpdated,
            this, &Controller::onWorkerProgress);
    connect(m_worker, &WordCountWorker::resultsReady,
            this, &Controller::onWorkerResults);
    connect(m_worker, &WordCountWorker::finished,
            this, &Controller::onWorkerFinished);

    connect(m_worker, &WordCountWorker::finished,
            m_thread, &QThread::quit);
    connect(m_worker, &WordCountWorker::finished,
            m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished,
            m_thread, &QObject::deleteLater);

    m_running = true;
    m_paused = false;
    m_progress = 0.0;
    emit runningChanged();
    emit pausedChanged();
    emit progressChanged();

    m_thread->start();
}

void Controller::pause()
{
    if (m_running && !m_paused && m_worker) {
        m_paused = true;
        emit pausedChanged();
        m_worker->pause();
    }
}

void Controller::resume()
{
    if (m_running && m_paused && m_worker) {
        m_paused = false;
        emit pausedChanged();
        m_worker->resume();
    }
}

void Controller::cancel()
{
    if (m_model) {
        m_model->updateData({}, {});
    }
    m_progress = 0;
    emit progressChanged();

    if (m_running && m_worker) {
        m_canceled = true;
        m_paused   = false;
        emit pausedChanged();

        m_worker->cancel();
    }

    if (m_running) {
        m_running = false;
        emit runningChanged();
    }
}

void Controller::onWorkerProgress(int percent)
{
    if (!m_canceled) {
        m_progress = percent;
        emit progressChanged();
    }
}

void Controller::onWorkerResults(const QStringList &words,
                                 const QList<int> &counts)
{
    // Игнорировать любые результаты после отмены
    if (!m_canceled && m_model) {
        m_model->updateData(words, counts);
    }
}

void Controller::onWorkerFinished()
{
    m_running  = false;
    m_paused   = false;
    m_canceled = false;

    emit runningChanged();
    emit pausedChanged();
}
