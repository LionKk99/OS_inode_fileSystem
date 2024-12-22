#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <ctime>    // 用于 time_t
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include <chrono>  // 用于生成时间戳

#define ADMIN 0
#define DOCTOR 1
#define PATIENT 2

#define INODE_NUMBER 1024 //一共1024个inode
#define BLOCK_NUMBER 1024*4 //一共1024*4个块
#define BLOCK_SIZE 1024 //每个块大小
#define MAX_LENGTH_FILENAME 28  //名称长度上线
#define MAX_LENGTH_USERNAME 28
#define MAX_GROUP_NUM 16    //用户组最大用户数
#define MAX_PATH_LEN 128    //最大路径大小
#define BLOCKS_PER_INODE 4  //一个inode中的块数上限
#define MAX_BACKUPFILE_SIZE 5   //最大备份数量
#define ENTRY_NUMBER 32 //每个目录块中最多可以存储的文件（或目录）

#define FILE_NAME "filesystem.dat" //文件系统默认保存位置
#define BACKUP_Folder "backup" //备份文件默认存储位置

#define N 0 //没有权限
#define R 1 //读取权限
#define W 2 //读写权限

// 该文件系统单个文件的directBlock有4个，最多支持12个singleIndirectBlock。
// 单个文件最多16个blocks，大小为16*1023byte。
struct Inode {
    int blockIDs[BLOCKS_PER_INODE];     //block编号
    int singleIndirectBlock = -1;       // 单级间接块指针(指向一个块用于存储额外的块地址)
    int blockNum;          //所拥有块数量
    int inodeID;      // Inode 编号
    int fileType;       // 文件类型（普通文件1/目录0）,-1的时候表示空闲可用
    unsigned int fileSize;         // 文件大小32位
    time_t createdTime;        // 文件创建时间
    time_t modifiedTime;       // 文件最后修改时间
    char permission[2] = { 'N', 'R' }; // [0]是一般用户（除创建者和用户组）权限(一般为N)，[1]是用户组的权限 (eg,W,R,N)
    char creator[MAX_LENGTH_USERNAME]; //创建者名字
    int groupNum;                      //用户组内数量
    char group[MAX_GROUP_NUM][MAX_LENGTH_USERNAME]; //用户组（'lihua','zhangsan'）

};


struct FileBlock {
    char content[BLOCK_SIZE]; //一个文件中最多可以写1023个字符，最后写终止符
};

struct DirectoryBlock { //路径block下记载的是该文件夹下所有内容，包括路径和文件（eg. /code/code1.txt）先找到 inodeID为1的根目录，然后看到它是个目录，找到directoryBlock的blockID，看到DirectoryBlock里的filename为code的inodeID(要求同一级下路径文件不同名，可以是code和code.txt)，
                        //然后再找到code这个inode下的directoryBlock的blockID，查看这个block得到这个目录下的所有filename，找到code1.txt以及其inodeID。用此id找到其blockIDs，也就是FileBlock
    char fileName[ENTRY_NUMBER][MAX_LENGTH_FILENAME];//32*28
    int inodeID[ENTRY_NUMBER];//32字节*4

    // 构造函数
    DirectoryBlock() {
        // 初始化 fileName 为 "nan" 且 inodeID 为 -1
        for (int i = 0; i < ENTRY_NUMBER; ++i) {
            strcpy_s(fileName[i], "nan");  // 用 "nan" 填充 fileName
            inodeID[i] = -1;             // 将 inodeID 设置为 -1
        }
    }
};
/*
    初始化文件系统结构:    
    ./
        - user
            - admin.txt
            - doctor.txt 
            - patient.txt
        - records
            - patientID_1.txt
            - patientID_2.txt
            ...
        - appointments
            - doctor_id-time.txt

 */
class fileSystem{
public:
    fileSystem(); //构造函数
    ~fileSystem(); //析构函数
    bool createDirectory(const char *path,const char *user); //成功1，失败0   
    bool deleteDirectory(const char* path, const char* user); //成功1，失败0  
    std::string displayDirectory(const char *path);//显示传入路径下的目录树
    bool createFile(const char* path, const char* user);
    bool writeFile(const char* path, const char* user, const std::string& context);//写入内容过大，或者文件不存在都会导致失败
    bool writeAppendFile(const char* path, const char* user, const std::string& context);
    std::string readFile(const char* path, const char* user);
    bool deleteFile(const char* path, const char* user);

    //记录
    bool creatRecords(const char* Pusername);//创建病人记录文件
    bool deleteRecords(const char* Pusername);//删除病人记录文件
    bool writeAppendRecords(const char* Pname, const char* Dname, const std::string& context);//写入记录文件
    std::string listRecords();//展示所有记录文件名
    std::string readRecords(const char* recordsName);

    //预约人数限制为8人，不得超过
    bool releaseAppointments(const char* Dname, std::string year, std::string month, std::string day);//医生发布预约
    bool revocationAppointments(const char* Dname, std::string year, std::string month, std::string day);
    bool writeAppointments(const char* Pname, const char* Dname, std::string year, std::string month, std::string day);//病人进行预约，先读预约文件，如果人数已经满了则拒绝写入
    bool deleteAppointments(const char* Pname, const char* Dname, std::string year, std::string month, std::string day);//找到文件对应行，跳过该行重新写入
    std::string listAppointments();//展示所有记录文件名
    std::string readAppointments(const char* Dname, std::string year, std::string month, std::string day);


    bool saveFileSystem(const std::string& filePath); //持久化文件系统
    bool loadFileSystem(const std::string& filePath);
    bool saveBackUp();
    bool loadBackUp(const std::string& filePath);
    std::string listBackUp();

    //功能函数

    bool createUser(int usertype, std::string username, std::string password);
    bool userLogin(int usertype, std::string username, std::string password);
    bool changePassword(int usertype, const std::string& username, const std::string& newPassword);
    bool deleteUser(const std::string& operatorName, int deletingUserType, const std::string& usernameToDelete);

    bool changeFilePermission(const std::string& username, const std::string& path, char generalPermission, char groupPermission);
    bool changeFileOwner(const std::string& filePath, const std::string& oper, const std::string& newOwner);
    bool adjustUserGroup(const std::string& filePath, const std::string& operatorName, const std::string& targetUsername, bool addUser);


    bool isAdmin(const std::string& username);//是否为管理员
    bool isDoctor(const std::string& username);
    bool isPatient(const std::string& username);
    bool isUser(const std::string& username);

    std::string getCurrentDateTime();//获取时间
private:
    //数据成员    
    Inode inodeMem[INODE_NUMBER];  // 存储所有的inode
    char inodeBitmap[BLOCK_NUMBER / 8];  // inode的使用位图    
    char blockBitmap[BLOCK_NUMBER / 8];  // block的使用位图
    DirectoryBlock blockMem[BLOCK_NUMBER];  // 存储所有block,既可以当作DirectoryBlock，也可以当作FileBlock
    std::string backupFileName[MAX_BACKUPFILE_SIZE];  // 备份文件名
    //FileBlock blockMem[BLOCK_NUMBER];  // 存储所有数据块(实际上不需要)

    //并发控制
    std::mutex mutex_;  // 用于文件系统操作的互斥锁

   

    // helper 辅助函数
    std::vector<std::string> splitPath(const char* path); //分割路径函数
    Inode* findInodeByPath(const char* path);
    int allocateInode();//找空闲的inode
    int allocateBlock();//找空闲的block
    void freeInode(int inodeID);// 还原位图中对应的位
    void freeBlock(int blockID);// 还原位图中对应的位
    
    void lockMutex();  // 加锁，锁住整个文件系统
    void unlockMutex();  // 解锁

    void updateBackupFileList();

    
};

#endif

/*
    Additional File System Features:
    1、密码登录
    2、密码修改
    3、用户删除（无法删除超级管理员,且只有超级管理员能够删除管理员）
    4、改变文件所有者
    5、调整用户组

*/