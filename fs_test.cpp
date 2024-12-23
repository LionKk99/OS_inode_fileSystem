#include <iostream>
#include <string>
#include <cassert>
#include  "fileSystem.h"

//#include <windows.h> //用于Sleep
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
using namespace std;
int main() {
    
    // 创建文件系统对象
    
    fileSystem fs;    
   
    cout << fs.listFileSnapshot("test.txt") << endl;
    fs.useFileSnapshots("test.txt", "2024-12-23_15:38:59", "admin");
    cout << "after use file snapshots:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;

    /*
    fs.createFile("test.txt", "admin");
    fs.writeFile("test.txt", "admin", "this is the test 1 !",1);
    cout << fs.readFile("test.txt", "admin") << endl;
    cout << fs.enableFileSnapshot("test.txt", "admin") << endl;
    cout << endl;
    cout << fs.listFileSnapshot("test.txt") << endl;
    Sleep(1000);//休眠一秒，保证时间不同
    fs.writeAppendFile("test.txt", "admin", "\nthis is the test 2 !");
    cout << endl;
    cout << fs.listFileSnapshot("test.txt") << endl;
    Sleep(1000);//休眠一秒，保证时间不同
    fs.writeAppendFile("test.txt", "admin", "\nthis is the test 3 !");
    cout << endl;
    cout << fs.listFileSnapshot("test.txt") << endl; */
    

    /*
    fs.createFile("test.txt", "admin");
    fs.writeFile("test.txt", "admin", "this is the test 1 !");
    cout << "original context:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    cout << fs.enableFileSnapshot("test.txt", "admin") << endl;
    fs.writeFile("test.txt", "admin", "this is the test 2 !");
    cout << "after change 2 context:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    fs.writeFile("test.txt", "admin", "this is the test 3 !");
    cout << "after change 3 context:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    cout << "File Snapshot list:" << endl;
    cout << fs.listFileSnapshot("test.txt") << endl;
    
    
    fs.useFileSnapshots("test.txt", "2024-12-23_14:18:29", "admin");
    cout << "after use File Snapshots:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
  
    
    fs.createFile("test.txt", "admin");
    fs.writeFile("test.txt", "admin", "this is the test 1 !");
    cout << "original context:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    cout << fs.enableFileSnapshot("test.txt", "admin") << endl;
    fs.writeFile("test.txt", "admin", "this is the test 2 !");
    cout << "after change context:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    cout << "File Snapshot list:" << endl;
    cout << fs.listFileSnapshot("test.txt") << endl;
  

   */ 
    //fs.createUser(PATIENT, "P1", "123");
    //fs.createUser(DOCTOR, "D1", "123");
    //fs.createUser(PATIENT, "P2", "123");
    //fs.createUser(PATIENT, "P3", "123");

    //fs.releaseAppointments("D1", 2024, 8, 8);
    //cout << fs.listAppointments() << endl;
    //fs.revocationAppointments("D1", 2024, 8, 8);
    //cout << fs.listAppointments() << endl;
    //fs.writeAppointments("P2", "D1", 2024, 8, 8);
    //fs.writeAppointments("P3", "D1", 2024, 8, 8);
    //fs.deleteAppointments("P2", "D1", 2024, 8, 8);
    //cout << fs.readAppointments("D1", 2024, 8, 8) << endl;

    //cout << fs.listRecords() << endl;
    // cout << fs.readRecords("P1.txt") << endl;
    //fs.writeAppendRecords("P1", "D1","something bad");
    //cout << fs.readRecords("P1.txt") << endl;
    /*
    cout << fs.displayDirectory("/") << endl;
    cout << fs.deleteFile("test.txt", "admin");
    cout << fs.displayDirectory("/") << endl;
    

    //fs.createFile("test.txt", "admin");
    //cout<<fs.displayDirectory("/")<<endl;
    //std::string data_a = std::string(5000, 'a');  // 
    //fs.writeFile("/test.txt", "admin", data_a);
    cout << "check the context in /test.txt before append:" << endl;
    //cout << fs.readFile("test.txt", "admin") << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    std::string data = std::string(2000, 'b');  // 
    fs.writeAppendFile("test.txt", "admin", data);
    cout << "check the context in /test.txt after append:" << endl;
    cout << fs.readFile("test.txt", "admin") << endl;
    
    //cout<< fs.writeAppendFile("/test.txt", "admin", data)<<endl;    //存在问题
    //cout << "check the context in /test.txt after append:" << endl;
    //cout << fs.readFile("/test.txt", "admin") << endl;

    //cout<<fs.displayDirectory("user/")<<endl;
    //cout << fs.readFile("user/doctor.txt", "admin")<<endl;
    //fs.createUser(DOCTOR, "doctor2", "12344");    
    //fs.deleteUser("admin", DOCTOR, "doctor2");
    //cout << fs.readFile("user/doctor.txt", "admin") << endl;
    
    

    fs.changePassword(1, "doctor", "123");
    fs.userLogin(1, "doctor", "123456");
    fs.userLogin(1, "doctor", "123");
   
    fs.loadBackUp("backup\\1733491089.dat");
    fs.listBackUp();
    

    fs.listBackUp();
    cout << fs.loadBackUp("backup\\1733491089.dat");
    std::cout << fs.displayDirectory("/") << std::endl;
    
    fs.saveBackUp();
    fs.listBackUp();

    std::cout << fs.displayDirectory("/") << std::endl;
    std::cout << fs.createDirectory("user1", "admin") << std::endl;
    std::cout << fs.displayDirectory("/") << std::endl;
   

    cout << sizeof(FileBlock) << endl;
    cout << sizeof(DirectoryBlock) << endl;

    

    //目录测试
    std::cout << fs.displayDirectory("/") << std::endl;
    std::cout << fs.createDirectory("user", "admin") << std::endl;
    std::cout << fs.displayDirectory("/") << std::endl;
    std::cout << fs.createDirectory("user1", "admin") << std::endl;
    std::cout << fs.displayDirectory("/") << std::endl;
    
    std::cout << fs.createDirectory("user/user1", "admin") << std::endl;
    
    std::cout << fs.displayDirectory("user/") << std::endl;

    std::cout << fs.deleteDirectory("user/user1", "admin") << std::endl;

    std::cout << fs.displayDirectory("user/") << std::endl;    

    //文件测试
    std::cout << fs.createFile("user/password.txt","admin") << std::endl;
    std::cout << fs.displayDirectory("user/") << std::endl;

    cout << "write and read test" << endl;
    cout << "succeeful write?" << endl;
    cout << fs.writeFile("user/password.txt", "admin", "this is a test iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\niiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\nnow the context exceed 1024 char") << std::endl;    
    cout << "user/password.txt:";
    cout << endl;    
    cout << fs.readFile("user/password.txt", "admin") << std::endl;
    cout << "\n now try append user/password.txt" << endl;
    cout << fs.writeAppendFile("user/password.txt", "admin", "\nthis ia a append test!") << endl;
    cout << fs.readFile("user/password.txt", "admin") << std::endl;

    std::cout << fs.deleteFile("user/password.txt", "admin") << std::endl;
    std::cout << fs.displayDirectory("user/") << std::endl;
    
    */

    return 0;
}
