#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();
    loadSettings();
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);


    // ================= PATHS =================

    auto* pathsBox = new QGroupBox("Paths");
    auto* pathsLayout = new QGridLayout(pathsBox);
    auto* browseLandMask = new QPushButton("Browse");
    buildOceanCacheButton = new QPushButton("Build Ocean Cache");

    landMaskEdit = new QLineEdit(R"(D:\ArnisTiles\ne_110m_land.geojson)");
    arnisPathEdit = new QLineEdit(R"(D:\ARNIS\arnis.exe)");
    workFolderEdit = new QLineEdit(R"(D:\ArnisTiles\working)");
    doneFolderEdit = new QLineEdit(R"(D:\ArnisTiles\done)");
    oceanCacheEdit = new QLineEdit(R"(D:\ArnisTiles\ocean_cache.csv)");
    oceanSampleGridSpin = new QSpinBox();
    oceanSampleGridSpin->setRange(1, 25);
    oceanSampleGridSpin->setValue(7);

    auto* browseArnis = new QPushButton("Browse");
    auto* browseWork = new QPushButton("Browse");
    auto* browseDone = new QPushButton("Browse");
    auto* browseOcean = new QPushButton("Browse");

    connect(buildOceanCacheButton, &QPushButton::clicked, this, &MainWindow::buildOceanCache);

    connect(browseLandMask, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this,
            "Select land GeoJSON",
            QString(),
            "GeoJSON files (*.geojson *.json)"
        );

        if (!file.isEmpty())
            landMaskEdit->setText(file);
        });

    connect(browseArnis, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Select Arnis executable");
        if (!file.isEmpty())
            arnisPathEdit->setText(file);
        });

    connect(browseWork, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select working folder");
        if (!dir.isEmpty())
            workFolderEdit->setText(dir);
        });

    connect(browseDone, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select done folder");
        if (!dir.isEmpty())
            doneFolderEdit->setText(dir);
        });

    connect(browseOcean, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Select ocean_cache.csv", QString(), "CSV files (*.csv)");
        if (!file.isEmpty())
            oceanCacheEdit->setText(file);
        });

    pathsLayout->addWidget(new QLabel("Arnis executable:"), 0, 0);
    pathsLayout->addWidget(arnisPathEdit, 0, 1);
    pathsLayout->addWidget(browseArnis, 0, 2);

    pathsLayout->addWidget(new QLabel("Working folder:"), 1, 0);
    pathsLayout->addWidget(workFolderEdit, 1, 1);
    pathsLayout->addWidget(browseWork, 1, 2);

    pathsLayout->addWidget(new QLabel("Done folder:"), 2, 0);
    pathsLayout->addWidget(doneFolderEdit, 2, 1);
    pathsLayout->addWidget(browseDone, 2, 2);

    pathsLayout->addWidget(new QLabel("Ocean cache CSV:"), 3, 0);
    pathsLayout->addWidget(oceanCacheEdit, 3, 1);
    pathsLayout->addWidget(browseOcean, 3, 2);

    pathsLayout->addWidget(new QLabel("Land GeoJSON:"), 4, 0);
    pathsLayout->addWidget(landMaskEdit, 4, 1);
    pathsLayout->addWidget(browseLandMask, 4, 2);

    // ================= SETTINGS =================

    auto* settingsBox = new QGroupBox("Generation Settings");
    auto* settingsLayout = new QGridLayout(settingsBox);

    westSpin = new QDoubleSpinBox();
    eastSpin = new QDoubleSpinBox();
    southSpin = new QDoubleSpinBox();
    northSpin = new QDoubleSpinBox();

    for (auto* spin : { westSpin, eastSpin, southSpin, northSpin }) {
        spin->setRange(-180.0, 180.0);
        spin->setDecimals(6);
    }

    westSpin->setValue(-170);
    eastSpin->setValue(-50);
    southSpin->setValue(10);
    northSpin->setValue(75);

    stepLonSpin = new QDoubleSpinBox();
    stepLatSpin = new QDoubleSpinBox();
    scaleSpin = new QDoubleSpinBox();
    oceanThresholdSpin = new QDoubleSpinBox();

    stepLonSpin->setRange(0.001, 20.0);
    stepLatSpin->setRange(0.001, 20.0);
    scaleSpin->setRange(0.001, 10.0);
    oceanThresholdSpin->setRange(0.0, 1.0);

    stepLonSpin->setDecimals(6);
    stepLatSpin->setDecimals(6);
    scaleSpin->setDecimals(6);
    oceanThresholdSpin->setDecimals(2);

    stepLonSpin->setValue(1.0);
    stepLatSpin->setValue(1.0);
    scaleSpin->setValue(0.015);
    oceanThresholdSpin->setValue(0.60);

    workersSpin = new QSpinBox();
    workersSpin->setRange(1, 16);
    workersSpin->setValue(2);

    timeoutSpin = new QSpinBox();
    timeoutSpin->setRange(60, 99999);
    timeoutSpin->setValue(600);

    avgTileTimeSpin = new QSpinBox();
    avgTileTimeSpin->setRange(1, 99999);
    avgTileTimeSpin->setValue(120);

    terrainCheck = new QCheckBox("Terrain");
    interiorCheck = new QCheckBox("Interior");
    roofCheck = new QCheckBox("Roof");
    useOceanCacheCheck = new QCheckBox("Use ocean cache skip");

    terrainCheck->setChecked(true);
    interiorCheck->setChecked(false);
    roofCheck->setChecked(true);
    useOceanCacheCheck->setChecked(true);

    int row = 0;

    settingsLayout->addWidget(new QLabel("West:"), row, 0);
    settingsLayout->addWidget(westSpin, row, 1);
    settingsLayout->addWidget(new QLabel("East:"), row, 2);
    settingsLayout->addWidget(eastSpin, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("South:"), row, 0);
    settingsLayout->addWidget(southSpin, row, 1);
    settingsLayout->addWidget(new QLabel("North:"), row, 2);
    settingsLayout->addWidget(northSpin, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("Step lon:"), row, 0);
    settingsLayout->addWidget(stepLonSpin, row, 1);
    settingsLayout->addWidget(new QLabel("Step lat:"), row, 2);
    settingsLayout->addWidget(stepLatSpin, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("Scale:"), row, 0);
    settingsLayout->addWidget(scaleSpin, row, 1);
    settingsLayout->addWidget(new QLabel("Timeout:"), row, 2);
    settingsLayout->addWidget(timeoutSpin, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("Workers:"), row, 0);
    settingsLayout->addWidget(workersSpin, row, 1);
    settingsLayout->addWidget(new QLabel("Avg tile seconds:"), row, 2);
    settingsLayout->addWidget(avgTileTimeSpin, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("Ocean threshold:"), row, 0);
    settingsLayout->addWidget(oceanThresholdSpin, row, 1);
    settingsLayout->addWidget(new QLabel("Ocean sample grid:"), row, 2);
    settingsLayout->addWidget(oceanSampleGridSpin, row, 3);

    row++;
    settingsLayout->addWidget(terrainCheck, row, 0);
    settingsLayout->addWidget(interiorCheck, row, 1);
    settingsLayout->addWidget(roofCheck, row, 2);
    settingsLayout->addWidget(useOceanCacheCheck, row, 3);

    // ================= BUTTONS =================

    estimateButton = new QPushButton("Estimate");
    startButton = new QPushButton("Start");
    stopButton = new QPushButton("Stop");
    saveSettingsButton = new QPushButton("Save Settings");
    loadSettingsButton = new QPushButton("Load Settings");

    stopButton->setEnabled(false);

    connect(estimateButton, &QPushButton::clicked, this, &MainWindow::estimateTiles);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startGeneration);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopGeneration);
    connect(saveSettingsButton, &QPushButton::clicked, this, &MainWindow::saveSettings);
    connect(loadSettingsButton, &QPushButton::clicked, this, &MainWindow::loadSettings);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(estimateButton);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(buildOceanCacheButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveSettingsButton);
    buttonLayout->addWidget(loadSettingsButton);


    // ================= STATUS =================

    estimateLabel = new QLabel("Estimate: not calculated yet");
    statusLabel = new QLabel("Status: idle");

    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    logBox = new QPlainTextEdit();
    logBox->setReadOnly(true);

    mainLayout->addWidget(pathsBox);
    mainLayout->addWidget(settingsBox);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(estimateLabel);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(logBox, 1);

    setCentralWidget(central);
    setWindowTitle("Arnis Globe Builder");
}

QVector<Tile> MainWindow::createTiles() const
{
    QVector<Tile> result;

    double west = westSpin->value();
    double east = eastSpin->value();
    double south = southSpin->value();
    double north = northSpin->value();

    double stepLon = stepLonSpin->value();
    double stepLat = stepLatSpin->value();

    int x = 0;

    for (double lon = west; lon < east; lon += stepLon) {
        int y = 1;

        for (double lat = south; lat < north; lat += stepLat) {
            Tile tile;
            tile.name = QString("%1_%2").arg(x).arg(y);
            tile.west = lon;
            tile.east = std::min(lon + stepLon, east);
            tile.south = lat;
            tile.north = std::min(lat + stepLat, north);

            result.append(tile);
            y++;
        }

        x++;
    }

    return result;
}

QHash<QString, double> MainWindow::loadOceanCache() const
{
    QHash<QString, double> cache;

    QFile file(oceanCacheEdit->text());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return cache;

    QTextStream in(&file);
    bool firstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty())
            continue;

        if (firstLine) {
            firstLine = false;
            if (line.toLower().contains("tile"))
                continue;
        }

        QStringList parts = line.split(",");
        if (parts.size() < 2)
            continue;

        QString tileName = parts[0].trimmed();
        bool ok = false;
        double oceanPercent = parts[1].trimmed().toDouble(&ok);

        if (ok)
            cache[tileName] = oceanPercent;
    }

    return cache;
}

QVector<Tile> MainWindow::applyOceanCacheSkipping(const QVector<Tile>& tiles)
{
    QVector<Tile> kept;
    skippedOcean = 0;

    if (!useOceanCacheCheck->isChecked()) {
        return tiles;
    }

    QHash<QString, double> cache = loadOceanCache();

    if (cache.isEmpty()) {
        log("Ocean cache is empty or missing. Ocean skipping will not be applied.");
        return tiles;
    }

    double threshold = oceanThresholdSpin->value();

    for (Tile tile : tiles) {
        if (cache.contains(tile.name)) {
            tile.oceanPercent = cache.value(tile.name);

            if (tile.oceanPercent >= threshold) {
                skippedOcean++;
                writeProgressCsv(tile, "ocean_skipped", QString("%1% ocean").arg(tile.oceanPercent * 100.0, 0, 'f', 1));
                continue;
            }
        }

        kept.append(tile);
    }

    return kept;
}

QSet<QString> MainWindow::loadDoneTilesFromFolders() const
{
    QSet<QString> done;

    QDir dir(doneFolderEdit->text());
    if (!dir.exists())
        return done;

    QFileInfoList folders = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : folders) {
        done.insert(info.fileName());
    }

    return done;
}

void MainWindow::estimateTiles()
{
    allTiles = createTiles();
    runnableTiles = applyOceanCacheSkipping(allTiles);

    QSet<QString> doneFolders = loadDoneTilesFromFolders();

    int alreadyDone = 0;
    for (const Tile& tile : runnableTiles) {
        if (doneFolders.contains(tile.name))
            alreadyDone++;
    }

    int tilesToGenerate = runnableTiles.size() - alreadyDone;

    double workers = std::max(1, workersSpin->value());
    double estimatedSeconds = (tilesToGenerate * avgTileTimeSpin->value()) / workers;

    double areaDeg = (eastSpin->value() - westSpin->value()) *
        (northSpin->value() - southSpin->value());

    double estimatedGb = areaDeg * 20.0 * std::pow(scaleSpin->value() / 0.2, 2.0);

    if (!allTiles.isEmpty()) {
        estimatedGb *= double(runnableTiles.size()) / double(allTiles.size());
    }

    estimateLabel->setText(
        QString("Total tiles: %1 | Ocean skipped: %2 | Existing: %3 | To generate: %4 | Estimated time: %5 | Estimated space: %6")
        .arg(allTiles.size())
        .arg(skippedOcean)
        .arg(alreadyDone)
        .arg(tilesToGenerate)
        .arg(formatTimeSeconds(estimatedSeconds))
        .arg(formatSizeGb(estimatedGb))
    );

    log("Estimate updated.");
}

QString MainWindow::buildOutputPath(const Tile& tile) const
{
    QDir doneDir(doneFolderEdit->text());
    return doneDir.filePath(tile.name);
}

QStringList MainWindow::buildArnisArgs(const Tile& tile, const QString& outputPath) const
{
    QString bbox = QString("%1,%2,%3,%4")
        .arg(tile.south, 0, 'f', 8)
        .arg(tile.west, 0, 'f', 8)
        .arg(tile.north, 0, 'f', 8)
        .arg(tile.east, 0, 'f', 8);

    QStringList args;
    args << "--bbox" << bbox;
    args << "--output-dir" << outputPath;
    args << "--scale" << QString::number(scaleSpin->value());
    args << "--timeout" << QString::number(timeoutSpin->value());

    if (terrainCheck->isChecked())
        args << "--terrain";

    args << "--interior" << (interiorCheck->isChecked() ? "true" : "false");
    args << "--roof" << (roofCheck->isChecked() ? "true" : "false");

    return args;
}

void MainWindow::startGeneration()
{
    estimateTiles();

    if (runnableTiles.isEmpty()) {
        log("No runnable tiles.");
        return;
    }

    QDir().mkpath(workFolderEdit->text());
    QDir().mkpath(doneFolderEdit->text());

    nextTileIndex = 0;
    completed = 0;
    failed = 0;
    skippedExisting = 0;
    stopRequested = false;
    activeProcesses.clear();

    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    progressBar->setValue(0);

    log(QString("Starting generation. Runnable tiles: %1").arg(runnableTiles.size()));
    launchMoreTiles();
}

void MainWindow::stopGeneration()
{
    stopRequested = true;
    log("Stop requested. Running Arnis processes will be killed.");

    QList<QProcess*> processes = activeProcesses.keys();

    for (QProcess* process : processes) {
        if (process && process->state() != QProcess::NotRunning) {
            process->kill();
        }
    }
}

void MainWindow::launchMoreTiles()
{
    if (stopRequested) {
        if (activeProcesses.isEmpty()) {
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            statusLabel->setText("Status: stopped");
            log("Stopped.");
        }
        return;
    }

    while (activeProcesses.size() < workersSpin->value() && nextTileIndex < runnableTiles.size()) {
        Tile tile = runnableTiles[nextTileIndex];
        nextTileIndex++;

        QString outputPath = buildOutputPath(tile);

        if (QDir(outputPath).exists()) {
            skippedExisting++;
            writeProgressCsv(tile, "skipped_existing", "Output folder already exists.");
            log(QString("Skipping %1 because folder already exists.").arg(tile.name));
            updateProgress();
            continue;
        }

        QProcess* process = new QProcess(this);
        activeProcesses.insert(process, tile);

        connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
            readProcessOutput(process);
            });

        connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
            readProcessOutput(process);
            });

        connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                handleProcessFinished(process, exitCode, exitStatus);
            });

        QStringList args = buildArnisArgs(tile, outputPath);

        log(QString("[%1/%2] Generating %3 (%4% ocean)")
            .arg(nextTileIndex)
            .arg(runnableTiles.size())
            .arg(tile.name)
            .arg(tile.oceanPercent * 100.0, 0, 'f', 1));

        log("Command: " + arnisPathEdit->text() + " " + args.join(" "));

        process->start(arnisPathEdit->text(), args);

        if (!process->waitForStarted(5000)) {
            activeProcesses.remove(process);
            failed++;
            writeProgressCsv(tile, "failed", "Could not start Arnis process.");
            log(QString("Failed to start Arnis for tile %1").arg(tile.name));
            process->deleteLater();
            updateProgress();
        }
    }

    updateProgress();

    if (activeProcesses.isEmpty() && nextTileIndex >= runnableTiles.size()) {
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        statusLabel->setText("Status: finished");
        progressBar->setValue(100);

        log(QString("Finished. Done: %1 | Existing skipped: %2 | Ocean skipped: %3 | Failed: %4")
            .arg(completed)
            .arg(skippedExisting)
            .arg(skippedOcean)
            .arg(failed));
    }
}

void MainWindow::handleProcessFinished(QProcess* process, int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!activeProcesses.contains(process)) {
        process->deleteLater();
        return;
    }

    Tile tile = activeProcesses.value(process);
    activeProcesses.remove(process);

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        completed++;
        writeProgressCsv(tile, "done", "Generated successfully.");
        log(QString("Done: %1").arg(tile.name));
    }
    else {
        failed++;
        writeProgressCsv(tile, "failed", QString("Exit code: %1").arg(exitCode));
        log(QString("Failed: %1 | Exit code: %2").arg(tile.name).arg(exitCode));
    }

    process->deleteLater();

    updateProgress();
    launchMoreTiles();
}

void MainWindow::readProcessOutput(QProcess* process)
{
    if (!process)
        return;

    QString output = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
    QString error = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();

    if (!output.isEmpty())
        log(output);

    if (!error.isEmpty())
        log(error);
}

double MainWindow::oceanPercentForTile(const Tile& tile, const LandMask& landMask) const
{
    int grid = oceanSampleGridSpin->value();

    int waterHits = 0;
    int total = 0;

    for (int row = 0; row < grid; row++) {
        for (int col = 0; col < grid; col++) {
            double lat = tile.south + ((row + 0.5) / double(grid)) * (tile.north - tile.south);
            double lon = tile.west + ((col + 0.5) / double(grid)) * (tile.east - tile.west);

            if (!landMask.isLand(lat, lon))
                waterHits++;

            total++;
        }
    }

    if (total == 0)
        return 0.0;

    return double(waterHits) / double(total);
}

void MainWindow::buildOceanCache()
{
    QVector<Tile> tiles = createTiles();

    if (tiles.isEmpty()) {
        QMessageBox::warning(this, "No tiles", "There are no tiles to check.");
        return;
    }

    if (landMaskEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing land file", "Select a land GeoJSON file first.");
        return;
    }

    QString error;
    LandMask landMask;

    log("Loading land GeoJSON...");

    if (!landMask.loadGeoJson(landMaskEdit->text(), &error)) {
        QMessageBox::warning(this, "Land mask failed", error);
        log("Land mask failed: " + error);
        return;
    }

    QFile file(oceanCacheEdit->text());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Write failed", "Could not write ocean cache CSV.");
        return;
    }

    QTextStream out(&file);
    out << "tile,ocean_percent,skip\n";

    stopRequested = false;
    stopButton->setEnabled(true);
    startButton->setEnabled(false);
    buildOceanCacheButton->setEnabled(false);

    int skipped = 0;
    double threshold = oceanThresholdSpin->value();

    log(QString("Building ocean cache for %1 tiles...").arg(tiles.size()));

    for (int i = 0; i < tiles.size(); i++) {
        if (stopRequested) {
            log("Ocean cache build stopped.");
            break;
        }

        Tile tile = tiles[i];

        double oceanPercent = oceanPercentForTile(tile, landMask);
        bool shouldSkip = oceanPercent >= threshold;

        if (shouldSkip)
            skipped++;

        out << tile.name << ","
            << QString::number(oceanPercent, 'f', 6) << ","
            << (shouldSkip ? "true" : "false") << "\n";

        int percent = int((double(i + 1) / double(tiles.size())) * 100.0);
        progressBar->setValue(percent);

        statusLabel->setText(
            QString("Building ocean cache: %1/%2 | Ocean skipped: %3")
            .arg(i + 1)
            .arg(tiles.size())
            .arg(skipped)
        );

        if ((i + 1) % 100 == 0) {
            log(QString("Ocean checked: %1/%2").arg(i + 1).arg(tiles.size()));
        }

        QApplication::processEvents();
    }

    file.close();

    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    buildOceanCacheButton->setEnabled(true);

    log(QString("Ocean cache finished. Skipped: %1").arg(skipped));

    estimateTiles();
}

void MainWindow::updateProgress()
{
    int finished = completed + failed + skippedExisting;
    int total = runnableTiles.size();

    int percent = total > 0 ? int((double(finished) / double(total)) * 100.0) : 100;
    progressBar->setValue(percent);

    statusLabel->setText(
        QString("Status: %1/%2 | Done: %3 | Existing skipped: %4 | Ocean skipped: %5 | Failed: %6 | Active: %7")
        .arg(finished)
        .arg(total)
        .arg(completed)
        .arg(skippedExisting)
        .arg(skippedOcean)
        .arg(failed)
        .arg(activeProcesses.size())
    );
}

void MainWindow::writeProgressCsv(const Tile& tile, const QString& status, const QString& message)
{
    QString path = QDir(doneFolderEdit->text()).filePath("progress.csv");

    QFile file(path);
    bool exists = file.exists();

    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);

    if (!exists) {
        out << "tile,status,message,west,south,east,north,time\n";
    }

    QString safeMessage = message;
    safeMessage.replace("\"", "\"\"");

    out << tile.name << ","
        << status << ","
        << "\"" << safeMessage << "\"" << ","
        << tile.west << ","
        << tile.south << ","
        << tile.east << ","
        << tile.north << ","
        << QDateTime::currentDateTime().toString(Qt::ISODate)
        << "\n";
}

QString MainWindow::formatTimeSeconds(double seconds) const
{
    int total = int(seconds);
    int hours = total / 3600;
    int minutes = (total % 3600) / 60;

    return QString("%1h %2m").arg(hours).arg(minutes);
}

QString MainWindow::formatSizeGb(double gb) const
{
    if (gb >= 1024.0)
        return QString("%1 TB").arg(gb / 1024.0, 0, 'f', 2);

    return QString("%1 GB").arg(gb, 0, 'f', 2);
}

QString MainWindow::appSettingsPath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("settings.json");
}

void MainWindow::saveSettings()
{
    QJsonObject obj;

    obj["arnisPath"] = arnisPathEdit->text();
    obj["workFolder"] = workFolderEdit->text();
    obj["doneFolder"] = doneFolderEdit->text();
    obj["oceanCache"] = oceanCacheEdit->text();

    obj["west"] = westSpin->value();
    obj["east"] = eastSpin->value();
    obj["south"] = southSpin->value();
    obj["north"] = northSpin->value();

    obj["stepLon"] = stepLonSpin->value();
    obj["stepLat"] = stepLatSpin->value();
    obj["scale"] = scaleSpin->value();

    obj["workers"] = workersSpin->value();
    obj["timeout"] = timeoutSpin->value();
    obj["avgTileTime"] = avgTileTimeSpin->value();

    obj["terrain"] = terrainCheck->isChecked();
    obj["interior"] = interiorCheck->isChecked();
    obj["roof"] = roofCheck->isChecked();

    obj["useOceanCache"] = useOceanCacheCheck->isChecked();
    obj["oceanThreshold"] = oceanThresholdSpin->value();

    obj["landMask"] = landMaskEdit->text();
    obj["oceanSampleGrid"] = oceanSampleGridSpin->value();

    QFile file(appSettingsPath());
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Save failed", "Could not write settings.json");
        return;
    }

    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    log("Settings saved.");
}

void MainWindow::loadSettings()
{
    QFile file(appSettingsPath());

    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        log("Could not open settings.json");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject()) {
        log("settings.json is invalid.");
        return;
    }

    QJsonObject obj = doc.object();

    arnisPathEdit->setText(obj.value("arnisPath").toString(arnisPathEdit->text()));
    workFolderEdit->setText(obj.value("workFolder").toString(workFolderEdit->text()));
    doneFolderEdit->setText(obj.value("doneFolder").toString(doneFolderEdit->text()));
    oceanCacheEdit->setText(obj.value("oceanCache").toString(oceanCacheEdit->text()));

    westSpin->setValue(obj.value("west").toDouble(westSpin->value()));
    eastSpin->setValue(obj.value("east").toDouble(eastSpin->value()));
    southSpin->setValue(obj.value("south").toDouble(southSpin->value()));
    northSpin->setValue(obj.value("north").toDouble(northSpin->value()));

    stepLonSpin->setValue(obj.value("stepLon").toDouble(stepLonSpin->value()));
    stepLatSpin->setValue(obj.value("stepLat").toDouble(stepLatSpin->value()));
    scaleSpin->setValue(obj.value("scale").toDouble(scaleSpin->value()));

    workersSpin->setValue(obj.value("workers").toInt(workersSpin->value()));
    timeoutSpin->setValue(obj.value("timeout").toInt(timeoutSpin->value()));
    avgTileTimeSpin->setValue(obj.value("avgTileTime").toInt(avgTileTimeSpin->value()));

    terrainCheck->setChecked(obj.value("terrain").toBool(terrainCheck->isChecked()));
    interiorCheck->setChecked(obj.value("interior").toBool(interiorCheck->isChecked()));
    roofCheck->setChecked(obj.value("roof").toBool(roofCheck->isChecked()));

    useOceanCacheCheck->setChecked(obj.value("useOceanCache").toBool(useOceanCacheCheck->isChecked()));
    oceanThresholdSpin->setValue(obj.value("oceanThreshold").toDouble(oceanThresholdSpin->value()));

    landMaskEdit->setText(obj.value("landMask").toString(landMaskEdit->text()));
    oceanSampleGridSpin->setValue(obj.value("oceanSampleGrid").toInt(oceanSampleGridSpin->value()));

    log("Settings loaded.");
}

void MainWindow::log(const QString& message)
{
    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    logBox->appendPlainText(QString("[%1] %2").arg(time, message));
}