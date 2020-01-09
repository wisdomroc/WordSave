#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "databasemanager.h"
class QTableWidgetItem;
class QListWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void slot_import();
    void slot_logic();
    void on_pushButton_last_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_save_clicked();
    void slot_itemPressed(QListWidgetItem *item);
    void slot_listWidgetMenu(const QPoint &point);
    void slot_delete();

private:
    void initDataFromFileName(const QString &filename);
    int getWordTablesCount(const QString &name);
    void findWordTablesInfo(const QString &filepath);
    void setupTableWidgetInfo(int index);               // 根据index初始化tableWidget的内容
    void setupTableWidgetColor(int index);              // 根据index初始化tableWidget的底色
    QString getSuitableValue(QTableWidgetItem *item);
    void setupListWidget();
    void updateInfoLabel(int curIndex);
    QStringList processLogicAccordingWordName(const QString &wordname);

private:
    QString m_filename;
    QMenu *m_menu;
    int m_curTable;
    int m_tableCount;
    QList<UnionInfo> m_unionInfoList;
    QStringList m_allInfoList;          // 所有的表格的信息list
    QStringList m_allFormatList;        // 所有的表格 格式信息list
    QList<int> m_allInfoFieldList;      // 所有的表格 字段信息list

    QStringList m_tableNames;
    DatabaseManager *m_databaseManager;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
