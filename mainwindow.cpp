#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QAxObject>
#include <QAxWidget>
#include <QTextCodec>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QListWidget>
#include <QProgressDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),m_curTable(1),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_menu = new QMenu(this);
    QAction *action_delete = m_menu->addAction(QStringLiteral("删除"));
    connect(action_delete, SIGNAL(triggered()), this, SLOT(slot_delete()));

    m_databaseManager = new DatabaseManager();
    m_databaseManager->initDatabase();
    m_databaseManager->createTableUnion();
    m_databaseManager->createTableDefine();
    m_databaseManager->createTableUnionDetail();
    connect(ui->action_import, &QAction::triggered, this, &MainWindow::slot_import);
    connect(ui->action_logic, &QAction::triggered, this, &MainWindow::slot_logic);
    connect(ui->listWidget, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(slot_itemPressed(QListWidgetItem *)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_listWidgetMenu(const QPoint &)));
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu); //! 需设置此方可弹出menu
    setupListWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setText(QStringLiteral("确定关闭？"));
    QPushButton *okBtn = msgBox.addButton(QStringLiteral("确定"), QMessageBox::AcceptRole);
    msgBox.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
    msgBox.setDefaultButton(okBtn);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::AcceptRole:
        event->accept();
        break;
    case QMessageBox::RejectRole:
        event->ignore();
        break;
    }
}

void MainWindow::slot_import()
{
    m_filename = QFileDialog::getOpenFileName(this,QStringLiteral("打开文件"), QDir::homePath(),QStringLiteral("Word (*.doc *.docx)"));
    if(m_filename.isEmpty())
    {
        return;
    }
    else
    {
        if(m_tableNames.contains(QFileInfo(m_filename).fileName()))
        {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("已经有相同名字的WORD文件已导入"));
            return;
        }

        QFile file(m_filename);
        if(!file.open(QIODevice::ReadWrite))
        {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("所选word文件处于打开状态，请关闭后再试"));
            return;
        }
        else
        {
            file.close();
        }
    }

    m_tableCount = getWordTablesCount(m_filename);
    qDebug() << "current wordname: " << m_filename << ", table count: " << m_tableCount << endl;
    m_curTable = 1;
    updateInfoLabel(m_curTable);
    findWordTablesInfo(m_filename);
    setupTableWidgetInfo(1);

    //! 初始化format list
    for(int i = 0; i < m_tableCount; i ++)
    {
        m_allFormatList.append("");
    }

    //! 数据库操作
    m_unionInfoList = m_databaseManager->insertUnionInfo(QFileInfo(m_filename).fileName(), m_tableCount, m_allInfoFieldList);
    m_databaseManager->createTableRecordAndTableFormat(m_unionInfoList);
    m_databaseManager->insertRecordInfo(m_unionInfoList, m_allInfoList);

    //! 更新TreeWidget
    setupListWidget();
}

void MainWindow::slot_logic()
{
    QStringList wordNames = m_databaseManager->getWordNames();
    qDebug() << "all word names: " << wordNames << endl;
    for(int i = 0; i < wordNames.count(); i ++)
    {
        QStringList detailList = processLogicAccordingWordName(wordNames.at(i));
        m_databaseManager->updateLogicInfo(wordNames.at(i), detailList);
    }
}

QStringList MainWindow::processLogicAccordingWordName(const QString &wordname)
{
    m_databaseManager->getDataFromFileName(wordname, m_tableCount, m_unionInfoList, m_allInfoList, m_allInfoFieldList, m_allFormatList);

    //! 开始处理
    QStringList resultList;
    for(int i = 0; i < m_tableCount; i ++)
    {
        QStringList curTableFormatList = m_allFormatList.at(i).split("**");
        for(int j = 0; j < curTableFormatList.count(); j ++)
        {
            char chA = 'A';
            QStringList curRowFormatList = curTableFormatList.at(j).split("*");
            for(int k = 0; k < curRowFormatList.count(); k ++)
            {
                char chX = chA + k;
                QString curStr = curRowFormatList.at(k);
                if(curStr.contains(":")) //! 判断出是输出单元格
                {
                    QStringList curOutputStr = curStr.split(";", QString::SkipEmptyParts);
                    QString curStr = tr("%1%2").arg(chX).arg(j + 1);
                    for(int m = 0; m < curOutputStr.count(); m ++)
                    {
                        QStringList curOutputOneStr = curOutputStr.at(m).split(":");
                        QString oneRowInfo = tr("%1*%2*%3~%3*%4~%4").arg(wordname).arg(curOutputOneStr.at(0)).arg(curOutputOneStr.at(1)).arg(curStr);
                        resultList.append(oneRowInfo);
                    }
                }
            }
        }
    }
    return resultList;
}

void MainWindow::on_pushButton_last_clicked()
{
    if(m_curTable > 1)
    {
        m_curTable --;
        setupTableWidgetInfo(m_curTable);
        setupTableWidgetColor(m_curTable);
        updateInfoLabel(m_curTable);
    }
}

void MainWindow::on_pushButton_next_clicked()
{
    if(m_curTable < m_tableCount)
    {
        m_curTable ++;
        setupTableWidgetInfo(m_curTable);
        setupTableWidgetColor(m_curTable);
        updateInfoLabel(m_curTable);
    }
}

void MainWindow::on_pushButton_save_clicked()
{
    //! 单一表格格式的保存
    QStringList formatStrList;
    int rowCount = ui->tableWidget->rowCount();
    int columnCount = ui->tableWidget->columnCount();
    for(int i = 0; i < rowCount; i ++)
    {
        QStringList rowStrList;
        for(int j = 0; j < columnCount; j ++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if(item != nullptr)
            {
                QString str = getSuitableValue(item);
                rowStrList.append(str);
            }
        }
        formatStrList.append(rowStrList.join("*"));
    }
    m_allFormatList.replace(m_curTable - 1, formatStrList.join("**"));
    m_databaseManager->insertFormatInfo(m_curTable, m_unionInfoList, formatStrList);
}

void MainWindow::slot_itemPressed(QListWidgetItem *item)
{
    m_filename = item->text();
    m_curTable = 1;
    initDataFromFileName(m_filename);
    updateInfoLabel(m_curTable);
    qDebug() << "current word, allInfoList.count: " << m_allInfoList.count() << endl;
    setupTableWidgetInfo(1);
    setupTableWidgetColor(1);
}

void MainWindow::slot_listWidgetMenu(const QPoint &point)
{
    QPoint point_ = mapToGlobal(point);
    point_ = point_ + QPoint(30, 60);
    m_menu->popup(point_);
}

void MainWindow::slot_delete()
{
    QString wordname = ui->listWidget->currentItem()->text();
    m_databaseManager->deleteOneWordRecord(wordname);
    setupListWidget();
}

void MainWindow::initDataFromFileName(const QString &filename)
{
    m_databaseManager->getDataFromFileName(filename, m_tableCount, m_unionInfoList, m_allInfoList, m_allInfoFieldList, m_allFormatList);
}

QString MainWindow::getSuitableValue(QTableWidgetItem *item)
{
    QString value;
    if(item->backgroundColor() == QColor(HEADER_COLOR))
    {
        value = tr("4");
    }
    else if(item->backgroundColor() == QColor(REMARK_COLOR))
    {
        value = tr("0");
    }
    else if(item->backgroundColor() == QColor(INPUT_COLOR))
    {
        value = tr("8;");
    }
    else if(item->backgroundColor() == QColor(ASSIST_COLOR))
    {
        value = tr("9;");
    }
    else if(item->backgroundColor() == QColor(OUTPUT_COLOR))
    {
        int row = item->row();
        int column = item->column();
        //横向查找
        QStringList horizontalStrList;
        char chA = 'A';
        for(int i = 0; i < column; i ++)
        {
            chA = chA + i;
            QTableWidgetItem *item = ui->tableWidget->item(row, i);
            if(item != nullptr)
            {
                if(item->backgroundColor() == QColor(INPUT_COLOR))
                {
                    horizontalStrList.append(tr("2:%1%2;").arg(chA).arg(row + 1));
                }
                else if(item->backgroundColor() == QColor(ASSIST_COLOR))
                {
                    horizontalStrList.append(tr("1:%1%2;").arg(chA).arg(row + 1));
                }
            }
        }
        //纵向查找
        QStringList verticalStrList;
        chA = 'A';
        chA = chA + column;
        for(int i = 0; i < row; i ++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(i, column);
            if(item != nullptr)
            {
                if(item->backgroundColor() == QColor(INPUT_COLOR))
                {
                    verticalStrList.append(tr("2:%1%2;").arg(chA).arg(i + 2));
                }
            }
        }
        QStringList strList;
        strList.append(horizontalStrList);
        strList.append(verticalStrList);
        value = strList.join("");
    }
    return value;
}

void MainWindow::setupListWidget()
{
    m_tableNames = m_databaseManager->getWordNames();
    ui->listWidget->clear();
    ui->listWidget->addItems(m_tableNames);
    if(m_tableNames.count() == 0)
    {
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
    }
}

void MainWindow::updateInfoLabel(int curIndex)
{
    ui->label_info->setText(QStringLiteral("第 ") + QString::number(curIndex) + " / " + QString::number(m_tableCount) + QStringLiteral(" 条"));
}

int MainWindow::getWordTablesCount(const QString &name)
{
    setCursor(Qt::WaitCursor);
    QProgressDialog progress(QStringLiteral("正在获取word文件中整体表格信息,请等待..."), QStringLiteral("终止"), 0, 1, this);
    progress.setWindowTitle(QStringLiteral("导入WORD"));
    progress.setEnabled(false);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QString filepath=name;
    QAxWidget *word = new QAxWidget("Word.Application");
    word->setProperty("Visible",QVariant(false));
    QAxObject *documents =word->querySubObject("Documents");
    documents->dynamicCall("Open(const QString&)",QString(filepath));
    QAxObject *doc = word->querySubObject("ActiveDocument");//获取当前工作簿


    QAxObject* tables = doc->querySubObject("Tables");
    int tableCount = tables->property("Count").toInt();

    doc->dynamicCall("Save()");
    documents->dynamicCall("Close()");//关闭工作簿
    word->dynamicCall("Quit()");//关闭excel

    delete doc;
    delete documents;
    delete word;
    doc = nullptr;
    documents = nullptr;
    word = nullptr;

    progress.setValue(1);
    setCursor(Qt::ArrowCursor);
    return tableCount;
}

void MainWindow::findWordTablesInfo(const QString &filepath)
{
    m_allInfoList.clear();

    setCursor(Qt::WaitCursor);
    QProgressDialog progress(QStringLiteral("正在获取word文件中单个表格信息,请等待..."), QStringLiteral("终止"), 0, m_tableCount, this);
    progress.setWindowTitle(QStringLiteral("导入WORD"));
    progress.setEnabled(false);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QAxWidget *word = new QAxWidget("Word.Application");
    word->setProperty("Visible",QVariant(false));
    QAxObject *document =word->querySubObject("Documents");
    document->dynamicCall("Open(const QString&)",QString(filepath));
    QAxObject *doc = word->querySubObject("ActiveDocument");//获取当前工作簿

    for(int i = 1; i <= m_tableCount; i ++)
    {
        progress.setValue(i);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if (progress.wasCanceled())
            break;

        QAxObject *table=doc->querySubObject("Tables(int)",i);
        QAxObject *rows = table->querySubObject("Rows");
        int rowsCount = rows->property("Count").toInt();
        QAxObject *columns = table->querySubObject("Columns");
        int columnsCount = columns->property("Count").toInt();

        QStringList infoList;
        for(int j = 1;j <= rowsCount; j ++)
        {
            QStringList tmpInfoList;
            for(int k = 1; k <= columnsCount; k ++)
            {
                QAxObject *cell=table->querySubObject("Cell(int,int)",j,k);
                if(cell == nullptr)
                {
                    tmpInfoList.append("");
                }
                else
                {
                    QString cellInfo_ = cell->querySubObject("Range")->property("Text").toString().simplified();
                    cellInfo_ = cellInfo_.replace("\u0007", ""); //! 去掉word中的回车
                    cellInfo_ = cellInfo_.replace(" ", "");
                    tmpInfoList.append(cellInfo_);
                }
            }
            infoList.append(tmpInfoList.join(","));
        }
        m_allInfoFieldList.append(infoList.at(0).split(",").count());
        m_allInfoList.append(infoList.join(";"));
    }

    doc->dynamicCall("Save()");
    document->dynamicCall("Close()");//关闭工作簿
    word->dynamicCall("Quit()");//关闭excel

    delete document;
    delete word;
    doc =nullptr;
    document=nullptr;
    word=nullptr;

    progress.setValue(m_tableCount);
    setCursor(Qt::ArrowCursor);
}

void MainWindow::setupTableWidgetInfo(int index)
{
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    QStringList infoList = m_allInfoList.at(index - 1).split(";");
    int rowCount = infoList.count();
    int columnCount = infoList.at(0).split(",").count();
    ui->tableWidget->setColumnCount(columnCount);
    ui->tableWidget->setRowCount(rowCount);
    for(int i = 0; i < rowCount; i ++)
    {
        QStringList tmpInfoList = infoList.at(i).split(",");
        for(int j = 0; j < columnCount; j ++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(tmpInfoList.at(j));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget->setItem(i, j, item);
        }
    }
    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::setupTableWidgetColor(int index)
{
    QString format = m_allFormatList.at(index - 1);
    QStringList formatList = format.split("**");
    for(int i = 0 ; i < formatList.count(); i ++)
    {
        QStringList rowList = formatList.at(i).split("*");
        for(int j = 0; j < rowList.count(); j ++)
        {
            QString curFormat = rowList.at(j);
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if(curFormat.contains(":"))
            {
                item->setBackgroundColor(QColor(OUTPUT_COLOR));
                if(item->foreground().color() == QColor("black"))
                {
                    item->setForeground(QBrush(QColor(OUTPUT_PEN_COLOR)));//white
                }
                item->setForeground(QBrush(QColor(OUTPUT_PEN_COLOR)));
            }
            else
            {
                if(curFormat == "4")
                {
                    item->setBackgroundColor(QColor(HEADER_COLOR));
                }
                else if(curFormat == "0")
                {
                    item->setBackgroundColor(QColor(REMARK_COLOR));
                    if(item->foreground().color() == QColor("black"))
                    {
                        item->setForeground(QBrush(QColor("white")));
                    }
                }
                else if(curFormat == "8;")
                {
                    item->setBackgroundColor(QColor(INPUT_COLOR));
                    if(item->foreground().color() == QColor("black"))
                    {
                        item->setForeground(QBrush(QColor("white")));
                    }
                }
                else if(curFormat == "9;")
                {
                    item->setBackgroundColor(QColor(ASSIST_COLOR));
                    if(item->foreground().color() == QColor("black"))
                    {
                        item->setForeground(QBrush(QColor("white")));
                    }
                }
            }
        }
    }
}
