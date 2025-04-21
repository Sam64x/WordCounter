# 2GIS_TESTING Application

## Описание
Это многопоточное Qt‑приложение (Qt 6.5+, C++17 + QML) для быстрой визуализации самых частых слов из большого текстового файла в режиме реального времени.

## Зависимости
C++17 or later
Qt 6.5 or later

## Как реализована многопоточность?
1. Создание и запуск рабочего потока

В методе Controller::startProcessing():

```cpp
    m_thread = new QThread(this);
    m_worker = new WordCountWorker(path);
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_worker, &WordCountWorker::process);
    m_thread->start();
```

По сигналу started() у воркера вызываем слот process() весь парсинг идёт не в UI‑потоке, а данном фоновом.

2. Обмен данными через сигналы и слоты

Все обновления UI (прогресс‑бар, модель данных) мы делаем только в UI‑потоке, в слотах Controller:

```cpp
    connect(m_worker, &WordCountWorker::progressUpdated, this, &Controller::onWorkerProgress);
    connect(m_worker, &WordCountWorker::resultsReady,   this, &Controller::onWorkerResults);
```

UI‑поток лишь получает раз в 256 KiB небольшие порции данных (топ‑15 слов и процент), обновляет модель и перерисовывает гистограмму.
Qt queued connections между потоками не блокирует ни UI‑поток, ни worker: сигналы просто кладутся в очередь, и потоки продолжают работать независимо.

## Архитектурные решения
1. C++
* Класс WordCountWorker (QObject в собственном QThread): отвечает за чтение файла по чанкам (64 KiB).
    - Корректное декодирование (QStringDecoder) позволяет читать файлы в формате операционной системы и UTF-8.
    - Поддержка pause/resume через QMutex + QWaitCondition, а также cancel — установка флага и выход из цикла.

* Класс Controller (QObject в GUI-потоке): создаёт и управляет потоком/воркером, обрабатывает сигналы
    - `startProcessing()` сбрасывает внутренние флаги, создаёт QThread и WordCountWorker, подключает сигналы/слоты, запускает поток.
    - `onWorkerProgress(int)` обновляет свойство progress (Q_PROPERTY) для ProgressBar.
    - `onWorkerResults(...)` вызывает model->updateData(words, counts), если не отменено.
    - `cancel()` всегда очищает модель (updateData), сбрасывает progress, флаги running=false, canceled=true, и отправляет воркеру cancel().

* Класс WordFrequencyModel (QAbstractListModel): хранит список m_entries (слово + счёт), 
    - обновляемый целиком через `updateData()`.
    - поддерживает `Q_PROPERTY maxCount` (максимальное значение для масштабирования гистограммы) `count` (число элементов) 
    - `onWorkerFinished()` сбрасывает running/paused/canceled для готовности к следующему запуску.
        
2. QML
* Гистограма:
    - Отрисовка гистрограммы реализована при помощи примитивов Repeater, Rectangle
    - Размерность рассчитывается адаптивно по пропорции взависимости от значения maxCount 
* Адаптивность интерфейса достигается путем использования Layouts, anchors 
    - отображаемый текст при уменьшении скрывается, не происходит наложения, при наведении (hover) видно подсказку (ToolTip)
* Стили:
    - Применены стили `QQuickStyle::setStyle("FluentWinUI3")` для QML компонентов
