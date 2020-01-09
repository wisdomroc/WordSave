#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

struct DefineInfo {
    QString name;
    QString detail;
};

struct UnionInfo {
    int no;                     //序号
    QString word_name;          //实际word的名字
    QString table_record_name;  //record表名
    QString table_format_name;  //format表名
    int field_count;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

public:
    bool initDatabase();
    //! 创建数据库表格
    bool createTableDefine();
    bool createTableUnion();
    bool createTableUnionDetail();
    void createTableRecordAndTableFormat(QList<UnionInfo> unionInfoList);

    //! 添加数据至数据库
    bool insertOrUpdateDefineInfo(DefineInfo defineInfo);
    QList<UnionInfo> insertUnionInfo(const QString &filepath, int tableCount, QList<int> filedCountList);
    void insertRecordInfo(QList<UnionInfo> unionInfoList, QStringList infoList);
    bool insertFormatInfo(int index, QList<UnionInfo> unionInfoList, QStringList infoList);

    //! 删除数据库中信息
    bool deleteDefineInfo(DefineInfo defineInfo);
    void deleteOneWordRecord(const QString &name);

    //! 更新数据库中信息
    void updateLogicInfo(const QString &wordname, const QStringList &detailInfoList);

    //! 查询数据库中的信息
    QStringList getWordNames();
    void getDataFromFileName(const QString &tablename, int &tableCount, QList<UnionInfo> &unionInfo, QStringList &allInfoList, QList<int> &allInfoFieldList, QStringList &allFormatList);

private:
    QList<DefineInfo> getDefineInfo();
    QList<UnionInfo> getUnionInfo();
    QStringList getRecordInfo();
    QStringList getFormatInfo();

private:
    QSqlDatabase db;

public slots:
};

#endif // DATABASEMANAGER_H
