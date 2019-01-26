#include "mytreeview.h"
#include <QFileInfo>
#include <QDateTime>
#include <QHeaderView>
#include "mainwindow.h"

MyTreeView::MyTreeView(QWidget * parent) : QTreeView(parent), model(new MyModel(this)), deleter(parent) {
    setUp();
    
}

void MyTreeView::setUp() {
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    model->setHorizontalHeaderLabels({"File name", "Size", "Date"});
    setModel(model);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    setColumnWidth(1, 150);
    setColumnWidth(2, 150);
    setColumnWidth(3, 80);
    setSortingEnabled(true);
    setUniformRowHeights(true);
}


QList<QStandardItem*> prepareRow(const QString &first,
                                                const QString &second,

                                  const QString &third) {
    QList<QStandardItem *> rowItems;

    QStandardItem * item = new QStandardItem;
    item->setText(first);
    item->setToolTip(QFileInfo(first).fileName());

    rowItems << item;
    rowItems << new QStandardItem(second);
    rowItems << new QStandardItem(third);
    return rowItems;
}

QList<QStandardItem*> MyTreeView::buildDuplicatesItem(QString const& fileName) {
    auto firstFile = QFileInfo(fileName);
    auto pr = prepareRow(firstFile.filePath(), QString::number(firstFile.size()), firstFile.birthTime().toString("dd.MM.yyyy"));

    return pr;
}

void MyTreeView::clean() {
    model->clean();
    setUp();
}
