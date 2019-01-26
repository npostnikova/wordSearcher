#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>
#include <QStandardItemModel>
#include <memory>
#include <QApplication>


struct MyModel : public QStandardItemModel {
    MyModel(QWidget * parent);

    ~MyModel();

    void clean();

    void appendRow(QList<QStandardItem*> const& data);

    void appendRows(std::vector<QString> const& data);

};

struct MyTreeView : QTreeView {
    MyTreeView(QWidget * parent = nullptr);
    
    static QList<QStandardItem*> buildDuplicatesItem(QString const& fileName);

    void deleteTraverse();
    
    MyModel* getModel() {
        return model;
    }

    void clean();

private:
    void setUp();

    MyModel* model;

    QWidget* deleter;
};

#endif // MYTREEVIEW_H
