#include "databasemanager.h"
#include <QDebug>
#include <QtSql/QSqlQuery>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{

}

DatabaseManager::~DatabaseManager()
{
    if(db.isOpen())
        db.close();
}

bool DatabaseManager::initDatabase()
{
    QStringList strList = db.drivers();
    qDebug() << "qt supported database names: " << strList << endl;
    qDebug() << "we use: QSQLITE" << endl;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("SaveDB.db");
    db.setUserName("cepri");
    db.setPassword("cepri");
    bool ok = db.open();
    return ok;
}

bool DatabaseManager::createTableDefine()
{
    QString createStr = "CREATE TABLE IF NOT EXISTS TABLE_DEFINE(NAME TEXT, DETAIL TEXT)";
    QSqlQuery query(createStr);
    bool ok = query.exec();
    return ok;
}

bool DatabaseManager::createTableUnion()
{
    QString createStr = "CREATE TABLE IF NOT EXISTS TABLE_UNION(NO INTEGER, WORD_NAME TEXT, TABLE_RECORD_NAME TEXT, TABLE_FORMAT_NAME TEXT)";
    QSqlQuery query(createStr);
    bool ok = query.exec();
    return ok;
}

bool DatabaseManager::createTableUnionDetail()
{
    QString createStr = "CREATE TABLE IF NOT EXISTS TABLE_UNIONDETAIL(WORD_NAME TEXT, SELF TEXT, TYPE TEXT, CONTROL TEXT, OPERATION)";
    QSqlQuery query(createStr);
    bool ok = query.exec();
    return ok;
}

void DatabaseManager::createTableRecordAndTableFormat(QList<UnionInfo> unionInfoList)
{
    QSqlQuery query;
    for(int i = 0; i < unionInfoList.count(); i ++)
    {
        QStringList recordHeaderList;
        QStringList formatHeaderList;
        for(int j = 0; j < unionInfoList.at(i).field_count; j ++)
        {
            recordHeaderList.append(tr("INFO_%1 TEXT").arg(j + 1));
            formatHeaderList.append(tr("FLAG_%1 TEXT").arg(j + 1));
        }
        QString createStr = tr("CREATE TABLE IF NOT EXISTS %1(%2)").arg(unionInfoList.at(i).table_record_name).arg(recordHeaderList.join(","));
        query.exec(createStr);
        createStr = tr("CREATE TABLE IF NOT EXISTS %1(%2)").arg(unionInfoList.at(i).table_format_name).arg(formatHeaderList.join(","));
        query.exec(createStr);

    }
}

bool DatabaseManager::insertOrUpdateDefineInfo(DefineInfo defineInfo)
{
    QString insertStr = tr("INSERT INTO TABLE_DEFINE(NAME, DETAIL) VALUES ('%1', '%2')").arg(defineInfo.name).arg(defineInfo.detail);
    QSqlQuery query(insertStr);
    bool ok = query.exec();
    return ok;
}

QList<UnionInfo> DatabaseManager::insertUnionInfo(const QString &filepath, int tableCount, QList<int> filedCountList)
{
    QString selectStr = tr("SELECT COUNT(*) FROM TABLE_UNION WHERE WORD_NAME='%1'").arg(filepath);
    QSqlQuery query;
    int count = 0;
    query.exec(selectStr);
    if(query.next())
    {
        count = query.value(0).toInt();
    }
    if(count != 0)
    {
        return QList<UnionInfo>();
    }

    selectStr = tr("SELECT COUNT(*) FROM TABLE_UNION");
    query.exec(selectStr);
    if(query.next())
    {
        count = query.value(0).toInt();
    }
    qDebug() << "TABLE_UNION row count:" << count << endl;

    QList<UnionInfo> unionInfoList;
    for(int i = 0; i < tableCount; i ++)
    {
        count ++;
        UnionInfo unionInfo;
        unionInfo.no = count;
        unionInfo.table_record_name = tr("TABLE_RECORD_%1").arg(unionInfo.no);
        unionInfo.table_format_name = tr("TABLE_FORMAT_%1").arg(unionInfo.no);
        unionInfo.word_name = filepath;
        unionInfo.field_count = filedCountList.at(i);
        unionInfoList.append(unionInfo);
        QString insertStr = tr("INSERT INTO TABLE_UNION(NO, WORD_NAME, TABLE_RECORD_NAME, TABLE_FORMAT_NAME) VALUES (%1, '%2', '%3', '%4')").arg(unionInfo.no).arg(unionInfo.word_name).arg(unionInfo.table_record_name).arg(unionInfo.table_format_name);
        query.exec(insertStr);
        qDebug() << insertStr << endl;
    }
    return unionInfoList;
}

void DatabaseManager::insertRecordInfo(QList<UnionInfo> unionInfoList, QStringList infoList)
{
    for(int i = 0; i < infoList.count(); i ++)
    {
        //! 以表为单位
        db.transaction();
        QStringList rowInfoList = infoList.at(i).split(";");
        for(int j = 0; j < rowInfoList.count(); j ++)
        {
            //! 以表中一行为单位
            QStringList columnInfoList = rowInfoList.at(j).split(",");
            QStringList headerList;
            QStringList valueList;
            for(int k = 0; k < columnInfoList.count(); k ++)
            {
                headerList.append(tr("INFO_%1").arg(k + 1));
                valueList.append(tr("'%1'").arg(columnInfoList.at(k)));
            }
            QString insertStr = tr("INSERT INTO %1(%2) VALUES (%3)").arg(unionInfoList.at(i).table_record_name).arg(headerList.join(", ")).arg(valueList.join(", "));
            QSqlQuery query(insertStr);
            query.prepare(insertStr);
        }
        bool ok = db.commit();
        if(!ok)
        {
            db.rollback();
        }
    }
}

bool DatabaseManager::insertFormatInfo(int index, QList<UnionInfo> unionInfoList, QStringList infoList)
{
    db.transaction();
    QSqlQuery query;
    //! 先删除format表格中的数据
    QString deleteStr = tr("DELETE FROM %1").arg(unionInfoList.at(index - 1).table_format_name);
    query.exec(deleteStr);
    //! 再为format表格，添加新的数据
    for(int i = 0; i < infoList.count(); i ++)
    {
        QStringList rowInfoList = infoList.at(i).split("*");
        QStringList headerList;
        QStringList valueList;
        for(int i = 0; i < rowInfoList.count(); i ++)
        {
            headerList.append(tr("FLAG_%1").arg(i + 1));
            valueList.append(tr("'%1'").arg(rowInfoList.at(i)));
        }

        QString insertStr = tr("INSERT INTO %1(%2) VALUES (%3)").arg(unionInfoList.at(index - 1).table_format_name).arg(headerList.join(", ")).arg(valueList.join(", "));
        query.exec(insertStr);
    }
    bool ok = db.commit();
    if(!ok)
    {
        db.rollback();
    }
    return ok;
}

bool DatabaseManager::deleteDefineInfo(DefineInfo defineInfo)
{
    QString deleteStr = tr("DELETE FROM TABLE_DEFINE WHERE NAME='%1'").arg(defineInfo.name);
    QSqlQuery query(deleteStr);
    bool ok = query.exec();
    return ok;
}

void DatabaseManager::deleteOneWordRecord(const QString &name)
{
    QSqlQuery query;
    QString selectStr = tr("SELECT TABLE_RECORD_NAME, TABLE_FORMAT_NAME FROM TABLE_UNION WHERE WORD_NAME='%1'").arg(name);
    query.exec(selectStr);
    QStringList recordNameList;
    QStringList formatNameList;
    while (query.next()) {
        recordNameList.append(query.value(0).toString());
        formatNameList.append(query.value(1).toString());
    }

    for(int i = 0; i < recordNameList.count(); i ++)
    {
        QString dropRecordStr = tr("DROP TABLE %1").arg(recordNameList.at(i));
        QString dropFormatStr = tr("DROP TABLE %1").arg(formatNameList.at(i));
        bool ok_record = query.exec(dropRecordStr);
        bool ok_format = query.exec(dropFormatStr);
        qDebug() << "DROP TABLE RECORD result:" << ok_record << endl;
        qDebug() << "DROP TABLE FORMAT result:" << ok_format << endl;
    }

    QString deleteStr = tr("DELETE FROM TABLE_UNION WHERE WORD_NAME='%1'").arg(name);
    bool ok = query.exec(deleteStr);
    qDebug() <<"DELETE FROM TABLE_UNION result: " << ok << endl;
}

void DatabaseManager::updateLogicInfo(const QString &wordname, const QStringList &detailInfoList)
{
    QString deleteStr = tr("DELETE FROM TABLE_UNIONDETAIL WHERE WORD_NAME=%1").arg(wordname);
    QSqlQuery query;
    query.exec(deleteStr);
    for(int i = 0; i < detailInfoList.count(); i ++)
    {
        QStringList curDetailInfo = detailInfoList.at(i).split("*");
        QString insertStr = tr("INSERT INTO TABLE_UNIONDETAIL(WORD_NAME, SELF, TYPE, CONTROL) VALUES ('%1', '%2', '%3', '%4')").arg(curDetailInfo.at(0)).arg(curDetailInfo.at(1)).arg(curDetailInfo.at(2)).arg(curDetailInfo.at(3));
        query.exec(insertStr);
    }
}

QStringList DatabaseManager::getWordNames()
{
    QStringList tableNameList;
    QString selectStr = tr("SELECT DISTINCT WORD_NAME FROM TABLE_UNION");
    QSqlQuery query;
    query.exec(selectStr);
    while (query.next()) {
        tableNameList.append(query.value(0).toString());
    }
    return tableNameList;
}

void DatabaseManager::getDataFromFileName(const QString &tablename, int &tableCount, QList<UnionInfo> &unionInfoList, QStringList &allInfoList, QList<int> &allInfoFieldList, QStringList &allFormatList)
{
    unionInfoList.clear();
    allInfoList.clear();
    allInfoFieldList.clear();
    allFormatList.clear();


    QString selectStr = tr("SELECT WORD_NAME, TABLE_RECORD_NAME, TABLE_FORMAT_NAME FROM TABLE_UNION WHERE WORD_NAME='%1'").arg(tablename);
    qDebug() << selectStr << endl;
    QSqlQuery query;
    query.exec(selectStr);
    tableCount = 0;
    while (query.next())
    {
        tableCount ++;
        UnionInfo unionInfo;
        unionInfo.word_name = tablename;
        unionInfo.table_record_name = query.value(1).toString();
        unionInfo.table_format_name = query.value(2).toString();
        //! 还有两个信息暂时获取不到:unionInfo.no、unionInfo.field_count
        unionInfoList.append(unionInfo);
    }

    for(int i = 0; i < unionInfoList.count(); i ++)
    {
        UnionInfo unionInfo = unionInfoList.at(i);
        int fieldCountRecord = 0;
        int fieldCountFormat = 0;
        QStringList recordList;
        QStringList formatList;


        QString selectFieldCountRecordStr = tr("PRAGMA table_info(%1)").arg(unionInfo.table_record_name); //query(0、1、2)分别代表cid、name、type
        query.exec(selectFieldCountRecordStr);
        while (query.next()) {
            fieldCountRecord ++;
        }

        QString selectFieldCountFormatStr = tr("PRAGMA table_info(%1)").arg(unionInfo.table_format_name); //query(0、1、2)分别代表cid、name、type
        query.exec(selectFieldCountFormatStr);
        while (query.next()) {
            fieldCountFormat ++;
        }

        if(fieldCountFormat != fieldCountRecord)
        {
            qCritical() << "headers' count not matched datas' column" << endl;
        }

        //! 列数
        allInfoFieldList.append(fieldCountRecord);


        QString selectRecordStr = tr("SELECT * FROM %1").arg(unionInfo.table_record_name);
        query.exec(selectRecordStr);
        while (query.next()) {
            QStringList rowInfoList;
            for(int i = 0; i < fieldCountRecord; i ++)
            {
                rowInfoList.append(query.value(i).toString());
            }
            recordList.append(rowInfoList.join(","));
        }
        //! 原始记录内容
        allInfoList.append(recordList.join(";"));

        QString selectFormatStr = tr("SELECT * FROM %1").arg(unionInfo.table_format_name);
        query.exec(selectFormatStr);
        while (query.next()) {
            QStringList rowInfoList;
            for(int i = 0; i < fieldCountFormat; i ++)
            {
                rowInfoList.append(query.value(i).toString());
            }
            formatList.append(rowInfoList.join("*"));
        }
        //! 记录的格式信息
        allFormatList.append(formatList.join("**"));
    }
}

QList<DefineInfo> DatabaseManager::getDefineInfo()
{
    QString selectStr = tr("SELECT * FROM TABLE_DEFINE");
    QSqlQuery query(selectStr);
    QList<DefineInfo> defineInfoList;
    while (query.next()) {
        DefineInfo defineInfo;
        defineInfo.name = query.value(0).toString();
        defineInfo.detail = query.value(1).toString();
        defineInfoList.append(defineInfo);
    }
    return defineInfoList;
}

QList<UnionInfo> DatabaseManager::getUnionInfo()
{
    QString selectStr = tr("SELECT * FROM TABLE_UNION");
    QSqlQuery query(selectStr);
    QList<UnionInfo> unionInfoList;
    while (query.next()) {
        UnionInfo unionInfo;
        unionInfo.no = query.value(0).toInt();
        unionInfo.word_name = query.value(1).toString();
        unionInfo.table_record_name = query.value(2).toString();
        unionInfo.table_format_name = query.value(3).toString();
        unionInfoList.append(unionInfo);
    }
    return unionInfoList;
}

QStringList DatabaseManager::getRecordInfo()
{
    QString selectStr = tr("SELECT * FROM TABLE_RECORD");
    QSqlQuery query(selectStr);
    QStringList recordInfoList;
    while (query.next()) {
        for(int i = 0; i < 6; i ++)
        {
            recordInfoList.append(query.value(i).toString());
        }
    }
    return recordInfoList;
}

QStringList DatabaseManager::getFormatInfo()
{
    QString selectStr = tr("SELECT * FROM TABLE_RECORD");
    QSqlQuery query(selectStr);
    QStringList formatInfoList;
    while (query.next()) {
        for(int i = 0; i < 6; i ++)
        {
            formatInfoList.append(query.value(i).toString());
        }
    }
    return formatInfoList;
}
