#include "MainWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QStringConverter>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupSignals();
    applyTheme(QStringLiteral("Aurora Triad"));
    updateStatus(QStringLiteral("已就绪：请先选择项目，再执行导入或管理。"));
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("StarBOM - BOM 半自动整理工具"));

    const QScreen *screen = QGuiApplication::primaryScreen();
    const QRect available = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
    resize(qRound(available.width() * 0.9), qRound(available.height() * 0.9));
    setMinimumSize(1280, 820);

    m_centralWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    auto *splitter = new QSplitter(Qt::Horizontal, m_centralWidget);
    splitter->setChildrenCollapsible(false);

    auto *leftScroll = new QScrollArea(splitter);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);

    auto *leftPanel = new QFrame();
    leftPanel->setObjectName(QStringLiteral("leftPanel"));
    leftPanel->setFixedWidth(340);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(14, 14, 14, 14);
    leftLayout->setSpacing(14);
    leftScroll->setWidget(leftPanel);

    auto *rightPanel = new QFrame(splitter);
    rightPanel->setObjectName(QStringLiteral("rightPanel"));
    rightPanel->setMinimumWidth(760);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(12, 10, 12, 10);
    rightLayout->setSpacing(8);

    splitter->addWidget(leftScroll);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({340, qMax(900, width() - 340)});

    mainLayout->addWidget(splitter);
    setCentralWidget(m_centralWidget);

    setupLeftPanel();
    setupRightPanel();
}

void MainWindow::setupLeftPanel()
{
    auto *leftPanel = findChild<QFrame *>(QStringLiteral("leftPanel"));
    auto *layout = qobject_cast<QVBoxLayout *>(leftPanel->layout());

    auto *brandFrame = new QFrame(leftPanel);
    auto *brandLayout = new QGridLayout(brandFrame);
    brandLayout->setContentsMargins(10, 8, 10, 8);
    brandLayout->setHorizontalSpacing(8);
    brandLayout->setVerticalSpacing(2);

    auto *titleLabel = new QLabel(QStringLiteral("StarBOM"), brandFrame);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));

    auto *githubLink = new QLabel(leftPanel);
    githubLink->setText(QStringLiteral("<span style='font-size:12px'>🐙</span> <a href='https://github.com/890mn/StarBOM'>890mn</a>"));
    githubLink->setTextFormat(Qt::RichText);
    githubLink->setOpenExternalLinks(true);
    githubLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    githubLink->setObjectName(QStringLiteral("githubLabel"));

    brandLayout->addWidget(titleLabel, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    brandLayout->addWidget(githubLink, 0, 1, 1, 1, Qt::AlignRight | Qt::AlignVCenter);

    auto *metaFrame = new QFrame(leftPanel);
    auto *metaLayout = new QHBoxLayout(metaFrame);
    metaLayout->setContentsMargins(6, 0, 6, 0);
    metaLayout->setSpacing(6);
    auto *themeLabel = new QLabel(QStringLiteral("主题："), metaFrame);
    m_themeToggleBtn = new QPushButton(metaFrame);
    m_themeToggleBtn->setObjectName(QStringLiteral("themeToggleBtn"));
    m_themeToggleBtn->setFlat(true);
    m_themeToggleBtn->setCursor(Qt::PointingHandCursor);
    auto *versionLabel = new QLabel(QStringLiteral("v0.3.0"), metaFrame);
    versionLabel->setObjectName(QStringLiteral("versionLabel"));
    metaLayout->addWidget(themeLabel);
    metaLayout->addWidget(m_themeToggleBtn, 0, Qt::AlignLeft);
    metaLayout->addStretch();
    metaLayout->addWidget(versionLabel, 0, Qt::AlignRight);

    auto *importGroup = new QGroupBox(QStringLiteral("导入"), leftPanel);
    auto *importLayout = new QVBoxLayout(importGroup);
    importLayout->setSpacing(10);

    auto *quickImportBtn = new QPushButton(QStringLiteral("立创导入（XLS）"), importGroup);
    auto *xlsImportBtn = new QPushButton(QStringLiteral("从 XLS/XLSX 导入"), importGroup);
    auto *ocrImportBtn = new QPushButton(QStringLiteral("OCR 图片导入（后续）"), importGroup);
    quickImportBtn->setMinimumHeight(40);
    xlsImportBtn->setMinimumHeight(42);
    ocrImportBtn->setMinimumHeight(40);

    importLayout->addWidget(quickImportBtn);
    importLayout->addWidget(xlsImportBtn);
    importLayout->addWidget(ocrImportBtn);

    auto *exportGroup = new QGroupBox(QStringLiteral("导出"), leftPanel);
    auto *exportLayout = new QVBoxLayout(exportGroup);
    auto *csvExportBtn = new QPushButton(QStringLiteral("导出 CSV"), exportGroup);
    csvExportBtn->setMinimumHeight(42);
    exportLayout->addWidget(csvExportBtn);

    auto *projectGroup = new QGroupBox(QStringLiteral("项目"), leftPanel);
    auto *projectLayout = new QVBoxLayout(projectGroup);
    projectLayout->setSpacing(10);
    m_projectList = new QListWidget(projectGroup);
    m_projectList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_projectList->setMinimumHeight(220);
    m_projectList->addItems(
        {QStringLiteral("全部项目"), QStringLiteral("电源管理板 RevA"), QStringLiteral("传感器节点 V2"), QStringLiteral("验证样机 Proto-3")});
    m_projectList->setCurrentRow(0);

    auto *projectBtnLayout = new QHBoxLayout();
    auto *newProjectBtn = new QPushButton(QStringLiteral("新建"), projectGroup);
    auto *renameProjectBtn = new QPushButton(QStringLiteral("重命名"), projectGroup);
    auto *clearProjectBtn = new QPushButton(QStringLiteral("取消选中"), projectGroup);
    newProjectBtn->setMinimumHeight(38);
    renameProjectBtn->setMinimumHeight(38);
    clearProjectBtn->setMinimumHeight(38);

    projectBtnLayout->addWidget(newProjectBtn);
    projectBtnLayout->addWidget(renameProjectBtn);
    projectBtnLayout->addWidget(clearProjectBtn);

    projectLayout->addWidget(m_projectList);
    projectLayout->addLayout(projectBtnLayout);

    auto *categoryGroup = new QGroupBox(QStringLiteral("分类组"), leftPanel);
    auto *categoryLayout = new QVBoxLayout(categoryGroup);
    categoryLayout->setSpacing(10);
    m_categoryList = new QListWidget(categoryGroup);
    m_categoryList->setMinimumHeight(180);
    m_categoryList->addItems({QStringLiteral("电阻电容"), QStringLiteral("芯片 IC"), QStringLiteral("连接器"), QStringLiteral("机械件")});

    auto *categoryBtnLayout = new QHBoxLayout();
    auto *newCategoryBtn = new QPushButton(QStringLiteral("新增"), categoryGroup);
    auto *editCategoryBtn = new QPushButton(QStringLiteral("修改"), categoryGroup);
    newCategoryBtn->setMinimumHeight(38);
    editCategoryBtn->setMinimumHeight(38);
    categoryBtnLayout->addWidget(newCategoryBtn);
    categoryBtnLayout->addWidget(editCategoryBtn);

    categoryLayout->addWidget(m_categoryList);
    categoryLayout->addLayout(categoryBtnLayout);

    layout->addWidget(brandFrame);
    layout->addWidget(metaFrame);
    layout->addWidget(importGroup);
    layout->addWidget(exportGroup);
    layout->addWidget(projectGroup, 2);
    layout->addWidget(categoryGroup, 2);
    layout->addStretch();

    m_themeSelector = new QComboBox(this);
    m_themeSelector->addItems({QStringLiteral("Aurora Triad"), QStringLiteral("Citrus Triad"), QStringLiteral("Slate Triad")});
    m_themeToggleBtn->setText(m_themeSelector->currentText());

    connect(m_themeToggleBtn, &QPushButton::clicked, this, [this] {
        if (!m_themeSelector || m_themeSelector->count() == 0) {
            return;
        }
        const int next = (m_themeSelector->currentIndex() + 1) % m_themeSelector->count();
        m_themeSelector->setCurrentIndex(next);
    });

    connect(quickImportBtn, &QPushButton::clicked, this, [this] {
        importLichuangSpreadsheetFlow();
    });

    connect(xlsImportBtn, &QPushButton::clicked, this, [this] {
        importLichuangSpreadsheetFlow();
    });

    connect(ocrImportBtn, &QPushButton::clicked, this, [this] {
        updateStatus(QStringLiteral("OCR 导入：目标项目 %1（识别流程待接入）。").arg(currentProjectText()));
    });

    connect(csvExportBtn, &QPushButton::clicked, this, [this] {
        updateStatus(QStringLiteral("CSV 导出任务已触发：范围 %1").arg(currentProjectText()));
    });

    connect(newProjectBtn, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString name = QInputDialog::getText(this,
                                                   QStringLiteral("新建项目"),
                                                   QStringLiteral("项目名称："),
                                                   QLineEdit::Normal,
                                                   {},
                                                   &ok)
                                 .trimmed();
        if (ok && !name.isEmpty()) {
            m_projectList->addItem(name);
            m_projectList->setCurrentRow(m_projectList->count() - 1);
            updateStatus(QStringLiteral("已新增并选中项目：%1").arg(name));
        }
    });

    connect(renameProjectBtn, &QPushButton::clicked, this, [this] {
        auto *item = m_projectList->currentItem();
        if (!item || item->text() == QStringLiteral("全部项目")) {
            updateStatus(QStringLiteral("请先选择一个具体项目再重命名。"));
            return;
        }

        bool ok = false;
        const QString name = QInputDialog::getText(this,
                                                   QStringLiteral("重命名项目"),
                                                   QStringLiteral("新名称："),
                                                   QLineEdit::Normal,
                                                   item->text(),
                                                   &ok)
                                 .trimmed();
        if (ok && !name.isEmpty()) {
            item->setText(name);
            updateStatus(QStringLiteral("项目已重命名为：%1").arg(name));
        }
    });

    connect(clearProjectBtn, &QPushButton::clicked, this, [this] {
        m_projectList->clearSelection();
        updateStatus(QStringLiteral("项目未选中，当前按“全部项目”显示。"));
    });

    connect(m_projectList, &QListWidget::itemSelectionChanged, this, [this] {
        updateStatus(QStringLiteral("当前项目范围：%1").arg(currentProjectText()));
    });

    connect(newCategoryBtn, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString name = QInputDialog::getText(this,
                                                   QStringLiteral("新增分类组"),
                                                   QStringLiteral("分类名称："),
                                                   QLineEdit::Normal,
                                                   {},
                                                   &ok)
                                 .trimmed();
        if (ok && !name.isEmpty()) {
            m_categoryList->addItem(name);
            updateStatus(QStringLiteral("已新增分类组：%1").arg(name));
        }
    });

    connect(editCategoryBtn, &QPushButton::clicked, this, [this] {
        auto *item = m_categoryList->currentItem();
        if (!item) {
            updateStatus(QStringLiteral("请先选择要修改的分类组。"));
            return;
        }

        bool ok = false;
        const QString name = QInputDialog::getText(this,
                                                   QStringLiteral("修改分类组"),
                                                   QStringLiteral("新分类名称："),
                                                   QLineEdit::Normal,
                                                   item->text(),
                                                   &ok)
                                 .trimmed();
        if (ok && !name.isEmpty()) {
            item->setText(name);
            updateStatus(QStringLiteral("分类组已修改为：%1").arg(name));
        }
    });
}

void MainWindow::setupRightPanel()
{
    auto *rightPanel = findChild<QFrame *>(QStringLiteral("rightPanel"));
    auto *layout = qobject_cast<QVBoxLayout *>(rightPanel->layout());

    auto *tabStrip = new QWidget(rightPanel);
    tabStrip->setObjectName(QStringLiteral("tabStrip"));
    auto *tabLayout = new QHBoxLayout(tabStrip);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(8);

    auto *bomViewBtn = new QPushButton(QStringLiteral("BOM 视图"), tabStrip);
    auto *inventoryBtn = new QPushButton(QStringLiteral("库存视图"), tabStrip);
    auto *analyzeBtn = new QPushButton(QStringLiteral("差异分析"), tabStrip);

    bomViewBtn->setCheckable(true);
    inventoryBtn->setCheckable(true);
    analyzeBtn->setCheckable(true);
    bomViewBtn->setChecked(true);

    m_searchInput = new QLineEdit(tabStrip);
    m_searchInput->setMinimumHeight(38);
    m_searchInput->setPlaceholderText(QStringLiteral("全文搜索（料号 / 位号 / 规格 / 备注）"));
    m_clearSearchBtn = new QPushButton(QStringLiteral("清空"), tabStrip);
    m_clearSearchBtn->setMinimumHeight(38);

    tabLayout->addWidget(bomViewBtn);
    tabLayout->addWidget(inventoryBtn);
    tabLayout->addWidget(analyzeBtn);
    tabLayout->addStretch();
    tabLayout->addWidget(m_searchInput, 2);
    tabLayout->addWidget(m_clearSearchBtn);

    m_viewStack = new QStackedWidget(rightPanel);

    auto *bomPage = new QWidget(m_viewStack);
    auto *bomLayout = new QVBoxLayout(bomPage);

    auto *columnConfigRow = new QWidget(bomPage);
    auto *columnConfigLayout = new QHBoxLayout(columnConfigRow);
    columnConfigLayout->setContentsMargins(0, 0, 0, 0);
    columnConfigLayout->setSpacing(6);
    columnConfigLayout->addWidget(new QLabel(QStringLiteral("显示列："), columnConfigRow));
    for (int i = 0; i < 6; ++i) {
        auto *selector = new QComboBox(columnConfigRow);
        selector->setMinimumWidth(130);
        m_bomColumnSelectors.append(selector);
        columnConfigLayout->addWidget(selector);
        connect(selector, &QComboBox::currentIndexChanged, this, [this](int) {
            applyBomColumnSelection();
        });
    }
    columnConfigLayout->addStretch();

    m_bomTable = new QTableWidget(6, 8, bomPage);
    m_bomTable->setHorizontalHeaderLabels({QStringLiteral("项目"),
                                           QStringLiteral("位号"),
                                           QStringLiteral("分类"),
                                           QStringLiteral("料号"),
                                           QStringLiteral("规格"),
                                           QStringLiteral("数量"),
                                           QStringLiteral("供应商"),
                                           QStringLiteral("备注")});
    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_bomTable->horizontalHeader()->setSectionsClickable(true);
    m_bomTable->setSortingEnabled(true);
    m_bomTable->verticalHeader()->setVisible(false);

    const QList<QStringList> bomRows {
        {QStringLiteral("电源管理板 RevA"), QStringLiteral("R1-R8"), QStringLiteral("电阻电容"), QStringLiteral("RC0603-10K"), QStringLiteral("10K 1% 0603"), QStringLiteral("8"), QStringLiteral("LCSC"), QStringLiteral("常规库存")},
        {QStringLiteral("电源管理板 RevA"), QStringLiteral("C1-C4"), QStringLiteral("电阻电容"), QStringLiteral("CC0603-100N"), QStringLiteral("100nF 16V X7R"), QStringLiteral("4"), QStringLiteral("LCSC"), QStringLiteral("去耦")},
        {QStringLiteral("传感器节点 V2"), QStringLiteral("U1"), QStringLiteral("芯片 IC"), QStringLiteral("STM32G0"), QStringLiteral("QFN32"), QStringLiteral("1"), QStringLiteral("LCSC"), QStringLiteral("主控")},
        {QStringLiteral("验证样机 Proto-3"), QStringLiteral("J1"), QStringLiteral("连接器"), QStringLiteral("CONN-USB-C"), QStringLiteral("16P 立贴"), QStringLiteral("1"), QStringLiteral("LCSC"), QStringLiteral("调试接口")}
    };

    for (int i = 0; i < bomRows.size(); ++i) {
        for (int j = 0; j < bomRows[i].size(); ++j) {
            m_bomTable->setItem(i, j, new QTableWidgetItem(bomRows[i][j]));
        }
    }
    bomLayout->addWidget(columnConfigRow);
    bomLayout->addWidget(m_bomTable);
    captureBomSourceFromCurrentTable();
    refreshBomColumnSelectors();
    applyBomColumnSelection();

    auto *inventoryPage = new QWidget(m_viewStack);
    auto *inventoryLayout = new QVBoxLayout(inventoryPage);
    m_inventoryTable = new QTableWidget(5, 6, inventoryPage);
    m_inventoryTable->setHorizontalHeaderLabels({QStringLiteral("项目"), QStringLiteral("料号"), QStringLiteral("现存"), QStringLiteral("需求"), QStringLiteral("缺口"), QStringLiteral("建议")});
    m_inventoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_inventoryTable->verticalHeader()->setVisible(false);

    const QList<QStringList> invRows {
        {QStringLiteral("电源管理板 RevA"), QStringLiteral("RC0603-10K"), QStringLiteral("1200"), QStringLiteral("800"), QStringLiteral("0"), QStringLiteral("可用")},
        {QStringLiteral("传感器节点 V2"), QStringLiteral("STM32G0"), QStringLiteral("20"), QStringLiteral("30"), QStringLiteral("10"), QStringLiteral("建议补货")},
        {QStringLiteral("验证样机 Proto-3"), QStringLiteral("CONN-USB-C"), QStringLiteral("5"), QStringLiteral("10"), QStringLiteral("5"), QStringLiteral("可替代料")}
    };

    for (int i = 0; i < invRows.size(); ++i) {
        for (int j = 0; j < invRows[i].size(); ++j) {
            m_inventoryTable->setItem(i, j, new QTableWidgetItem(invRows[i][j]));
        }
    }
    inventoryLayout->addWidget(m_inventoryTable);

    auto *analysisPage = new QWidget(m_viewStack);
    auto *analysisLayout = new QVBoxLayout(analysisPage);
    auto *analysisHint = new QLabel(QStringLiteral("差异分析视图：后续接入版本对比、替代料推荐、成本变化趋势。\n（搜索框已保留，后续可扩展到分析结果项高亮）"), analysisPage);
    analysisHint->setWordWrap(true);
    analysisLayout->addWidget(analysisHint);
    analysisLayout->addStretch();

    m_viewStack->addWidget(bomPage);
    m_viewStack->addWidget(inventoryPage);
    m_viewStack->addWidget(analysisPage);

    auto *statusBarBox = new QFrame(rightPanel);
    auto *statusLayout = new QHBoxLayout(statusBarBox);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLabel = new QLabel(statusBarBox);
    statusLayout->addWidget(m_statusLabel);

    layout->addWidget(tabStrip, 1);
    layout->addWidget(m_viewStack, 9);
    layout->addWidget(statusBarBox);

    auto setActive = [bomViewBtn, inventoryBtn, analyzeBtn](QPushButton *active) {
        bomViewBtn->setChecked(active == bomViewBtn);
        inventoryBtn->setChecked(active == inventoryBtn);
        analyzeBtn->setChecked(active == analyzeBtn);
    };

    connect(bomViewBtn, &QPushButton::clicked, this, [this, setActive, bomViewBtn] {
        setActive(bomViewBtn);
        m_viewStack->setCurrentIndex(0);
        applySearchHighlight(m_searchInput->text().trimmed());
        updateStatus(QStringLiteral("已切换到 BOM 视图。"));
    });

    connect(inventoryBtn, &QPushButton::clicked, this, [this, setActive, inventoryBtn] {
        setActive(inventoryBtn);
        m_viewStack->setCurrentIndex(1);
        applySearchHighlight(m_searchInput->text().trimmed());
        updateStatus(QStringLiteral("已切换到库存视图。"));
    });

    connect(analyzeBtn, &QPushButton::clicked, this, [this, setActive, analyzeBtn] {
        setActive(analyzeBtn);
        m_viewStack->setCurrentIndex(2);
        updateStatus(QStringLiteral("已切换到差异分析视图。"));
    });

    connect(m_searchInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        applySearchHighlight(text.trimmed());
    });

    connect(m_clearSearchBtn, &QPushButton::clicked, this, [this] {
        m_searchInput->clear();
        applySearchHighlight({});
        updateStatus(QStringLiteral("搜索关键字已清空。"));
    });
}

void MainWindow::setupSignals()
{
    connect(m_themeSelector, &QComboBox::currentTextChanged, this, [this](const QString &name) {
        applyTheme(name);
        if (m_themeToggleBtn) {
            m_themeToggleBtn->setText(name);
        }
        updateStatus(QStringLiteral("主题已切换：%1").arg(name));
    });
}

void MainWindow::applyTheme(const QString &themeName)
{
    QString primary = QStringLiteral("#2E5BFF");
    QString secondary = QStringLiteral("#FF7A00");
    QString accent = QStringLiteral("#00B8A9");
    QString panel = QStringLiteral("#F5F7FB");
    QString text = QStringLiteral("#1F2937");

    if (themeName == QStringLiteral("Citrus Triad")) {
        primary = QStringLiteral("#306B34");
        secondary = QStringLiteral("#FF7F11");
        accent = QStringLiteral("#4D2D52");
        panel = QStringLiteral("#F8F6F0");
    } else if (themeName == QStringLiteral("Slate Triad")) {
        primary = QStringLiteral("#384E77");
        secondary = QStringLiteral("#D66A6A");
        accent = QStringLiteral("#6AB187");
        panel = QStringLiteral("#F3F5F7");
    }

    const QString style = QString(R"(
        QMainWindow, QWidget {
            background: %1;
            color: %2;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 14px;
        }
        QFrame#leftPanel, QFrame#rightPanel {
            background: white;
            border: 1px solid #D8E0EA;
            border-radius: 10px;
        }
        QLabel#titleLabel {
            font-size: 36px;
            font-weight: 800;
            color: %3;
            letter-spacing: 0.5px;
        }
        QLabel#versionLabel {
            color: #6B7280;
            font-size: 12px;
            font-weight: 500;
        }
        QLabel#githubLabel {
            color: #334155;
            font-size: 14px;
            margin-left: 4px;
        }
        QLabel#githubLabel a {
            color: %3;
            text-decoration: none;
            font-weight: 700;
        }
        QGroupBox {
            border: 1px solid #D8E0EA;
            border-radius: 10px;
            margin-top: 12px;
            font-weight: 700;
            padding-top: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 3px;
            color: %3;
        }
        QPushButton {
            background: %3;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 12px;
            font-weight: 600;
        }
        QPushButton:hover { background: %4; }
        QPushButton:checked { background: %5; }
        QPushButton#themeToggleBtn {
            background: transparent;
            color: %3;
            padding: 0;
            border: none;
            text-decoration: underline;
            font-weight: 700;
        }
        QPushButton#themeToggleBtn:hover {
            background: transparent;
            color: %4;
        }
        QLineEdit, QListWidget, QTableWidget, QComboBox {
            border: 1px solid #CCD6E2;
            border-radius: 8px;
            background: white;
            padding: 6px;
        }
        QListWidget::item {
            border-left: 2px solid transparent;
            padding-left: 8px;
        }
        QListWidget::item:selected {
            background: rgba(%6, 0.18);
            border-radius: 4px;
            color: #0F172A;
            font-weight: 700;
            border-left: 5px solid %3;
        }
        QHeaderView::section {
            background: %3;
            color: white;
            padding: 6px;
            border: none;
        }
        QWidget#tabStrip {
            background: transparent;
        }
    )")
                              .arg(panel, text, primary, secondary, accent, QStringLiteral("%1, %2, %3").arg(QColor(primary).red()).arg(QColor(primary).green()).arg(QColor(primary).blue()));

    qApp->setStyleSheet(style);
}

void MainWindow::captureBomSourceFromCurrentTable()
{
    m_bomSourceHeaders.clear();
    m_bomSourceRows.clear();
    if (!m_bomTable) {
        return;
    }

    for (int c = 0; c < m_bomTable->columnCount(); ++c) {
        auto *item = m_bomTable->horizontalHeaderItem(c);
        m_bomSourceHeaders.append(item ? item->text() : QStringLiteral("列%1").arg(c + 1));
    }

    for (int r = 0; r < m_bomTable->rowCount(); ++r) {
        QStringList row;
        for (int c = 0; c < m_bomTable->columnCount(); ++c) {
            auto *item = m_bomTable->item(r, c);
            row.append(item ? item->text() : QString());
        }
        m_bomSourceRows.append(row);
    }
}

void MainWindow::refreshBomColumnSelectors()
{
    if (m_bomColumnSelectors.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_bomColumnSelectors.size(); ++i) {
        auto *selector = m_bomColumnSelectors[i];
        if (!selector) {
            continue;
        }
        const QSignalBlocker blocker(selector);
        const QString current = selector->currentText();
        selector->clear();
        selector->addItems(m_bomSourceHeaders);
        int index = selector->findText(current, Qt::MatchExactly);
        if (index < 0) {
            index = qMin(i, selector->count() - 1);
        }
        selector->setCurrentIndex(qMax(index, 0));
    }
}

void MainWindow::applyBomColumnSelection()
{
    if (!m_bomTable || m_bomSourceHeaders.isEmpty() || m_bomColumnSelectors.isEmpty()) {
        return;
    }

    const int visibleCols = m_bomColumnSelectors.size();
    const int rowCount = m_bomSourceRows.size();
    m_bomTable->setSortingEnabled(false);
    m_bomTable->clear();
    m_bomTable->setColumnCount(visibleCols);
    m_bomTable->setRowCount(rowCount);

    QStringList viewHeaders;
    QList<int> sourceIndexes;
    for (int i = 0; i < visibleCols; ++i) {
        auto *selector = m_bomColumnSelectors[i];
        const QString header = selector ? selector->currentText() : QString();
        viewHeaders.append(header);
        sourceIndexes.append(m_bomSourceHeaders.indexOf(header));
    }
    m_bomTable->setHorizontalHeaderLabels(viewHeaders);

    for (int r = 0; r < rowCount; ++r) {
        for (int c = 0; c < sourceIndexes.size(); ++c) {
            const int sourceIndex = sourceIndexes[c];
            QString text;
            if (sourceIndex >= 0 && sourceIndex < m_bomSourceRows[r].size()) {
                text = m_bomSourceRows[r][sourceIndex];
            }
            m_bomTable->setItem(r, c, new QTableWidgetItem(text));
        }
    }

    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_bomTable->setSortingEnabled(true);
}

void MainWindow::updateStatus(const QString &message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

QString MainWindow::currentProjectText() const
{
    const auto *item = m_projectList ? m_projectList->currentItem() : nullptr;
    if (!item || item->text() == QStringLiteral("全部项目")) {
        return QStringLiteral("全部项目");
    }
    return item->text();
}

void MainWindow::applySearchHighlight(const QString &keyword)
{
    highlightInTable(m_bomTable, keyword);
    highlightInTable(m_inventoryTable, keyword);

    if (keyword.isEmpty()) {
        updateStatus(QStringLiteral("搜索关键字为空，显示全部内容。"));
        return;
    }

    int matchCount = 0;
    const QList<QTableWidget *> tables {m_bomTable, m_inventoryTable};
    for (auto *table : tables) {
        if (!table) {
            continue;
        }
        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                const auto *item = table->item(row, col);
                if (item && item->text().contains(keyword, Qt::CaseInsensitive)) {
                    ++matchCount;
                }
            }
        }
    }

    updateStatus(QStringLiteral("搜索“%1”命中 %2 项。已高亮显示。").arg(keyword).arg(matchCount));
}

void MainWindow::highlightInTable(QTableWidget *table, const QString &keyword)
{
    if (!table) {
        return;
    }

    const bool enableHighlight = !keyword.isEmpty();
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int col = 0; col < table->columnCount(); ++col) {
            auto *item = table->item(row, col);
            if (!item) {
                continue;
            }

            const bool matched = enableHighlight && item->text().contains(keyword, Qt::CaseInsensitive);
            item->setBackground(matched ? QColor(QStringLiteral("#FFE58A")) : QColor(Qt::white));
            item->setForeground(QColor(QStringLiteral("#111827")));
            item->setToolTip(matched ? QStringLiteral("命中关键词：%1").arg(keyword) : QString());
        }
    }
}

bool MainWindow::ensureProjectForImport(QString *projectName)
{
    if (!m_projectList) {
        return false;
    }

    QStringList choices;
    for (int i = 0; i < m_projectList->count(); ++i) {
        const QString text = m_projectList->item(i)->text().trimmed();
        if (!text.isEmpty() && text != QStringLiteral("全部项目")) {
            choices.append(text);
        }
    }
    choices.removeDuplicates();
    choices.append(QStringLiteral("＋ 新建项目"));

    bool ok = false;
    const QString picked = QInputDialog::getItem(this,
                                                 QStringLiteral("选择导入项目"),
                                                 QStringLiteral("请先选择一个项目（或新建）："),
                                                 choices,
                                                 0,
                                                 false,
                                                 &ok);
    if (!ok || picked.isEmpty()) {
        updateStatus(QStringLiteral("已取消导入：未选择项目。"));
        return false;
    }

    QString targetProject = picked;
    if (picked == QStringLiteral("＋ 新建项目")) {
        const QString name = QInputDialog::getText(this,
                                                   QStringLiteral("新建项目"),
                                                   QStringLiteral("项目名称："),
                                                   QLineEdit::Normal,
                                                   {},
                                                   &ok)
                                 .trimmed();
        if (!ok || name.isEmpty()) {
            updateStatus(QStringLiteral("已取消导入：未创建项目。"));
            return false;
        }
        targetProject = name;

        bool exists = false;
        for (int i = 0; i < m_projectList->count(); ++i) {
            if (m_projectList->item(i)->text() == targetProject) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_projectList->addItem(targetProject);
        }
    }

    QList<QListWidgetItem *> matched = m_projectList->findItems(targetProject, Qt::MatchExactly);
    if (!matched.isEmpty()) {
        m_projectList->setCurrentItem(matched.first());
    }

    if (projectName) {
        *projectName = targetProject;
    }
    return true;
}

bool MainWindow::importLichuangSpreadsheetFlow()
{
    QString projectName;
    if (!ensureProjectForImport(&projectName)) {
        return false;
    }

    const QString path = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("选择立创导出文件"),
                                                      QString(),
                                                      QStringLiteral("Spreadsheet Files (*.xlsx *.xls *.csv);;All Files (*.*)"));
    if (path.isEmpty()) {
        updateStatus(QStringLiteral("已取消立创导入。"));
        return false;
    }

    QString csvPath;
    QString error;
    if (path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        csvPath = path;
    } else if (!convertSpreadsheetToCsv(path, &csvPath, &error)) {
        updateStatus(QStringLiteral("导入失败：%1").arg(error));
        return false;
    }

    if (!loadLichuangCsvIntoBomTable(csvPath, projectName, &error)) {
        updateStatus(QStringLiteral("导入失败：%1").arg(error));
        return false;
    }

    applySearchHighlight(m_searchInput ? m_searchInput->text().trimmed() : QString());
    updateStatus(QStringLiteral("已导入立创 BOM：%1，绑定项目：%2").arg(QFileInfo(path).fileName(), projectName));
    return true;
}

bool MainWindow::importSpreadsheetFlow()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("选择 BOM 文件"),
                                                      QString(),
                                                      QStringLiteral("Spreadsheet Files (*.xlsx *.xls *.csv);;All Files (*.*)"));
    if (path.isEmpty()) {
        updateStatus(QStringLiteral("已取消 XLS/XLSX 导入。"));
        return false;
    }

    QString csvPath;
    QString error;

    if (path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        csvPath = path;
    } else if (!convertSpreadsheetToCsv(path, &csvPath, &error)) {
        updateStatus(QStringLiteral("导入失败：%1").arg(error));
        return false;
    }

    if (!loadCsvIntoBomTable(csvPath, &error)) {
        updateStatus(QStringLiteral("导入失败：%1").arg(error));
        return false;
    }

    applySearchHighlight(m_searchInput ? m_searchInput->text().trimmed() : QString());
    updateStatus(QStringLiteral("已完成导入：%1（项目范围：%2）").arg(QFileInfo(path).fileName(), currentProjectText()));
    return true;
}

bool MainWindow::convertSpreadsheetToCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const
{
    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        if (error) {
            *error = QStringLiteral("无法获取临时目录。");
        }
        return false;
    }

    const QFileInfo info(inputPath);
    const QString outPath = QDir(tempDir).filePath(QStringLiteral("%1_starbom.csv").arg(info.completeBaseName()));

    if (info.suffix().compare(QStringLiteral("xlsx"), Qt::CaseInsensitive) == 0
        || info.suffix().compare(QStringLiteral("xls"), Qt::CaseInsensitive) == 0) {
        QString pyError;
        if (convertExcelToCsvWithPython(inputPath, outPath, &pyError) && QFile::exists(outPath)) {
            if (outputCsvPath) {
                *outputCsvPath = outPath;
            }
            return true;
        }
        if (error && !pyError.isEmpty()) {
            *error = pyError;
        }
    }

    auto runConverter = [&](const QString &program, const QStringList &args) -> bool {
        QProcess process;
        process.start(program, args);
        if (!process.waitForStarted(3000)) {
            return false;
        }
        process.waitForFinished(20000);
        return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    };

    bool libreofficeOk = false;
    const QStringList officeCandidates {QStringLiteral("libreoffice"), QStringLiteral("soffice")};
    for (const QString &program : officeCandidates) {
        libreofficeOk = runConverter(program,
                                     {QStringLiteral("--headless"),
                                      QStringLiteral("--convert-to"),
                                      QStringLiteral("csv:Text - txt - csv (StarCalc):44,34,76,1"),
                                      QStringLiteral("--outdir"),
                                      QFileInfo(outPath).absolutePath(),
                                      inputPath});
        if (libreofficeOk) {
            break;
        }
    }

    if (libreofficeOk) {
        const QString converted = QDir(QFileInfo(outPath).absolutePath())
                                      .filePath(QStringLiteral("%1.csv").arg(info.completeBaseName()));
        if (QFile::exists(converted)) {
            if (outputCsvPath) {
                *outputCsvPath = converted;
            }
            return true;
        }
    }

    const bool ssconvertOk = runConverter(QStringLiteral("ssconvert"), {inputPath, outPath});
    if (ssconvertOk && QFile::exists(outPath)) {
        if (outputCsvPath) {
            *outputCsvPath = outPath;
        }
        return true;
    }

    if (error) {
        *error = QStringLiteral("导入失败：未检测到可用转换器（libreoffice/soffice/ssconvert），且内置 Excel 解析不可用。\n"
                                "建议：安装 libreoffice（含 soffice 命令）或 ssconvert，或先另存为 CSV。\n文件：%1")
                     .arg(inputPath);
    }
    return false;
}

bool MainWindow::convertXlsxToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
{
    const QString pythonCode = QStringLiteral(R"PY(
import csv
import sys
import zipfile
import xml.etree.ElementTree as ET

in_path, out_path = sys.argv[1], sys.argv[2]
ns = {'m': 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}

with zipfile.ZipFile(in_path, 'r') as zf:
    shared = []
    if 'xl/sharedStrings.xml' in zf.namelist():
        root = ET.fromstring(zf.read('xl/sharedStrings.xml'))
        for si in root.findall('m:si', ns):
            text = ''.join(t.text or '' for t in si.findall('.//m:t', ns))
            shared.append(text)

    sheet_name = 'xl/worksheets/sheet1.xml'
    if sheet_name not in zf.namelist():
        sheets = [n for n in zf.namelist() if n.startswith('xl/worksheets/sheet') and n.endswith('.xml')]
        if not sheets:
            raise RuntimeError('xlsx 中未找到工作表')
        sheet_name = sorted(sheets)[0]

    root = ET.fromstring(zf.read(sheet_name))
    rows = []
    for row in root.findall('.//m:sheetData/m:row', ns):
        cells = {}
        max_col = -1
        for c in row.findall('m:c', ns):
            ref = c.attrib.get('r', '')
            letters = ''.join(ch for ch in ref if ch.isalpha())
            col = 0
            for ch in letters:
                col = col * 26 + (ord(ch.upper()) - 64)
            col = max(col - 1, 0)
            max_col = max(max_col, col)

            t = c.attrib.get('t', '')
            v = c.find('m:v', ns)
            val = ''
            if t == 'inlineStr':
                it = c.find('m:is/m:t', ns)
                if it is not None and it.text:
                    val = it.text
            elif t == 's' and v is not None and v.text and v.text.isdigit():
                idx = int(v.text)
                if 0 <= idx < len(shared):
                    val = shared[idx]
            elif v is not None and v.text:
                val = v.text
            cells[col] = val

        if max_col >= 0:
            line = [''] * (max_col + 1)
            for k, v in cells.items():
                line[k] = v
            rows.append(line)

with open(out_path, 'w', encoding='utf-8', newline='') as fp:
    writer = csv.writer(fp)
    writer.writerows(rows)
)PY");

    QProcess process;
    process.start(QStringLiteral("python3"), {QStringLiteral("-c"), pythonCode, inputPath, outputPath});
    if (!process.waitForStarted(3000)) {
        if (error) {
            *error = QStringLiteral("未找到 python3，无法使用内置 xlsx 解析。");
        }
        return false;
    }

    process.waitForFinished(20000);
    const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && QFile::exists(outputPath);
    if (!ok && error) {
        const QString stderrMsg = QString::fromUtf8(process.readAllStandardError()).trimmed();
        *error = stderrMsg.isEmpty() ? QStringLiteral("内置 xlsx 解析失败。")
                                    : QStringLiteral("内置 xlsx 解析失败：%1").arg(stderrMsg);
    }
    return ok;
}

bool MainWindow::convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
{
    if (inputPath.endsWith(QStringLiteral(".xlsx"), Qt::CaseInsensitive)) {
        return convertXlsxToCsvWithPython(inputPath, outputPath, error);
    }

    QString program = QStringLiteral("python3");
    QProcess check;
    check.start(program, {QStringLiteral("--version")});
    if (!check.waitForStarted(2500)) {
        program = QStringLiteral("python");
    } else {
        check.waitForFinished(2500);
    }

    const QString pythonCode = QStringLiteral(R"PY(
import csv
import sys

in_path, out_path = sys.argv[1], sys.argv[2]

try:
    import xlrd
except Exception as exc:
    raise RuntimeError(f'缺少 xlrd 依赖，无法读取 .xls 文件: {exc}')

book = xlrd.open_workbook(in_path)
if book.nsheets <= 0:
    raise RuntimeError('xls 中未找到工作表')

sheet = book.sheet_by_index(0)
rows = []
for r in range(sheet.nrows):
    line = []
    for c in range(sheet.ncols):
        cell = sheet.cell_value(r, c)
        if isinstance(cell, float) and cell.is_integer():
            line.append(str(int(cell)))
        else:
            line.append(str(cell))
    rows.append(line)

with open(out_path, 'w', encoding='utf-8', newline='') as fp:
    writer = csv.writer(fp)
    writer.writerows(rows)
)PY");

    QProcess process;
    process.start(program, {QStringLiteral("-c"), pythonCode, inputPath, outputPath});
    if (!process.waitForStarted(3000)) {
        if (error) {
            *error = QStringLiteral("未找到 python3/python，无法执行 .xls 解析。");
        }
        return false;
    }

    process.waitForFinished(20000);
    const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && QFile::exists(outputPath);
    if (!ok && error) {
        const QString stderrMsg = QString::fromUtf8(process.readAllStandardError()).trimmed();
        if (stderrMsg.contains(QStringLiteral("xlrd"), Qt::CaseInsensitive)) {
            *error = QStringLiteral(".xls 导入失败：缺少 Python 包 xlrd。请执行 `python3 -m pip install xlrd` 后重试，或将文件另存为 .xlsx/.csv。\n%1")
                         .arg(stderrMsg);
        } else {
            *error = stderrMsg.isEmpty() ? QStringLiteral(".xls 解析失败。")
                                         : QStringLiteral(".xls 解析失败：%1").arg(stderrMsg);
        }
    }
    return ok;
}


bool MainWindow::loadCsvIntoBomTable(const QString &csvPath, QString *error)
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = QStringLiteral("无法打开 CSV 文件：%1").arg(csvPath);
        }
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QList<QStringList> rows;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }
        rows.append(parseCsvLine(line));
    }

    if (rows.isEmpty()) {
        if (error) {
            *error = QStringLiteral("CSV 为空，没有可导入数据。");
        }
        return false;
    }

    QStringList headers = rows.takeFirst();
    if (headers.isEmpty()) {
        headers = {QStringLiteral("项目"),
                   QStringLiteral("位号"),
                   QStringLiteral("分类"),
                   QStringLiteral("料号"),
                   QStringLiteral("规格"),
                   QStringLiteral("数量"),
                   QStringLiteral("供应商"),
                   QStringLiteral("备注")};
    }

    const int cols = qMax(headers.size(), 1);
    m_bomTable->clear();
    m_bomTable->setColumnCount(cols);
    m_bomTable->setHorizontalHeaderLabels(headers);
    m_bomTable->setRowCount(rows.size());

    for (int r = 0; r < rows.size(); ++r) {
        const QStringList row = rows[r];
        for (int c = 0; c < cols; ++c) {
            const QString text = c < row.size() ? row[c].trimmed() : QString();
            m_bomTable->setItem(r, c, new QTableWidgetItem(text));
        }
    }

    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    captureBomSourceFromCurrentTable();
    refreshBomColumnSelectors();
    applyBomColumnSelection();
    return true;
}

bool MainWindow::loadLichuangCsvIntoBomTable(const QString &csvPath, const QString &projectName, QString *error)
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = QStringLiteral("无法打开 CSV 文件：%1").arg(csvPath);
        }
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QList<QStringList> rows;
    while (!in.atEnd()) {
        rows.append(parseCsvLine(in.readLine()));
    }

    const auto normalized = [](QString v) {
        return v.remove(' ').remove('	').trimmed();
    };

    int headerRow = -1;
    for (int r = 0; r < rows.size(); ++r) {
        const QString merged = normalized(rows[r].join(QString()));
        if (merged.contains(QStringLiteral("商品编号"))
            && merged.contains(QStringLiteral("厂家型号"))
            && merged.contains(QStringLiteral("订购数量（修改后）"))
            && merged.contains(QStringLiteral("商品金额"))) {
            headerRow = r;
            break;
        }
    }

    if (headerRow < 0) {
        if (error) {
            *error = QStringLiteral("未识别到立创表头（应包含第18行字段）。");
        }
        return false;
    }

    QList<QStringList> dataRows;
    for (int r = headerRow + 1; r < rows.size(); ++r) {
        const QStringList row = rows[r];
        const auto at = [&](int idx) { return idx < row.size() ? row[idx].trimmed() : QString(); };

        const QString itemCode = at(1);
        const QString brand = at(2);
        const QString model = at(3);
        const QString pkg = at(4);
        const QString name = at(5);
        const QString qty = at(6);
        const QString unitPrice = at(9);
        const QString amount = at(10);

        const bool empty = itemCode.isEmpty() && brand.isEmpty() && model.isEmpty()
                           && pkg.isEmpty() && name.isEmpty() && qty.isEmpty()
                           && unitPrice.isEmpty() && amount.isEmpty();
        if (empty) {
            continue;
        }

        dataRows.append({projectName, itemCode, brand, model, pkg, name, qty, unitPrice, amount});
    }

    if (dataRows.isEmpty()) {
        if (error) {
            *error = QStringLiteral("立创导入未找到有效数据（应从第19行开始）。");
        }
        return false;
    }

    m_bomTable->clear();
    m_bomTable->setColumnCount(9);
    m_bomTable->setHorizontalHeaderLabels({QStringLiteral("项目"),
                                           QStringLiteral("商品编号"),
                                           QStringLiteral("品牌"),
                                           QStringLiteral("厂家型号"),
                                           QStringLiteral("封装"),
                                           QStringLiteral("商品名称"),
                                           QStringLiteral("订购数量（修改后）"),
                                           QStringLiteral("商品单价"),
                                           QStringLiteral("商品金额")});
    m_bomTable->setRowCount(dataRows.size());

    for (int r = 0; r < dataRows.size(); ++r) {
        for (int c = 0; c < dataRows[r].size(); ++c) {
            m_bomTable->setItem(r, c, new QTableWidgetItem(dataRows[r][c]));
        }
    }

    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    captureBomSourceFromCurrentTable();
    refreshBomColumnSelectors();
    applyBomColumnSelection();
    return true;
}

QStringList MainWindow::parseCsvLine(const QString &line) const
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current.append('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            result.append(current);
            current.clear();
        } else {
            current.append(ch);
        }
    }

    result.append(current);
    return result;
}
