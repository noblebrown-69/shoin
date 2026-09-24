#include "ShoinFrame.h"
#include "MonasteryEditor.h"
#include "DocumentIo.h"
#include <QApplication>
#include <algorithm>
#include <memory>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QFontComboBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QCloseEvent>
#include <QPixmap>
#include <QIcon>
#include <QRegularExpression>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QMarginsF>
#include <QProcess>
#include <QSet>
#include <QDebug>
#include <QTemporaryFile>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFileInfo>
#include <QPainter>
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QDateTime>
#include <QInputDialog>
#include <QSettings>
#include <QEventLoop>
#include <QWebEnginePage>
#include <QStringConverter>
#include <QSignalBlocker>
#include <QCoreApplication>
#include <QList>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <cstdio>
#include <QElapsedTimer>

// Embedded XPM icons for classic Word 6.0 look
static const char *bold_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"B c Black",
"BBBB............",
"B...B...........",
"B...B...........",
"BBBB............",
"B...B...........",
"B...B...........",
"BBBB............",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................","................",nullptr
};

static const char *italic_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"I c Black",
"........I.......",
".........I......",
"..........I.....",
"...........I....",
"............I...",
".............I..",
"..............I.",
"...............I",
"..............I.",
".............I..",
"............I...",
"...........I....",
"..........I.....",
".........I......",
"........I.......",
"................",
nullptr
};

static const char *underline_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"U c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"UUUUUUUUUUUUUUUU",
nullptr
};

static const char *alignleft_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"L c Black",
"   L............",
"  L.............",
" L..............",
"L...............",
" L..............",
"  L.............",
"   L............",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
nullptr
};

static const char *aligncenter_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"C c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
".....CCCCCCCC...",
nullptr
};

static const char *alignright_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"R c Black",
"............R   ",
".............R  ",
"..............R ",
"...............R",
"..............R ",
".............R  ",
"............R   ",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
nullptr
};

static const char *justify_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"J c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"JJJJJJJJJJJJJJJJ",
nullptr
};

static const char *bullet_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"o c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"......ooo.......",
nullptr
};

static const char *number_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"1 c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"......1.........",
nullptr
};

const char *folder_xpm[] = {
"16 16 3 1",
"  c None",
". c #3C2F2F",
"F c #D4AF37",
"................",
"................",
"................",
".FFFFFFFFFFFF...",
".F............F.",
".F............F.",
".F............F.",
".F............F.",
".F............F.",
".F............F.",
".F............F.",
".F............F.",
".FFFFFFFFFFFF...",
"................",
"................",
"................",nullptr
};



static const QString kDocumentFilter =
    QStringLiteral("Documents (*.html *.md *.markdown *.txt *.docx);;HTML (*.html);;Markdown (*.md *.markdown *.txt);;Word (*.docx)");

static QString ensureDocumentSuffix(QString fileName)
{
    const QString lower = fileName.toLower();
    if (lower.endsWith(QLatin1String(".html"))
        || lower.endsWith(QLatin1String(".md"))
        || lower.endsWith(QLatin1String(".markdown"))
        || lower.endsWith(QLatin1String(".txt"))
        || lower.endsWith(QLatin1String(".docx")))
        return fileName;
    return fileName + QStringLiteral(".html");
}

class MarkdownHighlighter : public QSyntaxHighlighter {
public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
        m_heading.setForeground(QColor(QStringLiteral("#4E9A06")));
        m_heading.setFontWeight(QFont::Bold);
        m_bold.setForeground(QColor(QStringLiteral("#CC0000")));
        m_bold.setFontWeight(QFont::Bold);
        m_code.setForeground(QColor(QStringLiteral("#3465A4")));
    }

protected:
    void highlightBlock(const QString &text) override
    {
        static const QRegularExpression headingRe(QStringLiteral("^#{1,6}\\s+.*"));
        static const QRegularExpression boldRe(QStringLiteral("(\\*\\*[^*]+\\*\\*|__[^_]+__)"));
        static const QRegularExpression codeRe(QStringLiteral("`[^`]+`"));
        if (headingRe.match(text).hasMatch())
            setFormat(0, text.size(), m_heading);
        for (auto it = boldRe.globalMatch(text); it.hasNext(); ) {
            const auto mm = it.next();
            setFormat(mm.capturedStart(), mm.capturedLength(), m_bold);
        }
        for (auto it = codeRe.globalMatch(text); it.hasNext(); ) {
            const auto mm = it.next();
            setFormat(mm.capturedStart(), mm.capturedLength(), m_code);
        }
    }

private:
    QTextCharFormat m_heading;
    QTextCharFormat m_bold;
    QTextCharFormat m_code;
};

static bool listenModeEnabled()
{
    const QStringList args = QCoreApplication::arguments();
    if (args.contains(QStringLiteral("--listen"))
        || args.contains(QStringLiteral("--save-as"))
        || args.contains(QStringLiteral("--print-info"))
        || args.contains(QStringLiteral("--print-to-file"))
        || args.contains(QStringLiteral("--print-lp")))
        return true;
    return qEnvironmentVariableIntValue("SHOIN_LISTEN") != 0
        || !qEnvironmentVariable("SHOIN_SAVE_AS").isEmpty();
}

static void listenLog(const char *key, const QString &value)
{
    std::fprintf(stdout, "%s: %s\n", key, qPrintable(value));
    std::fflush(stdout);
}

static QString argValueAfter(const QString &flag)
{
    const QStringList args = QCoreApplication::arguments();
    const int i = args.indexOf(flag);
    if (i >= 0 && i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-')))
        return args.at(i + 1);
    return QString();
}

static QString listenPrintToFilePath()
{
    return argValueAfter(QStringLiteral("--print-to-file"));
}

static bool listenPrintLpRequested()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--print-lp"));
}

static bool listenPrintLpExecute()
{
    return qEnvironmentVariableIntValue("MONASTERY_PRINT_LP") != 0
        || qEnvironmentVariableIntValue("SHOIN_PRINT_LP") != 0;
}

static QPageLayout defaultPrintPageLayout()
{
    return QPageLayout(QPageSize(QPageSize::Letter),
                       QPageLayout::Portrait,
                       QMarginsF(0.75, 0.75, 0.75, 0.75),
                       QPageLayout::Inch);
}

static QPageLayout pageLayoutFromPrinter(const QPrinter &printer)
{
    const QPageLayout layout = printer.pageLayout();
    return layout.isValid() ? layout : defaultPrintPageLayout();
}

static QStringList cupsLpArgv(const QString &printerName, int copies, const QString &pdfPath)
{
    QStringList args;
    if (!printerName.isEmpty())
        args << QStringLiteral("-d") << printerName;
    if (copies > 1)
        args << QStringLiteral("-n") << QString::number(copies);
    args << pdfPath;
    return args;
}

static QString formatCommandLine(const QString &program, const QStringList &args)
{
    QStringList parts;
    parts << program;
    parts += args;
    return parts.join(QLatin1Char(' '));
}

static void listenLogPrinters()
{
    const QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
    const QString def = QPrinterInfo::defaultPrinterName();
    bool brother = false;
    for (const QPrinterInfo &info : printers) {
        if (info.printerName().contains(QStringLiteral("Brother"), Qt::CaseInsensitive)) {
            brother = true;
            break;
        }
    }
    listenLog("printers", QString::number(printers.size()));
    listenLog("default_printer", def.isEmpty() ? QStringLiteral("(none)") : def);
    listenLog("has_brother", brother ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("print_dialog", printers.isEmpty() ? QStringLiteral("qt") : QStringLiteral("cups"));
}


static void showFamilyInCombo(QFontComboBox *combo, const QString &family)
{
    if (!combo || family.isEmpty())
        return;
    QSignalBlocker block(combo);
    combo->setCurrentFont(QFont(family));
    if (QString::compare(combo->currentText(), family, Qt::CaseInsensitive) == 0)
        return;
    if (!combo->isEditable())
        combo->setEditable(true);
    combo->setEditText(family);
    if (QString::compare(combo->currentText(), family, Qt::CaseInsensitive) != 0)
        combo->setCurrentText(family);
}

static void showSizeInCombo(QComboBox *combo, int pt)
{
    if (!combo || pt <= 0)
        return;
    QSignalBlocker block(combo);
    const QString s = QString::number(pt);
    if (combo->findText(s) < 0) {
        int i = 0;
        for (; i < combo->count(); ++i) {
            if (combo->itemText(i).toInt() > pt)
                break;
        }
        combo->insertItem(i, s);
    }
    combo->setCurrentText(s);
}


static bool isDocxPath(const QString &path)
{
    return QFileInfo(path).suffix().toLower() == QLatin1String("docx");
}

static QString listenSaveAsPath()
{
    const QStringList args = QCoreApplication::arguments();
    const int i = args.indexOf(QStringLiteral("--save-as"));
    if (i >= 0 && i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-')))
        return args.at(i + 1);
    const QString env = QString::fromLocal8Bit(qgetenv("SHOIN_SAVE_AS"));
    if (!env.isEmpty())
        return env;
    return QString::fromLocal8Bit(qgetenv("LISTEN_SAVE_AS"));
}

static void listenLogDocxPeek(const QString &path)
{
    bool hasCt = false;
    bool hasDoc = false;
    bool leftover = false;
    QString err;
    DocumentIo::docxPeek(path, &hasCt, &hasDoc, &leftover, &err);
    listenLog("has_content_types", hasCt ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("has_document_xml", hasDoc ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("leftover_html", leftover ? QStringLiteral("yes") : QStringLiteral("no"));
}

static bool isSourceDocumentPath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    return ext == QLatin1String("md")
        || ext == QLatin1String("markdown")
        || ext == QLatin1String("txt");
}

ShoinFrame::ShoinFrame(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMouseTracking(true);
    setStyleSheet("QWidget { background-color: #3C2F2F; }");
    setFont(QFont("Noto Serif", 12));
    setGeometry(100, 100, 800, 600);

    createDocsFolder();
    m_currentFilePath.clear();
    m_dragging = false;
    m_resizing = false;
    m_resizeDirection = None;
    m_currentTheme = themeForId(ThemeId::Leather);

    createActions();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    m_titleBar = new QWidget;
    m_titleBar->setStyleSheet("background-color: #3C2F2F;");
    m_titleBar->setFixedHeight(30);
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(10,0,10,0);

    m_minBtn = new QPushButton("—");
    m_minBtn->setFixedSize(30,30);
    m_minBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    m_maxBtn = new QPushButton("□");
    m_maxBtn->setFixedSize(30,30);
    m_maxBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_maxBtn, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
            setGeometry(m_normalGeometry);
        } else {
            m_normalGeometry = geometry();
            showMaximized();
        }
    });
    m_closeBtn = new QPushButton("×");
    m_closeBtn->setFixedSize(30,30);
    m_closeBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_closeBtn, &QPushButton::clicked, this, &QWidget::close);

    m_titleLabel = new QLabel("Shoin — 書院");
    QFont titleFont("Noto Serif", 10, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #D4AF37;");

    QWidget *leftSpacer = new QWidget();
    leftSpacer->setFixedWidth(90);

    titleLayout->addWidget(leftSpacer);
    titleLayout->addStretch();
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_minBtn);
    titleLayout->addWidget(m_maxBtn);
    titleLayout->addWidget(m_closeBtn);
    mainLayout->addWidget(m_titleBar);

    m_menuBar = new QMenuBar;
    QMenu *fileMenu = m_menuBar->addMenu("&File");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addAction(m_printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    for (QAction *action : fileMenu->actions())
        action->setIconVisibleInMenu(false);

    QMenu *editMenu = m_menuBar->addMenu("&Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_pageBreakAction);
    editMenu->addAction(m_checklistAction);

    QMenu *themeMenu = m_menuBar->addMenu("&Theme");
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);
    for (const Theme &th : allThemes()) {
        QAction *action = themeMenu->addAction(themeMenuName(th));
        action->setCheckable(true);
        action->setData(th.id);
        m_themeGroup->addAction(action);
        const ThemeId id = th.themeId;
        connect(action, &QAction::triggered, this, [this, id](bool checked) {
            if (checked)
                applyTheme(id);
        });
    }
    mainLayout->addWidget(m_menuBar);

    m_toolBar = new QToolBar;
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setIconSize(QSize(16, 16));

    m_boldAction->setIcon(QIcon(":/icons/bold.png"));
    m_italicAction->setIcon(QIcon(":/icons/italic.png"));
    m_underlineAction->setIcon(QIcon(":/icons/underline.png"));
    m_strikethroughAction->setIcon(QIcon(":/icons/strikethru.png"));
    m_alignLeftAction->setIcon(QIcon(":/icons/alignleft.png"));
    m_alignCenterAction->setIcon(QIcon(":/icons/aligncenter.png"));
    m_alignRightAction->setIcon(QIcon(":/icons/alignright.png"));
    m_justifyAction->setIcon(QIcon(":/icons/justify.png"));
    m_bulletAction->setIcon(QIcon(":/icons/bullet.png"));
    m_numberAction->setIcon(QIcon(":/icons/numbered.png"));
    m_checklistAction->setIcon(QIcon(":/icons/checklist.png"));
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();
    m_fontCombo = new QFontComboBox();
    m_fontCombo->setEditable(true);
    m_fontCombo->setCurrentFont(QFont("Noto Serif"));
    connect(m_fontCombo, &QComboBox::textActivated, this, &ShoinFrame::onFontChanged);
    m_toolBar->addWidget(m_fontCombo);
    m_sizeCombo = new QComboBox();
    m_sizeCombo->addItems({"8", "10", "12", "14", "16", "18", "20", "24", "28", "32"});
    m_sizeCombo->setCurrentText("12");
    connect(m_sizeCombo, &QComboBox::textActivated, this, &ShoinFrame::onSizeChanged);
    m_toolBar->addWidget(m_sizeCombo);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_boldAction);
    m_toolBar->addAction(m_italicAction);
    m_toolBar->addAction(m_underlineAction);
    m_toolBar->addAction(m_strikethroughAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_alignLeftAction);
    m_toolBar->addAction(m_alignCenterAction);
    m_toolBar->addAction(m_alignRightAction);
    m_toolBar->addAction(m_justifyAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_bulletAction);
    m_toolBar->addAction(m_numberAction);
    m_toolBar->addAction(m_checklistAction);
    mainLayout->addWidget(m_toolBar);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setReadOnly(false);
    m_fileModel->setRootPath(m_docsDir);
    m_fileModel->setNameFilters(QStringList() << "*.html" << "*.md" << "*.markdown" << "*.txt" << "*.docx");
    m_fileModel->setNameFilterDisables(false);
    m_fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_fileModel->setIconProvider(new SimpleIconProvider());

    m_treeView = new QTreeView(this);
    m_treeView->setAnimated(true);
    m_treeView->setItemsExpandable(true);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setRootIsDecorated(true);
    connect(m_treeView, &QTreeView::clicked, this, &ShoinFrame::onTreeClicked);
    m_treeView->setModel(m_fileModel);
    m_treeView->setRootIndex(m_fileModel->index(m_docsDir));
    m_treeView->setColumnHidden(1, true);
    m_treeView->setColumnHidden(2, true);
    m_treeView->setColumnHidden(3, true);
    m_treeView->setHeaderHidden(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QTreeView::doubleClicked, this, &ShoinFrame::onTreeDoubleClicked);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ShoinFrame::onTreeSelectionChanged);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &ShoinFrame::onTreeContextMenu);

    m_splitter->addWidget(m_treeView);

    m_treeView->setDragEnabled(true);
    m_treeView->setAcceptDrops(true);
    m_treeView->setDropIndicatorShown(true);
    m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);

    QWidget *editorContainer = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorContainer);
    editorLayout->setContentsMargins(0,0,0,0);
    editorLayout->setSpacing(0);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("Select a file to rename");
    m_titleEdit->setEnabled(false);
    connect(m_titleEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_currentFilePath.isEmpty())
            return;
        const QString typed = m_titleEdit->text().trimmed();
        if (typed.isEmpty())
            return;

        QString suffix = QFileInfo(m_currentFilePath).suffix();
        if (suffix.isEmpty())
            suffix = QStringLiteral("html");

        // Title edit shows basename without extension; keep that contract.
        QString base = typed;
        if (base.endsWith(QLatin1Char('.') + suffix, Qt::CaseInsensitive))
            base.chop(suffix.size() + 1);

        const QFileInfo oldInfo(m_currentFilePath);
        const QString newPath = oldInfo.absoluteDir().absoluteFilePath(base + QLatin1Char('.') + suffix);

        // Same path: no-op (avoid rename + FS model churn while still in returnPressed).
        if (QFileInfo(newPath).absoluteFilePath() == oldInfo.absoluteFilePath())
            return;

        if (QFile::exists(newPath)) {
            QMessageBox::warning(this, QStringLiteral("Rename"),
                                 QStringLiteral("A file named \"%1\" already exists.")
                                     .arg(QFileInfo(newPath).fileName()));
            return;
        }

        // Suppress tree selection handler for the whole rename window. QFileSystemModel
        // often emits an empty selection mid-rename; clearing/disabling m_titleEdit
        // while still inside returnPressed re-enters Qt Widgets paint/layout → SEGV.
        m_suppressTreeLoad = true;
        if (!QFile::rename(m_currentFilePath, newPath)) {
            m_suppressTreeLoad = false;
            QMessageBox::warning(this, QStringLiteral("Rename"),
                                 QStringLiteral("Could not rename file."));
            return;
        }

        m_currentFilePath = newPath;
        m_titleEdit->setText(QFileInfo(newPath).completeBaseName());
        updateTitleBar();
        m_statusBar->showMessage(QStringLiteral("File renamed to: %1")
                                     .arg(QFileInfo(newPath).fileName()));
        m_titleEdit->clearFocus();

        // Defer tree reselection until after the key event finishes.
        // restoreTreeSelection flips suppress internally; clear once afterward
        // so nested suppress cannot stick true.
        QTimer::singleShot(0, this, [this]() {
            restoreTreeSelection();
            m_suppressTreeLoad = false;
        });
    });
    editorLayout->addWidget(m_titleEdit);

    m_editorStack = new QStackedWidget(this);
    m_editor = new MonasteryEditor(m_editorStack);
    m_mdEdit = new QPlainTextEdit(m_editorStack);
    {
        QFont mono;
        mono.setFamilies({QStringLiteral("DejaVu Sans Mono"), QStringLiteral("Liberation Mono"),
                          QStringLiteral("Courier New"), QStringLiteral("monospace")});
        mono.setPointSize(12);
        m_mdEdit->setFont(mono);
    }
    m_mdEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_mdEdit->setTabStopDistance(32);
    m_mdEdit->setStyleSheet(QStringLiteral(
        "QPlainTextEdit { background-color: #1e1a17; color: #F5E8C7; border: none;"
        " selection-background-color: #5C4A3F; }"));
    new MarkdownHighlighter(m_mdEdit->document());
    connect(m_mdEdit, &QPlainTextEdit::modificationChanged, this, [this](bool) {
        updateTitleBar();
    });
    connect(m_mdEdit, &QPlainTextEdit::textChanged, this, [this]() {
        if (isMarkdownMode())
            updateWordCount();
    });
    m_editorStack->addWidget(m_editor);
    m_editorStack->addWidget(m_mdEdit);
    m_editorStack->setCurrentWidget(m_editor);
    editorLayout->addWidget(m_editorStack);
    m_splitter->addWidget(editorContainer);
    mainLayout->addWidget(m_splitter, 1);

    connect(m_editor, &MonasteryEditor::wordCountChanged, this, [this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
    connect(m_editor, &MonasteryEditor::selectionFontChanged, this, &ShoinFrame::onSelectionFontChanged);
    connect(m_editor, &MonasteryEditor::dirtyChanged, this, [this](bool) {
        updateTitleBar();
    });
    connect(m_editor, &MonasteryEditor::ready, this, [this](bool ok) {
        if (ok)
            applyTheme(m_currentTheme.themeId);
        if (!listenModeEnabled())
            return;
        QTimer::singleShot(250, this, [this]() {
            if (m_listenQuitArmed || m_listenPrintPending)
                return;
            if (!m_currentFilePath.isEmpty())
                return;
            emitListenHealth(QString());
            if (!maybeStartListenPrint())
                requestListenQuit();
        });
    });
    connect(m_editor, &MonasteryEditor::ledgerCheckRequested,
            this, &ShoinFrame::onLedgerCheckRequested);
    connect(m_editor->webView()->page(), &QWebEnginePage::pdfPrintingFinished,
            this, &ShoinFrame::onPdfPrintingFinished);

    m_statusBar = new QStatusBar;
    m_statusBar->setSizeGripEnabled(true);
    m_statusBar->setStyleSheet("background-color: #3C2F2F; color: #D4AF37;");
    m_statusBar->setFont(QFont("Noto Serif", 8));
    m_wordCountLabel = new QLabel("Words: 0");
    m_wordCountLabel->setAlignment(Qt::AlignRight);
    m_statusBar->addPermanentWidget(m_wordCountLabel);
    mainLayout->addWidget(m_statusBar);

    setLayout(mainLayout);
    setWindowIcon(QIcon(":/icons/shoin.png"));
    setMinimumSize(680, 460);
    m_normalGeometry = geometry();

    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ShoinFrame::onAutoSave);
    m_autoSaveTimer->start(30000);

    connect(m_undoAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->undo(); return; }
        m_editor->execCommand("undo");
    });
    connect(m_redoAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->redo(); return; }
        m_editor->execCommand("redo");
    });
    connect(m_cutAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->cut(); return; }
        m_editor->execCommand("cut");
    });
    connect(m_copyAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->copy(); return; }
        m_editor->execCommand("copy");
    });
    connect(m_pasteAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->paste(); return; }
        m_editor->execCommand("paste");
    });

    m_wordCountPollTimer = new QTimer(this);
    m_wordCountPollTimer->setInterval(900);
    connect(m_wordCountPollTimer, &QTimer::timeout, this, &ShoinFrame::updateWordCount);
    m_wordCountPollTimer->start();

    m_titleBar->installEventFilter(this);
    m_menuBar->installEventFilter(this);
    m_toolBar->installEventFilter(this);
    m_statusBar->installEventFilter(this);

    m_statusBar->showMessage("Ready");
    updateWordCount();

    QSettings settings(QStringLiteral("Shoin"), QStringLiteral("Shoin"));
    applyTheme(themeIdFromString(settings.value(QStringLiteral("theme"), QStringLiteral("leather")).toString()));
    updateTitleBar();
}

ShoinFrame::~ShoinFrame() {
}

QString ShoinFrame::getRealAppDir() {
    QByteArray appImage = qgetenv("APPIMAGE");
    if (!appImage.isEmpty()) {
        return QFileInfo(QString::fromUtf8(appImage)).absolutePath();
    }
    return QApplication::applicationDirPath();
}

void ShoinFrame::createDocsFolder() {
    m_docsDir = getRealAppDir() + "/Docs";
    QDir().mkpath(m_docsDir);
    QDir().mkpath(m_docsDir + "/Articles");
    QDir().mkpath(m_docsDir + "/Journal");
}

QString ShoinFrame::themeMenuName(const Theme &th) const {
    if (th.themeId == ThemeId::WordPerfect)
        return QStringLiteral("WordPerfect 5.1");
    if (th.themeId == ThemeId::Amber)
        return QStringLiteral("IBM Amber");
    return th.name;
}

void ShoinFrame::createActions() {
    m_newAction = new QAction("&New", this);
    m_newAction->setIcon(QIcon(":/icons/new.png"));
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setToolTip("New Entry");
    connect(m_newAction, &QAction::triggered, this, &ShoinFrame::onNewEntry);

    m_openAction = new QAction("&Open", this);
    m_openAction->setIcon(QIcon(":/icons/open.png"));
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setToolTip("New Folder");
    connect(m_openAction, &QAction::triggered, this, &ShoinFrame::onNewFolder);

    m_saveAction = new QAction("&Save", this);
    m_saveAction->setIcon(QIcon(":/icons/save.png"));
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &ShoinFrame::onSave);

    m_saveAsAction = new QAction("Save &As...", this);
    m_saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, &ShoinFrame::onSaveAs);

    m_printAction = new QAction("&Print", this);
    m_printAction->setShortcut(QKeySequence::Print);
    connect(m_printAction, &QAction::triggered, this, &ShoinFrame::onPrint);

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &ShoinFrame::onExit);

    m_boldAction = new QAction(this);
    m_boldAction->setCheckable(true);
    m_boldAction->setToolTip("Bold");
    m_boldAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(m_boldAction, &QAction::triggered, this, &ShoinFrame::onBold);

    m_italicAction = new QAction(this);
    m_italicAction->setCheckable(true);
    m_italicAction->setToolTip("Italic");
    m_italicAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(m_italicAction, &QAction::triggered, this, &ShoinFrame::onItalic);

    m_underlineAction = new QAction(this);
    m_underlineAction->setCheckable(true);
    m_underlineAction->setToolTip("Underline");
    m_underlineAction->setShortcut(QKeySequence("Ctrl+U"));
    connect(m_underlineAction, &QAction::triggered, this, &ShoinFrame::onUnderline);

    m_strikethroughAction = new QAction(this);
    m_strikethroughAction->setCheckable(true);
    m_strikethroughAction->setToolTip("Strikethrough");
    m_strikethroughAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(m_strikethroughAction, &QAction::triggered, this, &ShoinFrame::onStrikethrough);

    QActionGroup *alignGroup = new QActionGroup(this);

    m_alignLeftAction = new QAction(this);
    m_alignLeftAction->setCheckable(true);
    m_alignLeftAction->setToolTip("Align Left");
    connect(m_alignLeftAction, &QAction::triggered, this, &ShoinFrame::onAlignLeft);
    alignGroup->addAction(m_alignLeftAction);

    m_alignCenterAction = new QAction(this);
    m_alignCenterAction->setCheckable(true);
    m_alignCenterAction->setToolTip("Align Center");
    connect(m_alignCenterAction, &QAction::triggered, this, &ShoinFrame::onAlignCenter);
    alignGroup->addAction(m_alignCenterAction);

    m_alignRightAction = new QAction(this);
    m_alignRightAction->setCheckable(true);
    m_alignRightAction->setToolTip("Align Right");
    connect(m_alignRightAction, &QAction::triggered, this, &ShoinFrame::onAlignRight);
    alignGroup->addAction(m_alignRightAction);

    m_justifyAction = new QAction(this);
    m_justifyAction->setCheckable(true);
    m_justifyAction->setToolTip("Justify");
    connect(m_justifyAction, &QAction::triggered, this, &ShoinFrame::onJustify);
    alignGroup->addAction(m_justifyAction);

    m_bulletAction = new QAction(this);
    m_bulletAction->setToolTip("Bulleted List");
    connect(m_bulletAction, &QAction::triggered, this, &ShoinFrame::onBulletList);

    m_numberAction = new QAction(this);
    m_numberAction->setToolTip("Numbered List");
    connect(m_numberAction, &QAction::triggered, this, &ShoinFrame::onNumberedList);

    m_checklistAction = new QAction("Checklist", this);
    m_checklistAction->setToolTip("Checklist");
    connect(m_checklistAction, &QAction::triggered, this, &ShoinFrame::onChecklist);

    m_undoAction = new QAction("Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = new QAction("Redo", this);
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_cutAction = new QAction("Cu&t", this);
    m_cutAction->setShortcut(QKeySequence::Cut);

    m_copyAction = new QAction("&Copy", this);
    m_copyAction->setShortcut(QKeySequence::Copy);

    m_pasteAction = new QAction("&Paste", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);

    m_pageBreakAction = new QAction("Insert Page &Break", this);
    connect(m_pageBreakAction, &QAction::triggered, this, &ShoinFrame::onInsertPageBreak);

    m_newFolderAction = new QAction("New Folder", this);
    connect(m_newFolderAction, &QAction::triggered, this, &ShoinFrame::onNewFolder);

    m_newEntryAction = new QAction("New Entry", this);
    connect(m_newEntryAction, &QAction::triggered, this, &ShoinFrame::onNewEntry);
}

void ShoinFrame::createMenus() {}
void ShoinFrame::createToolBar() {}
void ShoinFrame::createStatusBar() {}

void ShoinFrame::onSave() {
    saveNow();
}

void ShoinFrame::onExit() {
    close();
}

void ShoinFrame::onAutoSave() {
    if (isMarkdownMode()) {
        if (!m_mdEdit || !m_mdEdit->document()->isModified())
            return;
        if (!hasNamedDocument())
            return;
        const QString dest = autosaveSidecarPath() + QStringLiteral(".md.txt");
        QFile f(dest);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            f.write(m_mdEdit->toPlainText().toUtf8());
            m_statusBar->showMessage("Auto-saved");
        }
        return;
    }
    if (!m_editor || !m_editor->isDirty())
        return;
    m_editor->fetchHtml([this](const QString &html) {
        if (!m_editor->isDirty())
            return;
        if (wouldClobberManuscript(html)) {
            m_statusBar->showMessage("Autosave skipped — empty or incomplete editor content");
            return;
        }
        const QString fileName = hasNamedDocument()
            ? autosaveSidecarPath()
            : (m_docsDir + "/.autosave/untitled.html");
        if (writeHtmlFile(fileName, html))
            m_statusBar->showMessage("Auto-saved");
    });
}

void ShoinFrame::onPrint() {
    QPrinter printer(QPrinter::HighResolution);
    const QString defName = QPrinterInfo::defaultPrinterName();
    if (!defName.isEmpty())
        printer.setPrinterName(defName);
    printer.setPageLayout(defaultPrintPageLayout());

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (isMarkdownMode()) {
        if (m_mdEdit)
            m_mdEdit->print(&printer);
        m_statusBar->showMessage("Printed");
        return;
    }

    const QPageLayout layout = pageLayoutFromPrinter(printer);
    const bool toFile = printer.outputFormat() == QPrinter::PdfFormat
                        || !printer.outputFileName().isEmpty();
    if (toFile) {
        QString fileName = printer.outputFileName();
        if (fileName.isEmpty())
            return;
        if (!fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
            fileName += QStringLiteral(".pdf");
        m_editor->webView()->page()->printToPdf(fileName, layout);
        m_statusBar->showMessage("Printing to file: " + fileName);
        return;
    }

    if (m_printTemp) {
        m_printTemp->deleteLater();
        m_printTemp = nullptr;
    }
    auto *tmp = new QTemporaryFile(QDir::tempPath() + QStringLiteral("/shoin-print-XXXXXX.pdf"), this);
    tmp->setAutoRemove(true);
    if (!tmp->open()) {
        delete tmp;
        QMessageBox::warning(this, "Print Failed", "Could not create a temporary PDF.");
        return;
    }
    const QString tmpPath = tmp->fileName();
    tmp->close();
    m_printTemp = tmp;
    m_pendingLpPdf = tmpPath;
    m_pendingLpPrinter = printer.printerName();
    m_pendingLpCopies = qMax(1, printer.copyCount());
    m_editor->webView()->page()->printToPdf(tmpPath, layout);
    m_statusBar->showMessage("Printing...");
}

void ShoinFrame::onPdfPrintingFinished(const QString &path, bool success)
{
    if (listenModeEnabled())
        listenLog("pdf_print", success ? QStringLiteral("success") : QStringLiteral("fail"));

    const bool cupsJob = !m_pendingLpPdf.isEmpty() && path == m_pendingLpPdf;
    if (cupsJob) {
        const QString pdf = m_pendingLpPdf;
        const QString printerName = m_pendingLpPrinter;
        const int copies = m_pendingLpCopies;
        m_pendingLpPdf.clear();
        m_pendingLpPrinter.clear();
        m_pendingLpCopies = 1;

        if (!success) {
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (!listenModeEnabled())
                QMessageBox::warning(this, "Print Failed",
                                     "Could not render the page for printing.");
            if (m_listenPrintPending)
                requestListenQuit();
            return;
        }

        const QStringList args = cupsLpArgv(printerName, copies, pdf);
        if (listenModeEnabled())
            listenLog("lp_argv", formatCommandLine(QStringLiteral("lp"), args));

        const bool allowLp = !listenModeEnabled() || listenPrintLpExecute();
        if (!allowLp) {
            listenLog("lp_dry_run", QStringLiteral("yes"));
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (m_listenPrintPending)
                requestListenQuit();
            return;
        }

        QProcess *lp = new QProcess(this);
        connect(lp, &QProcess::finished, this,
                [this, lp](int code, QProcess::ExitStatus st) {
            const QByteArray err = lp->readAllStandardError();
            lp->deleteLater();
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (code != 0 || st != QProcess::NormalExit) {
                if (!listenModeEnabled()) {
                    const QString msg = err.isEmpty()
                        ? QStringLiteral("lp failed")
                        : QString::fromLocal8Bit(err);
                    QMessageBox::warning(this, "Print Failed", msg);
                } else {
                    listenLog("lp_error", err.isEmpty() ? QStringLiteral("lp failed")
                                                        : QString::fromLocal8Bit(err));
                }
            } else {
                m_statusBar->showMessage("Sent to printer");
                if (listenModeEnabled())
                    listenLog("lp_sent", QStringLiteral("yes"));
            }
            if (m_listenPrintPending)
                requestListenQuit();
        });
        lp->start(QStringLiteral("lp"), args);
        if (!lp->waitForStarted(3000)) {
            if (!listenModeEnabled())
                QMessageBox::warning(this, "Print Failed", "Could not start lp.");
            else
                listenLog("lp_error", QStringLiteral("could not start lp"));
            lp->deleteLater();
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (m_listenPrintPending)
                requestListenQuit();
        }
        return;
    }

    if (!success) {
        if (!listenModeEnabled())
            QMessageBox::warning(this, "Print Failed",
                                 QStringLiteral("Could not write the print file:\n") + path);
    } else {
        m_statusBar->showMessage("Printed to: " + path);
    }

    if (m_listenPrintPending)
        requestListenQuit();
}

bool ShoinFrame::maybeStartListenPrint()
{
    if (listenPrintLpRequested()) {
        const QString def = QPrinterInfo::defaultPrinterName();
        const QStringList args = cupsLpArgv(def, 1, QStringLiteral("/tmp/shoin-print.pdf"));
        listenLog("lp_argv", formatCommandLine(QStringLiteral("lp"), args));
        listenLog("lp_dry_run", QStringLiteral("yes"));
    }

    const QString dest = listenPrintToFilePath();
    if (dest.isEmpty())
        return false;

    if (isMarkdownMode()) {
        if (!m_mdEdit)
            return false;
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(dest);
        printer.setPageLayout(defaultPrintPageLayout());
        m_mdEdit->print(&printer);
        const bool ok = QFileInfo(dest).exists() && QFileInfo(dest).size() > 0;
        listenLog("pdf_print", ok ? QStringLiteral("success") : QStringLiteral("fail"));
        return false;
    }
    if (!m_editor || !m_editor->webView() || !m_editor->webView()->page())
        return false;
    m_listenPrintPending = true;
    m_editor->webView()->page()->printToPdf(dest, defaultPrintPageLayout());
    m_statusBar->showMessage("Printing to file: " + dest);
    return true;
}


void ShoinFrame::onInsertPageBreak() {
    if (isMarkdownMode())
        return;
    m_editor->execCommand("insertHTML",
        "<div class=\"page-break\" style=\"page-break-after: always; border: none; border-top: 1px dashed #8B7355; margin: 30px 0;\"></div>");
}

void ShoinFrame::onBold() { if (!isMarkdownMode()) m_editor->execCommand("bold"); }
void ShoinFrame::onItalic() { if (!isMarkdownMode()) m_editor->execCommand("italic"); }
void ShoinFrame::onUnderline() { if (!isMarkdownMode()) m_editor->execCommand("underline"); }
void ShoinFrame::onStrikethrough() { if (!isMarkdownMode()) m_editor->execCommand("strikeThrough"); }
void ShoinFrame::onAlignLeft() { if (!isMarkdownMode()) m_editor->execCommand("justifyLeft"); }
void ShoinFrame::onAlignCenter() { if (!isMarkdownMode()) m_editor->execCommand("justifyCenter"); }
void ShoinFrame::onAlignRight() { if (!isMarkdownMode()) m_editor->execCommand("justifyRight"); }
void ShoinFrame::onJustify() { if (!isMarkdownMode()) m_editor->execCommand("justifyFull"); }
void ShoinFrame::onBulletList() { if (!isMarkdownMode()) m_editor->execCommand("insertUnorderedList"); }
void ShoinFrame::onNumberedList() { if (!isMarkdownMode()) m_editor->execCommand("insertOrderedList"); }
void ShoinFrame::onChecklist() { if (!isMarkdownMode()) m_editor->insertChecklist(); }

void ShoinFrame::onFontChanged(const QString &font) {
    if (!isMarkdownMode())
        m_editor->execCommand("fontName", font);
}


void ShoinFrame::onSelectionFontChanged(const QString &family, int pt)
{
    if (isMarkdownMode())
        return;
    if ((m_fontCombo && m_fontCombo->hasFocus()) || (m_sizeCombo && m_sizeCombo->hasFocus()))
        return;
    showFamilyInCombo(m_fontCombo, family);
    showSizeInCombo(m_sizeCombo, pt);
}

void ShoinFrame::dumpListenSelectionFont()
{
    if (!listenModeEnabled() || !m_editor || isMarkdownMode())
        return;

    auto family = std::make_shared<QString>();
    auto pt = std::make_shared<int>(0);
    auto caretFam = std::make_shared<QString>();
    auto caretPt = std::make_shared<int>(0);
    auto found = std::make_shared<bool>(false);
    auto done = std::make_shared<bool>(false);

    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < 4000) {
        *done = false;
        m_editor->requestHeadingFont([=](const QString &f, int p, const QString &cf, int cp, bool hit) {
            *family = f;
            *pt = p;
            *caretFam = cf;
            *caretPt = cp;
            *found = hit;
            *done = true;
        });
        QElapsedTimer wait;
        wait.start();
        while (!*done && wait.elapsed() < 400) {
            QEventLoop loop;
            QTimer::singleShot(40, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (*found && (!family->isEmpty() || !caretFam->isEmpty()))
            break;
        QEventLoop pause;
        QTimer::singleShot(80, &pause, &QEventLoop::quit);
        pause.exec();
    }

    const QString heading = family->isEmpty() ? *caretFam : *family;
    const int headingPt = *pt > 0 ? *pt : *caretPt;
    const QString caret = caretFam->isEmpty() ? *family : *caretFam;
    const int cpt = *caretPt > 0 ? *caretPt : *pt;
    if (!heading.isEmpty())
        listenLog("heading_font", heading);
    if (headingPt > 0)
        listenLog("heading_size", QString::number(headingPt));
    if (!caret.isEmpty())
        listenLog("caret_font", caret);
    if (cpt > 0)
        listenLog("caret_size", QString::number(cpt));
    onSelectionFontChanged(caret.isEmpty() ? heading : caret, cpt > 0 ? cpt : headingPt);
    if (m_fontCombo)
        listenLog("toolbar_font", m_fontCombo->currentText());
    if (m_sizeCombo)
        listenLog("toolbar_size", m_sizeCombo->currentText());
}


void ShoinFrame::onSizeChanged(const QString &size) {
    if (isMarkdownMode())
        return;
    bool ok = false;
    const int pt = size.toInt(&ok);
    if (!ok || pt <= 0)
        return;
    m_editor->applyFontSize(pt);
}

void ShoinFrame::updateWordCount() {
    if (isMarkdownMode()) {
        if (!m_wordCountLabel || !m_mdEdit)
            return;
        const QString t = m_mdEdit->toPlainText().trimmed();
        int count = 0;
        if (!t.isEmpty())
            count = t.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size();
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
        return;
    }
    m_editor->requestWordCount([this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
}

void ShoinFrame::closeEvent(QCloseEvent *event) {
    if (!confirmProceedIfDirty()) {
        event->ignore();
        return;
    }
    event->accept();
}

void ShoinFrame::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        const int border = 8;
        QRect rect = this->rect();
        QPoint pos = event->pos();

        if (pos.x() <= border && pos.y() <= border) {
            m_resizeDirection = TopLeft;
        } else if (pos.x() >= rect.width() - border && pos.y() <= border) {
            m_resizeDirection = TopRight;
        } else if (pos.x() <= border && pos.y() >= rect.height() - border) {
            m_resizeDirection = BottomLeft;
        } else if (pos.x() >= rect.width() - border && pos.y() >= rect.height() - border) {
            m_resizeDirection = BottomRight;
        } else if (pos.x() <= border) {
            m_resizeDirection = Left;
        } else if (pos.x() >= rect.width() - border) {
            m_resizeDirection = Right;
        } else if (pos.y() <= border) {
            m_resizeDirection = Top;
        } else if (pos.y() >= rect.height() - border) {
            m_resizeDirection = Bottom;
        } else {
            m_resizeDirection = None;
        }

        if (m_resizeDirection != None) {
            m_resizing = true;
            m_resizeStartPos = this->pos();
            m_resizeStartSize = size();
            m_resizeStartMousePos = event->globalPosition().toPoint();
            event->accept();
        } else if (m_titleBar->geometry().contains(event->pos())) {
            m_dragging = true;
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        } else {
            QWidget::mousePressEvent(event);
        }
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ShoinFrame::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    } else if (m_resizing) {
        QPoint delta = event->globalPosition().toPoint() - m_resizeStartMousePos;
        QPoint newPos = m_resizeStartPos;
        QSize newSize = m_resizeStartSize;

        switch (m_resizeDirection) {
            case Left:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                break;
            case Right:
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                break;
            case Top:
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case Bottom:
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            case TopLeft:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case TopRight:
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case BottomLeft:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            case BottomRight:
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            default:
                break;
        }

        setGeometry(QRect(newPos, newSize));
        event->accept();
    } else {
        const int border = 8;
        QRect rect = this->rect();
        QPoint pos = event->pos();

        if (pos.x() <= border && pos.y() <= border) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (pos.x() >= rect.width() - border && pos.y() <= border) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (pos.x() <= border && pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (pos.x() >= rect.width() - border && pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (pos.x() <= border || pos.x() >= rect.width() - border) {
            setCursor(Qt::SizeHorCursor);
        } else if (pos.y() <= border || pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        QWidget::mouseMoveEvent(event);
    }
}

void ShoinFrame::mouseReleaseEvent(QMouseEvent *event) {
    m_dragging = false;
    m_resizing = false;
    m_resizeDirection = None;
    QWidget::mouseReleaseEvent(event);
}

void ShoinFrame::mouseDoubleClickEvent(QMouseEvent *event) {
    if (m_titleBar->geometry().contains(event->pos())) {
        if (isMaximized()) {
            showNormal();
            setGeometry(m_normalGeometry);
        } else {
            m_normalGeometry = geometry();
            showMaximized();
        }
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void ShoinFrame::resizeEvent(QResizeEvent *event) {
    m_titleBar->setFixedWidth(width());
    QWidget::resizeEvent(event);
}

bool ShoinFrame::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint globalPos = mouseEvent->globalPosition().toPoint();
        QPoint localPos = mapFromGlobal(globalPos);

        const int border = 8;
        QRect rect = this->rect();

        if (localPos.x() >= 0 && localPos.x() < rect.width() &&
            localPos.y() >= 0 && localPos.y() < rect.height()) {
            if (localPos.x() <= border && localPos.y() <= border) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (localPos.x() >= rect.width() - border && localPos.y() <= border) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (localPos.x() <= border && localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (localPos.x() >= rect.width() - border && localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (localPos.x() <= border || localPos.x() >= rect.width() - border) {
                setCursor(Qt::SizeHorCursor);
            } else if (localPos.y() <= border || localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeVerCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ShoinFrame::onSaveAs() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", m_docsDir, kDocumentFilter, nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    fileName = ensureDocumentSuffix(fileName);
    m_currentFilePath = fileName;
    if (saveNow())
        updateTitleBar();
}

void ShoinFrame::onNewFolder() {
    QModelIndex index = m_treeView->currentIndex();
    QString path = m_fileModel->filePath(index);
    if (path.isEmpty() || !QFileInfo(path).isDir()) {
        path = m_docsDir;
    }
    QString folderName = QInputDialog::getText(this, "New Folder", "Folder name:");
    if (folderName.isEmpty()) return;
    QDir dir(path);
    if (dir.mkdir(folderName)) {
        m_statusBar->showMessage("Folder created: " + folderName);
    } else {
        QMessageBox::warning(this, "Error", "Could not create folder.");
    }
}

void ShoinFrame::onNewEntry() {
    if (!confirmProceedIfDirty())
        return;

    QModelIndex index = m_treeView->currentIndex();
    QString path = m_fileModel->filePath(index);
    if (path.isEmpty() || !QFileInfo(path).isDir()) {
        path = m_docsDir;
    }
    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm");
    QString fileName = dateTime + ".html";
    QString fullPath = QDir(path).absoluteFilePath(fileName);
    const QString starter = QStringLiteral("<p></p>");
    if (writeHtmlFile(fullPath, starter)) {
        m_currentFilePath = fullPath;
        setMarkdownMode(false);
        m_editor->setHtml(starter);
        m_editor->markClean();
        m_titleEdit->setText(dateTime);
        m_titleEdit->setEnabled(true);
        updateTitleBar();
        m_statusBar->showMessage("Entry created: " + fileName);
        updateWordCount();
    } else {
        QMessageBox::warning(this, "Error", "Could not create entry.");
    }
}

void ShoinFrame::onTreeDoubleClicked(const QModelIndex &index) {
    const QString filePath = m_fileModel->filePath(index);
    if (QFileInfo(filePath).isDir()) {
        m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
        return;
    }
    if (filePath == m_currentFilePath)
        return;
    if (!confirmProceedIfDirty()) {
        restoreTreeSelection();
        return;
    }
    openPath(filePath);
}

void ShoinFrame::onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (m_suppressTreeLoad)
        return;
    if (selected.indexes().isEmpty()) {
        m_titleEdit->clear();
        m_titleEdit->setEnabled(false);
        return;
    }
    QModelIndex index = selected.indexes().first();
    QString filePath = m_fileModel->filePath(index);
    if (QFileInfo(filePath).isFile()) {
        if (filePath == m_currentFilePath)
            return;
        if (!confirmProceedIfDirty()) {
            restoreTreeSelection();
            return;
        }
        openPath(filePath);
    } else {
        m_titleEdit->clear();
        m_titleEdit->setEnabled(false);
        m_statusBar->showMessage("Folder selected");
    }
}

void ShoinFrame::onTreeContextMenu(const QPoint &pos) {
    QModelIndex index = m_treeView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    menu.addAction(m_newFolderAction);
    menu.addAction(m_newEntryAction);
    menu.addSeparator();
    QAction *renameAction = menu.addAction("Rename");
    connect(renameAction, &QAction::triggered, [this, index]() {
        m_treeView->edit(index);
    });
    QAction *deleteAction = menu.addAction("Delete");
    connect(deleteAction, &QAction::triggered, [this, index]() {
        QString filePath = m_fileModel->filePath(index);
        QFileInfo info(filePath);
        if (info.isDir()) {
            QDir dir(filePath);
            if (dir.removeRecursively()) {
                m_statusBar->showMessage("Folder deleted: " + filePath);
            } else {
                QMessageBox::warning(this, "Error", "Could not delete folder.");
            }
        } else {
            if (QFile::remove(filePath)) {
                if (filePath == m_currentFilePath) {
                    m_currentFilePath.clear();
                    setMarkdownMode(false);
                    m_editor->setHtml("<p></p>");
                    m_editor->markClean();
                    if (m_mdEdit) {
                        QSignalBlocker block(m_mdEdit);
                        m_mdEdit->clear();
                        m_mdEdit->document()->setModified(false);
                    }
                    updateTitleBar();
                }
                m_statusBar->showMessage("File deleted: " + filePath);
            } else {
                QMessageBox::warning(this, "Error", "Could not delete file.");
            }
        }
    });
    menu.exec(m_treeView->mapToGlobal(pos));
}

void ShoinFrame::onTreeClicked(const QModelIndex &index) {
    if (m_fileModel->isDir(index))
        m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
}

bool ShoinFrame::openHtmlFile(const QString &filePath) {
    return openPath(filePath);
}

void ShoinFrame::restoreTreeSelection() {
    m_suppressTreeLoad = true;
    if (m_currentFilePath.isEmpty()) {
        m_treeView->clearSelection();
    } else {
        const QModelIndex idx = m_fileModel->index(m_currentFilePath);
        if (idx.isValid())
            m_treeView->setCurrentIndex(idx);
    }
    m_suppressTreeLoad = false;
}

bool ShoinFrame::hasNamedDocument() const {
    return !m_currentFilePath.isEmpty() && !m_currentFilePath.contains("Shoin_AutoSave.html");
}

QString ShoinFrame::documentDisplayName() const {
    if (!hasNamedDocument())
        return QStringLiteral("Untitled");
    return QFileInfo(m_currentFilePath).fileName();
}

QString ShoinFrame::autosaveSidecarPath() const {
    const QFileInfo info(m_currentFilePath);
    QString rel = info.absoluteFilePath();
    if (rel.startsWith(m_docsDir))
        rel = rel.mid(m_docsDir.size());
    while (rel.startsWith('/'))
        rel.remove(0, 1);
    rel.replace('/', "__");
    QDir().mkpath(m_docsDir + "/.autosave");
    return m_docsDir + "/.autosave/" + rel;
}

void ShoinFrame::updateTitleBar() {
    const bool dirty = documentIsDirty();
    QString title = QStringLiteral("Shoin — ") + documentDisplayName();
    if (dirty)
        title += QStringLiteral(" *");
    if (m_titleLabel)
        m_titleLabel->setText(title);
    setWindowTitle(title);
}

bool ShoinFrame::htmlLooksEmpty(const QString &html) const {
    QString t = html;
    t.replace(QRegularExpression("<[^>]+>"), QStringLiteral(" "));
    t.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    t.replace(QStringLiteral("&#160;"), QStringLiteral(" "));
    return t.trimmed().isEmpty();
}

bool ShoinFrame::wouldClobberManuscript(const QString &incoming) const {
    if (htmlLooksEmpty(incoming))
        return true;
    const QString lastGood = m_editor ? m_editor->lastGoodHtml() : QString();
    if (lastGood.size() > 200 && incoming.trimmed().size() * 10 < lastGood.size())
        return true;
    return false;
}

bool ShoinFrame::writeHtmlFile(const QString &path, const QString &html) {
    QString err;
    if (!DocumentIo::writeFromHtml(path, html, &err)) {
        if (listenModeEnabled())
            listenLog("save_error", err.isEmpty() ? QStringLiteral("write failed") : err);
        else
            QMessageBox::warning(this, "Save Failed",
                                 "Could not write:\n" + path + "\n" + err);
        return false;
    }
    return true;
}

bool ShoinFrame::persistDocument(const QString &path, const QString &html, bool markCleanAfter) {
    QString body = html;
    if (htmlLooksEmpty(body) && m_editor && !isMarkdownMode() && !htmlLooksEmpty(m_editor->lastGoodHtml()))
        body = m_editor->lastGoodHtml();
    if (wouldClobberManuscript(body)) {
        if (listenModeEnabled())
            listenLog("save_error", QStringLiteral("wouldClobber empty or incomplete html"));
        m_statusBar->showMessage("Save skipped — empty or incomplete editor content. Last good copy kept.");
        return false;
    }
    if (!writeHtmlFile(path, body))
        return false;
    m_statusBar->showMessage("Saved to " + path);
    if (markCleanAfter) {
        m_editor->markClean();
        updateTitleBar();
    }
    return true;
}

bool ShoinFrame::ensureSavePath() {
    if (hasNamedDocument())
        return true;
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", m_docsDir, kDocumentFilter, nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return false;
    fileName = ensureDocumentSuffix(fileName);
    m_currentFilePath = fileName;
    updateTitleBar();
    return true;
}

QString ShoinFrame::waitForEditorHtml()
{
    QString captured;
    if (!m_editor)
        return captured;

    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < 8000) {
        auto done = std::make_shared<bool>(false);
        m_editor->fetchHtml([&](const QString &html) {
            captured = html;
            *done = true;
        });
        if (!*done) {
            QEventLoop loop;
            QTimer pump;
            pump.setInterval(15);
            QObject::connect(&pump, &QTimer::timeout, [&]() {
                if (*done)
                    loop.quit();
            });
            pump.start();
            QTimer::singleShot(400, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (!htmlLooksEmpty(captured))
            return captured;
        QEventLoop pause;
        QTimer::singleShot(80, &pause, &QEventLoop::quit);
        pause.exec();
    }
    if (htmlLooksEmpty(captured))
        captured = m_editor->lastGoodHtml();
    return captured;
}

bool ShoinFrame::saveNow() {
    if (!ensureSavePath())
        return false;

    const bool destSource = isSourceDocumentPath(m_currentFilePath);
    if (isMarkdownMode() && destSource)
        return saveMarkdownNow();

    QString html;
    if (isMarkdownMode())
        html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
    else
        html = waitForEditorHtml();
    return persistDocument(m_currentFilePath, html, true);
}

bool ShoinFrame::confirmProceedIfDirty() {
    const bool dirty = documentIsDirty();
    if (!dirty)
        return true;

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Unsaved Changes",
        "Document has unsaved changes. Save before continuing?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return false;
    if (reply == QMessageBox::Yes)
        return saveNow();
    return true;
}

void ShoinFrame::applyUiFont(const Theme &theme)
{
    QFont ui;
    if (theme.themeId == ThemeId::WordPerfect)
        ui.setFamilies({"IBM Plex Mono", "Fixed", "Courier New", "DejaVu Sans Mono", "sans-serif"});
    else if (theme.themeId == ThemeId::Leather)
        ui.setFamilies({"Noto Serif", "Georgia", "serif"});
    else
        ui.setFamilies({"Courier New", "Liberation Mono", "DejaVu Sans Mono", "monospace"});
    ui.setPointSize(10);
    setFont(ui);
    if (m_titleLabel) {
        QFont title = ui;
        title.setPointSize(10);
        title.setBold(true);
        m_titleLabel->setFont(title);
    }
    if (m_statusBar) {
        QFont status = ui;
        status.setPointSize(8);
        m_statusBar->setFont(status);
    }
    if (m_wordCountLabel)
        m_wordCountLabel->setFont(m_statusBar ? m_statusBar->font() : ui);
}

void ShoinFrame::colorizeToolbarIcons(const Theme &theme)
{
    const QColor tint(theme.accent);
    auto tinted = [tint](const QString &path) {
        QPixmap src(path);
        if (src.isNull())
            return QIcon();
        QPixmap dest(src.size());
        dest.fill(Qt::transparent);
        QPainter p(&dest);
        p.drawPixmap(0, 0, src);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(dest.rect(), tint);
        p.end();
        return QIcon(dest);
    };

    m_newAction->setIcon(tinted(QStringLiteral(":/icons/new.png")));
    m_openAction->setIcon(tinted(QStringLiteral(":/icons/open.png")));
    m_saveAction->setIcon(tinted(QStringLiteral(":/icons/save.png")));
    m_boldAction->setIcon(tinted(QStringLiteral(":/icons/bold.png")));
    m_italicAction->setIcon(tinted(QStringLiteral(":/icons/italic.png")));
    m_underlineAction->setIcon(tinted(QStringLiteral(":/icons/underline.png")));
    m_strikethroughAction->setIcon(tinted(QStringLiteral(":/icons/strikethru.png")));
    m_alignLeftAction->setIcon(tinted(QStringLiteral(":/icons/alignleft.png")));
    m_alignCenterAction->setIcon(tinted(QStringLiteral(":/icons/aligncenter.png")));
    m_alignRightAction->setIcon(tinted(QStringLiteral(":/icons/alignright.png")));
    m_justifyAction->setIcon(tinted(QStringLiteral(":/icons/justify.png")));
    m_bulletAction->setIcon(tinted(QStringLiteral(":/icons/bullet.png")));
    m_numberAction->setIcon(tinted(QStringLiteral(":/icons/numbered.png")));
    m_checklistAction->setIcon(tinted(QStringLiteral(":/icons/checklist.png")));
}

void ShoinFrame::applyTheme(ThemeId id)
{
    const Theme t = themeForId(id);
    m_currentTheme = t;
    applyUiFont(t);

    const QString chromeFg = t.pageAsObject ? t.pageBg : t.textOnChrome;
    const QString btnCss = QStringLiteral("border: none; background: transparent; color: %1;").arg(t.textOnChrome);

    setStyleSheet(QStringLiteral("QWidget { background-color: %1; }").arg(t.chromeBg));

    if (m_titleBar)
        m_titleBar->setStyleSheet(QStringLiteral("background-color: %1;").arg(t.chromeBg));
    if (m_titleLabel)
        m_titleLabel->setStyleSheet(QStringLiteral("color: %1;").arg(t.textOnChrome));
    if (m_minBtn)
        m_minBtn->setStyleSheet(btnCss);
    if (m_maxBtn)
        m_maxBtn->setStyleSheet(btnCss);
    if (m_closeBtn)
        m_closeBtn->setStyleSheet(btnCss);

    if (m_menuBar) {
        m_menuBar->setStyleSheet(QStringLiteral(
            "QMenuBar { background-color: %1; color: %2; }"
            "QMenuBar::item { background-color: transparent; color: %2; padding: 4px 8px; }"
            "QMenuBar::item:selected { background-color: %3; color: %4; }"
            "QMenu { background-color: %5; color: %6; border: 1px solid %7; }"
            "QMenu::item { background-color: transparent; color: %6; }"
            "QMenu::item:selected { background-color: %3; color: %4; }"
            "QMenu::separator { height: 1px; background: %7; }")
            .arg(t.menuBarBg, t.menuBarText, t.menuSelectedBg, t.menuSelectedFg,
                 t.themeId == ThemeId::WordPerfect ? t.menuBarBg : t.chromeBg,
                 t.themeId == ThemeId::WordPerfect ? t.menuBarText : t.textOnChrome,
                 t.chromeLo));
    }

    if (m_toolBar) {
        m_toolBar->setStyleSheet(QStringLiteral(
            "QToolBar {"
            "  background-color: %1;"
            "  border-left: 8px solid %2;"
            "  border-right: 8px solid %2;"
            "  border-top: 0;"
            "  border-bottom: 0;"
            "  padding: 4px 0;"
            "}"
            "QToolButton { background-color: transparent; border: none; padding: 2px; }"
            "QToolButton:hover { background-color: %3; border-radius: 2px; }"
            "QToolButton:pressed { background-color: %4; }"
            "QComboBox { background-color: %4; color: %5; border: 1px solid %3; border-radius: 2px; padding: 2px; min-width: 60px; }"
            "QComboBox:hover { background-color: %3; }"
            "QComboBox::drop-down { border: none; background-color: %4; }"
            "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 4px solid %5; margin-right: 4px; }"
            "QComboBox QAbstractItemView { background-color: %2; color: %5; border: 1px solid %4; selection-background-color: %3; selection-color: %5; }")
            .arg(t.chromeMid, t.chromeBg, t.chromeHi, t.chromeLo, chromeFg));
    }

    if (m_statusBar)
        m_statusBar->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(t.chromeBg, t.textOnChrome));
    if (m_wordCountLabel)
        m_wordCountLabel->setStyleSheet(QStringLiteral("color: %1;").arg(t.textOnChrome));

    const QString treeFg = t.pageAsObject ? t.pageBg : t.textOnChrome;
    if (m_treeView) {
        m_treeView->setStyleSheet(QStringLiteral(
            "QTreeView, QTreeView::item, QTreeView::branch {"
            "  color: %1;"
            "  background-color: %2;"
            "  selection-background-color: %3;"
            "  selection-color: %4;"
            "}"
            "QTreeView::item:selected, QTreeView::branch:selected {"
            "  background-color: %3;"
            "  color: %4;"
            "}"
            "QTreeView QLineEdit {"
            "  color: %1;"
            "  background-color: %5;"
            "  selection-background-color: %3;"
            "  selection-color: %4;"
            "}")
            .arg(treeFg, t.chromeBg, t.chromeHi, t.pageAsObject ? t.pageBg : t.accent, t.chromeLo));
    }
    if (m_titleEdit) {
        m_titleEdit->setStyleSheet(QStringLiteral(
            "QLineEdit {"
            "  color: %1;"
            "  background-color: %2;"
            "  selection-background-color: %3;"
            "  selection-color: %4;"
            "  border: 1px solid %5;"
            "  padding: 3px;"
            "}")
            .arg(treeFg, t.chromeLo, t.chromeHi, t.pageAsObject ? t.pageBg : t.accent, t.chromeHi));
    }

    const QString menuBg = (t.themeId == ThemeId::WordPerfect) ? t.menuBarBg : t.chromeMid;
    const QString menuFg = (t.themeId == ThemeId::WordPerfect) ? t.menuBarText : t.textOnChrome;
    QString dialogCss = QStringLiteral(
        "QMessageBox { background-color: __BG__; color: __FG__; }"
        "QMessageBox QLabel { color: __FG__; font-weight: bold; }"
        "QMessageBox QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 5px; }"
        "QMessageBox QPushButton:hover { background-color: __HI__; }"
        "QFileDialog { background-color: __BG__; color: __FG__; }"
        "QFileDialog QLabel, QFileDialog QLineEdit, QFileDialog QTreeView, QFileDialog QListView, QFileDialog QComboBox, QFileDialog QHeaderView::section { color: __FG__; background-color: __BG__; }"
        "QFileDialog QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 4px 8px; }"
        "QFileDialog QPushButton:hover { background-color: __HI__; }"
        "QMenu { background-color: __MENUBG__; color: __MENUFG__; border: 1px solid __BG__; }"
        "QMenu::item:selected { background-color: __SELBG__; color: __SELFG__; }"
        "QDialog { background-color: __BG__; color: __FG__; }"
        "QDialog QLabel { color: __FG__; }"
        "QLineEdit { background-color: __LO__; color: __CHROMEFG__; border: 1px solid __HI__; padding: 4px; }"
        "QDialog QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 5px; }"
        "QInputDialog, QDialog { background-color: __BG__; color: __FG__; }"
        "QInputDialog QLabel { color: __FG__; }"
        "QInputDialog QLineEdit { color: __CHROMEFG__; background-color: __LO__; }"
        "QInputDialog QPushButton { background-color: __MID__; color: __FG__; }");
    dialogCss.replace(QStringLiteral("__BG__"), t.chromeBg);
    dialogCss.replace(QStringLiteral("__FG__"), t.textOnChrome);
    dialogCss.replace(QStringLiteral("__MID__"), t.chromeMid);
    dialogCss.replace(QStringLiteral("__HI__"), t.chromeHi);
    dialogCss.replace(QStringLiteral("__LO__"), t.chromeLo);
    dialogCss.replace(QStringLiteral("__MENUBG__"), menuBg);
    dialogCss.replace(QStringLiteral("__MENUFG__"), menuFg);
    dialogCss.replace(QStringLiteral("__SELBG__"), t.menuSelectedBg);
    dialogCss.replace(QStringLiteral("__SELFG__"), t.menuSelectedFg);
    dialogCss.replace(QStringLiteral("__CHROMEFG__"), chromeFg);
    qApp->setStyleSheet(dialogCss);

    colorizeToolbarIcons(t);

    if (m_themeGroup) {
        const QSignalBlocker blocker(m_themeGroup);
        for (QAction *action : m_themeGroup->actions())
            action->setChecked(action->data().toString() == t.id);
    }

    if (m_editor)
        m_editor->applyTheme(t);

    QSettings settings(QStringLiteral("Shoin"), QStringLiteral("Shoin"));
    settings.setValue(QStringLiteral("theme"), t.id);
}

QIcon ShoinFrame::createToolbarIcon(const QString &symbol) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setPen(QColor(40, 40, 40));
    p.setFont(QFont("Noto Sans", 11, QFont::Bold));
    p.drawText(pix.rect(), Qt::AlignCenter, symbol);
    return QIcon(pix);
}

bool ShoinFrame::isMarkdownSourcePath(const QString &path) const
{
    return isSourceDocumentPath(path);
}

bool ShoinFrame::isMarkdownMode() const
{
    return m_markdownMode;
}

void ShoinFrame::setFormatActionsEnabled(bool on)
{
    const QList<QAction *> acts = {
        m_boldAction, m_italicAction, m_underlineAction, m_strikethroughAction,
        m_alignLeftAction, m_alignCenterAction, m_alignRightAction, m_justifyAction,
        m_bulletAction, m_numberAction, m_checklistAction, m_pageBreakAction
    };
    for (QAction *a : acts) {
        if (a)
            a->setEnabled(on);
    }
    if (m_fontCombo)
        m_fontCombo->setEnabled(on);
    if (m_sizeCombo)
        m_sizeCombo->setEnabled(on);
    if (!on && m_mdEdit) {
        const QFont f = m_mdEdit->font();
        showFamilyInCombo(m_fontCombo, f.family());
        showSizeInCombo(m_sizeCombo, f.pointSize());
    }
}

void ShoinFrame::setMarkdownMode(bool on)
{
    m_markdownMode = on;
    if (m_editorStack && m_mdEdit && m_editor)
        m_editorStack->setCurrentWidget(on ? static_cast<QWidget *>(m_mdEdit)
                                          : static_cast<QWidget *>(m_editor));
    setFormatActionsEnabled(!on);
}

bool ShoinFrame::documentIsDirty() const
{
    if (isMarkdownMode())
        return m_mdEdit && m_mdEdit->document()->isModified();
    if (!m_editor)
        return false;
    return m_editor->isDirty() || m_editor->queryDirtyNow();
}

bool ShoinFrame::openPath(const QString &path)
{
    if (path.isEmpty())
        return false;
    const QString abs = QFileInfo(path).absoluteFilePath();
    bool ok = false;
    if (isSourceDocumentPath(abs))
        ok = loadMarkdownDocument(abs);
    else
        ok = loadHtmlDocument(abs);
    if (ok && listenModeEnabled()) {
        QTimer::singleShot(400, this, [this]() {
            emitListenHealth(m_currentFilePath);
            if (!maybeStartListenPrint())
                requestListenQuit();
        });
    }
    return ok;
}

bool ShoinFrame::loadHtmlDocument(const QString &path)
{
    QString err;
    const QString html = DocumentIo::htmlFromFile(path, &err);
    if (!err.isEmpty()) {
        if (listenModeEnabled())
            listenLog("open_error", err);
        else
            QMessageBox::warning(this, "Open Failed",
                                 "Could not read:\n" + path + "\n" + err);
        return false;
    }
    setMarkdownMode(false);
    m_editor->setHtml(html);
    m_currentFilePath = path;
    m_editor->markClean();
    if (m_titleEdit) {
        m_titleEdit->setText(QFileInfo(path).baseName());
        m_titleEdit->setEnabled(true);
    }
    updateTitleBar();
    m_statusBar->showMessage("File opened: " + path);
    updateWordCount();
    return true;
}

bool ShoinFrame::loadMarkdownDocument(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Failed",
                             "Could not read:\n" + path);
        return false;
    }
    const QString text = QString::fromUtf8(file.readAll());
    file.close();
    setMarkdownMode(true);
    {
        QSignalBlocker block(m_mdEdit);
        m_mdEdit->setPlainText(text);
    }
    m_mdEdit->document()->setModified(false);
    m_currentFilePath = path;
    if (m_titleEdit) {
        m_titleEdit->setText(QFileInfo(path).baseName());
        m_titleEdit->setEnabled(true);
    }
    updateTitleBar();
    m_statusBar->showMessage("File opened: " + path);
    updateWordCount();
    return true;
}

bool ShoinFrame::saveMarkdownNow()
{
    if (!ensureSavePath())
        return false;
    if (!isSourceDocumentPath(m_currentFilePath)) {
        const QString html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
        return persistDocument(m_currentFilePath, html, true);
    }
    if (!m_mdEdit) {
        m_statusBar->showMessage("No markdown buffer to save");
        return false;
    }
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write:\n" + m_currentFilePath);
        return false;
    }
    const QByteArray bytes = m_mdEdit->toPlainText().toUtf8();
    if (file.write(bytes) != bytes.size()) {
        QMessageBox::warning(this, "Save Failed",
                             "Short write:\n" + m_currentFilePath);
        return false;
    }
    m_mdEdit->document()->setModified(false);
    updateTitleBar();
    m_statusBar->showMessage("Saved to " + m_currentFilePath);
    return true;
}


void ShoinFrame::onLedgerCheckRequested(const QString &id)
{
    static const QRegularExpression idRe(QStringLiteral("^[A-Za-z0-9_-]+$"));
    static QSet<QString> stampedThisSession;
    const QString cleanId = id.trimmed();
    if (cleanId.isEmpty() || !idRe.match(cleanId).hasMatch()) {
        qWarning("ledger check: rejected id");
        return;
    }
    if (stampedThisSession.contains(cleanId)) {
        if (m_statusBar)
            m_statusBar->showMessage(QStringLiteral("Done: %1 (already stamped)").arg(cleanId), 2000);
        return;
    }

    const QString name = QFileInfo(m_currentFilePath).fileName();
    static const QRegularExpression dailyRe(
        QStringLiteral("^(\\d{4}-\\d{2}-\\d{2})\\s+Daily\\.html$"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = dailyRe.match(name);
    if (!m.hasMatch()) {
        qWarning("ledger check: current file is not a dated Daily.html (%s)",
                 qPrintable(name));
        // Quiet: non-Daily ticks (WEEKEND etc.) — no statusBar spam
        return;
    }
    const QString date = m.captured(1);

    auto *proc = new QProcess(this);
    const QStringList args{
        QStringLiteral("/home/franklin/.hermes/scripts/ledger"),
        QStringLiteral("check"),
        cleanId,
        QStringLiteral("--date"),
        date,
        QStringLiteral("--note"),
        QStringLiteral("Shoin checkbox"),
    };
    QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [this, proc, cleanId](int code, QProcess::ExitStatus status) {
        const QByteArray err = proc->readAllStandardError();
        if (status != QProcess::NormalExit || code != 0) {
            qWarning("ledger check failed for %s (exit %d): %s",
                     qPrintable(cleanId), code, err.constData());
            if (m_statusBar) {
                const QString detail = QString::fromUtf8(err).trimmed();
                m_statusBar->showMessage(
                    QStringLiteral("Ledger fail %1%2")
                        .arg(cleanId)
                        .arg(detail.isEmpty() ? QString() : QStringLiteral(": ") + detail.left(80)),
                    5000);
            }
        } else {
            stampedThisSession.insert(cleanId);
            if (m_statusBar)
                m_statusBar->showMessage(QStringLiteral("Done: %1").arg(cleanId), 2500);
        }
        proc->deleteLater();
    });
    QObject::connect(proc, &QProcess::errorOccurred, this, [this, proc, cleanId](QProcess::ProcessError) {
        qWarning("ledger check process error for %s: %s",
                 qPrintable(cleanId), qPrintable(proc->errorString()));
        if (m_statusBar) {
            m_statusBar->showMessage(
                QStringLiteral("Ledger fail %1: %2").arg(cleanId, proc->errorString()),
                5000);
        }
        proc->deleteLater();
    });
    proc->start(QStringLiteral("python3"), args);
}

void ShoinFrame::emitListenHealth(const QString &openedPath)
{
    if (!listenModeEnabled())
        return;
    if (!openedPath.isEmpty())
        listenLog("opened", openedPath);

    const bool docx = isDocxPath(openedPath) || isDocxPath(m_currentFilePath);
    bool healthOk = true;
    if (isMarkdownMode() && m_mdEdit) {
        const QString buf = m_mdEdit->toPlainText();
        listenLog("mode", QStringLiteral("markdown-source"));
        listenLog("has_hash", buf.contains(QLatin1Char('#')) ? QStringLiteral("yes") : QStringLiteral("no"));
        listenLog("has_pipe", buf.contains(QLatin1Char('|')) ? QStringLiteral("yes") : QStringLiteral("no"));
        listenLog("html_preview", QStringLiteral("no"));
    } else if (!openedPath.isEmpty() || docx) {
        listenLog("mode", docx ? QStringLiteral("html-docx") : QStringLiteral("html"));
        listenLog("html_preview", QStringLiteral("yes"));
        if (docx) {
            QString herr;
            const bool ooxml = DocumentIo::docxLooksHealthy(openedPath.isEmpty() ? m_currentFilePath : openedPath, &herr);
            listenLog("ooxml", ooxml ? QStringLiteral("yes") : QStringLiteral("no"));
            if (!ooxml)
                healthOk = false;
        }
    }

    const QString dest = listenSaveAsPath();
    if (!dest.isEmpty()) {
        QString html;
        if (isMarkdownMode())
            html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
        else
            html = waitForEditorHtml();
        {
            const QString probe = html.isEmpty() && m_editor ? m_editor->lastGoodHtml() : html;
            listenLog("table_count", QString::number(probe.toLower().count(QStringLiteral("<table"))));
            const bool fontSpan = probe.contains(QLatin1String("font-family"), Qt::CaseInsensitive)
                || probe.contains(QLatin1String("font-size"), Qt::CaseInsensitive);
            listenLog("has_font_span", fontSpan ? QStringLiteral("yes") : QStringLiteral("no"));
        }
        if (htmlLooksEmpty(html)) {
            listenLog("save_error", QStringLiteral("empty editor html"));
            healthOk = false;
        } else {
            QString err;
            if (!DocumentIo::writeFromHtml(dest, html, &err)) {
                listenLog("save_error", err.isEmpty() ? QStringLiteral("writeFromHtml failed") : err);
                healthOk = false;
            } else {
                listenLog("saved", QFileInfo(dest).absoluteFilePath());
            }
        }
        if (isDocxPath(dest)) {
            listenLogDocxPeek(dest);
            QString herr;
            if (!DocumentIo::docxLooksHealthy(dest, &herr))
                healthOk = false;
        }
    } else {
        const QString probe = m_editor ? m_editor->lastGoodHtml() : QString();
        listenLog("table_count", QString::number(probe.toLower().count(QStringLiteral("<table"))));
        const bool fontSpan = probe.contains(QLatin1String("font-family"), Qt::CaseInsensitive)
            || probe.contains(QLatin1String("font-size"), Qt::CaseInsensitive);
        listenLog("has_font_span", fontSpan ? QStringLiteral("yes") : QStringLiteral("no"));
        if (docx)
            listenLogDocxPeek(openedPath);
    }

    dumpListenSelectionFont();

    listenLogPrinters();

    listenLog("still_running", QStringLiteral("yes"));
    listenLog("health", healthOk ? QStringLiteral("ok") : QStringLiteral("fail"));
}

void ShoinFrame::requestListenQuit()
{
    if (!listenModeEnabled() || m_listenQuitArmed)
        return;
    m_listenQuitArmed = true;
    QTimer::singleShot(800, qApp, []() {
        QCoreApplication::quit();
    });
}
