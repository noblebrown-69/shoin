#ifndef SHOINFRAME_H
#define SHOINFRAME_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QItemSelection>
#include "Theme.h"

extern const char *folder_xpm[];

class SimpleIconProvider : public QFileIconProvider {
public:
    QIcon icon(const QFileInfo &info) const override {
        if (info.isDir()) return QIcon(QPixmap(folder_xpm));
        return QIcon::fromTheme("text-x-generic", QIcon(":/icons/file.png"));
    }
};

class MonasteryEditor;
class QPlainTextEdit;
class QStackedWidget;
class QFontComboBox;
class QComboBox;
class QTemporaryFile;

class ShoinFrame : public QWidget {
    Q_OBJECT

public:
    ShoinFrame(QWidget *parent = nullptr);
    ~ShoinFrame();
    static QString getRealAppDir();
    bool openPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSave();
    void onSaveAs();
    void onExit();
    void onAutoSave();
    void onBold();
    void onItalic();
    void onUnderline();
    void onStrikethrough();
    void onAlignLeft();
    void onAlignCenter();
    void onAlignRight();
    void onJustify();
    void onBulletList();
    void onNumberedList();
    void onChecklist();
    void onFontChanged(const QString &font);
    void onSizeChanged(const QString &size);
    void onSelectionFontChanged(const QString &family, int pt);
    void onPrint();
    void onInsertPageBreak();
    void updateWordCount();
    void onNewFolder();
    void onNewEntry();
    void onTreeDoubleClicked(const QModelIndex &index);
    void onTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTreeContextMenu(const QPoint &pos);
    void onTreeClicked(const QModelIndex &index);
    void updateTitleBar();
    void applyTheme(ThemeId id);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDocsFolder();
    QIcon createToolbarIcon(const QString &symbol);
    void colorizeToolbarIcons(const Theme &theme);
    void applyUiFont(const Theme &theme);

    bool confirmProceedIfDirty();
    bool hasNamedDocument() const;
    QString documentDisplayName() const;
    QString autosaveSidecarPath() const;
    bool htmlLooksEmpty(const QString &html) const;
    bool wouldClobberManuscript(const QString &incoming) const;
    bool writeHtmlFile(const QString &path, const QString &html);
    bool persistDocument(const QString &path, const QString &html, bool markCleanAfter);
    bool ensureSavePath();
    bool saveNow();
    bool saveMarkdownNow();
    QString waitForEditorHtml();
    bool openHtmlFile(const QString &filePath);
    bool loadHtmlDocument(const QString &path);
    bool loadMarkdownDocument(const QString &path);
    bool isMarkdownSourcePath(const QString &path) const;
    bool isMarkdownMode() const;
    void setMarkdownMode(bool on);
    void setFormatActionsEnabled(bool on);
    void emitListenHealth(const QString &openedPath);
    void dumpListenSelectionFont();
    void requestListenQuit();
    void onPdfPrintingFinished(const QString &path, bool success);
    bool maybeStartListenPrint();
    void onLedgerCheckRequested(const QString &id);
    bool documentIsDirty() const;
    void restoreTreeSelection();
    QString themeMenuName(const Theme &th) const;

    MonasteryEditor *m_editor = nullptr;
    QStackedWidget *m_editorStack = nullptr;
    QPlainTextEdit *m_mdEdit = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
    QTimer *m_wordCountPollTimer = nullptr;
    QString m_docsDir;
    QString m_currentFilePath;
    QRect m_normalGeometry;
    bool m_suppressTreeLoad = false;
    bool m_didOfferRestore = false;
    bool m_markdownMode = false;
    bool m_listenQuitArmed = false;
    bool m_listenPrintPending = false;
    QTemporaryFile *m_printTemp = nullptr;
    QString m_pendingLpPdf;
    QString m_pendingLpPrinter;
    int m_pendingLpCopies = 1;

    QWidget *m_titleBar = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_maxBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QMenuBar *m_menuBar = nullptr;
    QToolBar *m_toolBar = nullptr;
    QFontComboBox *m_fontCombo = nullptr;
    QComboBox *m_sizeCombo = nullptr;
    QStatusBar *m_statusBar = nullptr;
    QActionGroup *m_themeGroup = nullptr;
    Theme m_currentTheme;

    QPoint m_dragPosition;
    bool m_dragging = false;

    bool m_resizing;
    QPoint m_resizeStartPos;
    QPoint m_resizeStartMousePos;
    QSize m_resizeStartSize;
    enum ResizeDirection { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeDirection m_resizeDirection;

    QAction *m_newAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_saveAsAction = nullptr;
    QAction *m_printAction = nullptr;
    QAction *m_exitAction = nullptr;
    QAction *m_boldAction = nullptr;
    QAction *m_italicAction = nullptr;
    QAction *m_underlineAction = nullptr;
    QAction *m_strikethroughAction = nullptr;
    QAction *m_alignLeftAction = nullptr;
    QAction *m_alignCenterAction = nullptr;
    QAction *m_alignRightAction = nullptr;
    QAction *m_justifyAction = nullptr;
    QAction *m_bulletAction = nullptr;
    QAction *m_numberAction = nullptr;
    QAction *m_checklistAction = nullptr;
    QAction *m_pageBreakAction = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_cutAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;
    QAction *m_newFolderAction = nullptr;
    QAction *m_newEntryAction = nullptr;
    QLabel *m_wordCountLabel = nullptr;
    QLineEdit *m_titleEdit = nullptr;

    QSplitter *m_splitter = nullptr;
    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_fileModel = nullptr;
};

#endif // SHOINFRAME_H
