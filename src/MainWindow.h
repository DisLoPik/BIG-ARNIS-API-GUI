#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QLabel>

#include "LandMask.h"

struct Tile
{
    QString name;
    double west = 0;
    double east = 0;
    double south = 0;
    double north = 0;
    double oceanPercent = 0.0;
    bool oceanSkipped = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    // UI
    QLineEdit* arnisPathEdit;
    QLineEdit* workFolderEdit;
    QLineEdit* doneFolderEdit;
    QLineEdit* oceanCacheEdit;
    QLineEdit* landMaskEdit;

    QDoubleSpinBox* westSpin;
    QDoubleSpinBox* eastSpin;
    QDoubleSpinBox* southSpin;
    QDoubleSpinBox* northSpin;

    QDoubleSpinBox* stepLonSpin;
    QDoubleSpinBox* stepLatSpin;
    QDoubleSpinBox* scaleSpin;
    QDoubleSpinBox* oceanThresholdSpin;

    QSpinBox* workersSpin;
    QSpinBox* timeoutSpin;
    QSpinBox* avgTileTimeSpin;
    QSpinBox* oceanSampleGridSpin;

    QCheckBox* terrainCheck;
    QCheckBox* interiorCheck;
    QCheckBox* roofCheck;
    QCheckBox* useOceanCacheCheck;

    QLabel* estimateLabel;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    QPlainTextEdit* logBox;

    QPushButton* estimateButton;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* saveSettingsButton;
    QPushButton* loadSettingsButton;
    QPushButton* buildOceanCacheButton;

    // State
    QVector<Tile> allTiles;
    QVector<Tile> runnableTiles;
    QHash<QProcess*, Tile> activeProcesses;

    int nextTileIndex = 0;
    int completed = 0;
    int failed = 0;
    int skippedExisting = 0;
    int skippedOcean = 0;
    bool stopRequested = false;

private slots:
    void estimateTiles();
    void startGeneration();
    void stopGeneration();
    void launchMoreTiles();
    void saveSettings();
    void loadSettings();
    void buildOceanCache();

private:
    void buildUi();

    QVector<Tile> createTiles() const;
    QVector<Tile> applyOceanCacheSkipping(const QVector<Tile>& tiles);

    QString buildOutputPath(const Tile& tile) const;
    QStringList buildArnisArgs(const Tile& tile, const QString& outputPath) const;

    void handleProcessFinished(QProcess* process, int exitCode, QProcess::ExitStatus exitStatus);
    void readProcessOutput(QProcess* process);

    void updateProgress();
    void writeProgressCsv(const Tile& tile, const QString& status, const QString& message = "");
    QSet<QString> loadDoneTilesFromFolders() const;
    QHash<QString, double> loadOceanCache() const;

    double oceanPercentForTile(const Tile& tile, const LandMask& landMask) const;

    QString formatTimeSeconds(double seconds) const;
    QString formatSizeGb(double gb) const;

    QString appSettingsPath() const;
    void log(const QString& message);
};