/**
 * @file DocumentArea.cpp
 * @brief 多文档标签管理区实现
 * @author hxxcxx
 * @date 2026-04-23
 */
#include "DocumentArea.h"
#include "DocWidget.h"
#include "UIDocument.h"

#include <QLabel>
#include <QTabBar>
#include <QVBoxLayout>

//===================================================
// DocumentArea
//===================================================

DocumentArea::DocumentArea(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &DocumentArea::onCurrentTabChanged);

    layout->addWidget(m_tabWidget);
    showWelcomePage();
}

DocumentArea::~DocumentArea() {
    for (auto& [w, doc] : m_docs) {
        delete doc;
    }
    m_docs.clear();
}

DocWidget* DocumentArea::addDocument(UIDocument* uiDoc, const QString& title) {
    hideWelcomePage();

    auto* docWidget = new DocWidget(this);
    m_docs[docWidget] = uiDoc;
    docWidget->setUIDocument(uiDoc);

    int idx = m_tabWidget->addTab(docWidget, title);
    m_tabWidget->setCurrentIndex(idx);
    updateTabBarVisibility();

    emit documentOpened(title);
    return docWidget;
}

void DocumentArea::closeCurrentDocument() {
    int idx = m_tabWidget->currentIndex();
    if (idx >= 0) closeDocument(idx);
}

void DocumentArea::closeDocument(int index) {
    auto* w = m_tabWidget->widget(index);
    auto* docWidget = qobject_cast<DocWidget*>(w);
    if (!docWidget) return;

    m_docs.erase(docWidget);
    m_tabWidget->removeTab(index);
    docWidget->deleteLater();

    updateTabBarVisibility();
    emit documentClosed();

    if (m_tabWidget->count() == 0) {
        showWelcomePage();
    }
}

DocWidget* DocumentArea::currentDocWidget() const {
    auto* w = m_tabWidget->currentWidget();
    return qobject_cast<DocWidget*>(w);
}

int DocumentArea::documentCount() const {
    return static_cast<int>(m_docs.size());
}

void DocumentArea::showWelcomePage() {
    if (m_welcomePage) return;

    m_welcomePage = new QLabel(
        "<h2 style='color:#888;'>MulanGeo</h2>"
        "<p style='color:#aaa;'>Open a CAD file to begin: File → Open, or drag & drop</p>",
        this);
    m_welcomePage->setAlignment(Qt::AlignCenter);
    m_welcomePage->setStyleSheet("background-color: #373a45;");
    m_tabWidget->addTab(m_welcomePage, tr("Welcome"));
    m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    emit currentDocumentChanged({});
}

void DocumentArea::hideWelcomePage() {
    if (!m_welcomePage) return;

    int idx = m_tabWidget->indexOf(m_welcomePage);
    if (idx >= 0) {
        m_tabWidget->removeTab(idx);
    }
    m_welcomePage->deleteLater();
    m_welcomePage = nullptr;
}

void DocumentArea::updateTabBarVisibility() {
    // 有文档时显示标签栏，仅有欢迎页时隐藏
    m_tabWidget->tabBar()->setVisible(!m_docs.empty());
}

void DocumentArea::onTabCloseRequested(int index) {
    closeDocument(index);
}

void DocumentArea::onCurrentTabChanged(int index) {
    if (index < 0) return;

    auto* w = m_tabWidget->widget(index);
    auto* docWidget = qobject_cast<DocWidget*>(w);
    if (!docWidget) {
        emit currentDocumentChanged({});
        return;
    }

    auto it = m_docs.find(docWidget);
    if (it != m_docs.end()) {
        QString name = QString::fromStdString(it->second->document().displayName());
