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
    leftPanel->setMinimumWidth(420);
    leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    splitter->setSizes({qRound(width() * 0.22), qRound(width() * 0.78)});

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
    auto *versionLabel = new QLabel(QStringLiteral("v0.3.0"), brandFrame);
    versionLabel->setObjectName(QStringLiteral("versionLabel"));

    brandLayout->addWidget(titleLabel, 0, 0, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    brandLayout->addWidget(versionLabel, 1, 1, 1, 1, Qt::AlignRight | Qt::AlignTop);

    auto *githubLink = new QLabel(leftPanel);
    githubLink->setText(QStringLiteral("🐙 <a href='https://github.com/890mn/StarBOM'>890mn</a>"));
    githubLink->setTextFormat(Qt::RichText);
    githubLink->setOpenExternalLinks(true);
    githubLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    githubLink->setObjectName(QStringLiteral("githubLabel"));

    auto *importGroup = new QGroupBox(QStringLiteral("导入"), leftPanel);
    auto *importLayout = new QVBoxLayout(importGroup);
    importLayout->setSpacing(10);

    m_quickImportInput = new QLineEdit(importGroup);
    m_quickImportInput->setMinimumHeight(38);
    m_quickImportInput->setPlaceholderText(QStringLiteral("立创导出内容关键字 / 链接（后续完善）"));

    auto *quickImportBtn = new QPushButton(QStringLiteral("立创导入（规划中）"), importGroup);
    auto *xlsImportBtn = new QPushButton(QStringLiteral("从 XLS/XLSX 导入"), importGroup);
    auto *ocrImportBtn = new QPushButton(QStringLiteral("OCR 图片导入（后续）"), importGroup);
    quickImportBtn->setMinimumHeight(40);
    xlsImportBtn->setMinimumHeight(42);
    ocrImportBtn->setMinimumHeight(40);

    importLayout->addWidget(m_quickImportInput);
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

    auto *themeGroup = new QGroupBox(QStringLiteral("主题"), leftPanel);
    auto *themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(8);
    m_themeSelector = new QComboBox(themeGroup);
    m_themeSelector->setMinimumHeight(38);
    m_themeSelector->addItems({QStringLiteral("Aurora Triad"), QStringLiteral("Citrus Triad"), QStringLiteral("Slate Triad")});
    themeLayout->addWidget(new QLabel(QStringLiteral("三元色主题："), themeGroup));
    themeLayout->addWidget(m_themeSelector);

    layout->addWidget(brandFrame);
    layout->addWidget(githubLink);
    layout->addWidget(importGroup);
    layout->addWidget(exportGroup);
    layout->addWidget(projectGroup, 2);
    layout->addWidget(categoryGroup, 2);
    layout->addWidget(themeGroup);
    layout->addStretch();

    connect(quickImportBtn, &QPushButton::clicked, this, [this] {
        const QString key = m_quickImportInput->text().trimmed();
        if (key.isEmpty()) {
            updateStatus(QStringLiteral("立创导入：请先输入关键字或链接。"));
            return;
        }
        updateStatus(QStringLiteral("立创导入已就绪，目标项目：%1，输入：%2").arg(currentProjectText(), key));
    });

    connect(xlsImportBtn, &QPushButton::clicked, this, [this] {
        importSpreadsheetFlow();
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
    m_bomTable = new QTableWidget(6, 8, bomPage);
    m_bomTable->setHorizontalHeaderLabels({QStringLiteral("项目"),
                                           QStringLiteral("位号"),
                                           QStringLiteral("分类"),
                                           QStringLiteral("料号"),
                                           QStringLiteral("规格"),
                                           QStringLiteral("数量"),
                                           QStringLiteral("供应商"),
                                           QStringLiteral("备注")});
    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    bomLayout->addWidget(m_bomTable);

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
        QLineEdit, QListWidget, QTableWidget, QComboBox {
            border: 1px solid #CCD6E2;
            border-radius: 8px;
            background: white;
            padding: 6px;
        }
        QListWidget::item:selected {
            background: rgba(46, 91, 255, 0.18);
            border-radius: 4px;
            color: #0F172A;
            font-weight: 700;
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
                              .arg(panel, text, primary, secondary, accent);

    qApp->setStyleSheet(style);
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

    auto runConverter = [&](const QString &program, const QStringList &args) -> bool {
        QProcess process;
        process.start(program, args);
        if (!process.waitForStarted(3000)) {
            return false;
        }
        process.waitForFinished(20000);
        return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    };

    const bool libreofficeOk = runConverter(QStringLiteral("libreoffice"),
                                            {QStringLiteral("--headless"),
                                             QStringLiteral("--convert-to"),
                                             QStringLiteral("csv"),
                                             QStringLiteral("--outdir"),
                                             QFileInfo(outPath).absolutePath(),
                                             inputPath});

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
        *error = QStringLiteral("系统未检测到可用转换器（libreoffice/ssconvert），请安装后重试，或先另存为 CSV。\n文件：%1")
                     .arg(inputPath);
    }
    return false;
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

    m_bomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
