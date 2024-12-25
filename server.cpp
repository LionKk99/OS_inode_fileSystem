#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <queue>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <fstream>						
#include <iomanip>						
#include <chrono>
#include  "filesystem.h"

#define BUFFER_SIZE 1024*16

fileSystem fs;

std::string port_num;
std::vector<std::thread> threads;
using namespace std;


void AdministratorMode(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    string username = "";
    string password = "";
    string reply = "";
    while (1) {

        reply = "plz enter user name";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string name(buffer, bytesRead);
        username = name;

        reply = "plz enter password";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string pd(buffer, bytesRead);        
        password = pd;

        //读者获得锁
        fs.lockRC();
        if (fs.rc == 0) {
            fs.lockFS();
        }
        fs.rc++;
        fs.unlockRC();

        if (fs.userLogin(ADMIN, username, password)) {
            reply = "admin log in success";
            send(clientSocket, reply.c_str(), reply.length(), 0);

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }            
            fs.unlockRC();

            break;
        }
        else {
            reply = "user name or password do not match,plz try again";
            send(clientSocket, reply.c_str(), reply.length(), 0);

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

            return;
        }
    }

    cout << "welcome admin user:" << username << endl;

    while (1) {        
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        std::string command(buffer, bytesRead);

        //命令反馈

        // 0. 退出登录
        if (command == "0. back to upper directory") {
            cout << "admin user:" << username << " log out" << endl;
            return;
        }
        // 1. 创建新用户
        else if (command == "1. create user") {
            reply = "plz enter new user type : admin -- 0, doctor -- 1, patient -- 2";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string usertype(buffer, bytesRead);//用户类型
            int userType = -1;
            if (usertype == "0") {
                userType = 0;
            }
            else if (usertype == "1") {
                userType = 1;
            }
            else if (usertype == "2") {
                userType = 2;
            }
            else {
                cout << "invalid type" << endl;
            }

            reply = "plz enter username";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string newUserName(buffer, bytesRead);//用户名

            reply = "plz enter user password";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string newUserPW(buffer, bytesRead);

            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.createUser(userType, newUserName, newUserPW)) {
                reply = "successfully create a new user:" + newUserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to create a new user" + newUserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
        // 2. 删除用户
        else if (command == "2. delete user") {
            reply = "plz enter delete user type : admin -- 0, doctor -- 1, patient -- 2";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string d_usertype(buffer, bytesRead);//用户类型
            int d_userType = -1;
            if (d_usertype == "0") {
                d_userType = 0;
            }
            else if (d_usertype == "1") {
                d_userType = 1;
            }
            else if (d_usertype == "2") {
                d_userType = 2;
            }
            else {
                cout << "invalid type" << endl;
            }

            reply = "plz enter delete username";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string d_UserName(buffer, bytesRead);//用户名
                       

            //此处后面需要加锁
            
            //写者请求锁
            fs.lockFS();

            if (fs.deleteUser(username,d_userType, d_UserName)) {
                reply = "successfully delete user:" + d_UserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to delete user:" + d_UserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
        // 3. 创建备份
        else if (command == "3. create backup") {
            
            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.saveBackUp()) {
                reply = "successfully create a backup:" ;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to create a backup:" ;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
        // 4. 备份目录
        else if (command == "4. list backup") {

            //此处后面需要加锁

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            if (fs.listBackUp().empty() ){
                reply = "backup list is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.listBackUp();
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

        }
        // 5. 导入备份
        else if (command == "5. load backup") {

            reply = "plz enter backup file path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string backup_file_path(buffer, bytesRead);

            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.loadBackUp(backup_file_path)) {
                reply = "successfully load a backup:" + backup_file_path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to load a backup:" + backup_file_path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }       
        // 6. 修改用户密码
        else if (command == "6. change user password") {                   
                      
            reply = "plz enter user type : admin -- 0, doctor -- 1, patient -- 2";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string usertype(buffer, bytesRead);//用户类型
            int userType = -1;
            if (usertype == "0") {
                userType = 0;
            }
            else if (usertype == "1") {
                userType = 1;
            }
            else if (usertype == "2") {
                userType = 2;
            }
            else {
                cout << "invalid type" << endl;
            }

            reply = "plz enter username";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string UserName(buffer, bytesRead);

            reply = "plz enter user new password";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string UserPW(buffer, bytesRead);

            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.changePassword(userType, UserName, UserPW)) {
                reply = "successfully change user password: " + UserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to change user password: " + UserName;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
    }

}

void DoctorMode(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    string username = "";
    string password = "";
    string reply = "";
    while (1) {

        reply = "plz enter user name";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string name(buffer, bytesRead);
        username = name;

        reply = "plz enter password";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string pd(buffer, bytesRead);
        password = pd;

        //读者获得锁
        fs.lockRC();
        if (fs.rc == 0) {
            fs.lockFS();
        }
        fs.rc++;
        fs.unlockRC();

        if (fs.userLogin(DOCTOR, username, password)) {
            reply = "doctor log in success";
            send(clientSocket, reply.c_str(), reply.length(), 0);

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

            break;
        }
        else {
            reply = "user name or password do not match,plz try again";
            send(clientSocket, reply.c_str(), reply.length(), 0);

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

            return;
        }
    }

    cout << "welcome doctor user:" << username << endl;

    while (1) {        
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        std::string command(buffer, bytesRead);

        //命令反馈

        // 0. 退出登录
        if (command == "0. back to upper directory") {
            cout << "doctor user:" << username << " log out" << endl;
            return;
        }
        // 1. 
        else if (command == "1. list records") {
            
            //此处后面需要加锁
            
            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            if (fs.listRecords().empty()) {
                reply = "records list is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.listRecords();
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

        }
        //2
        else if (command == "2. write append records") {

            reply = "plz enter patient username";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string Pname(buffer, bytesRead);//

            reply = "plz enter context(type '-1' to finish)";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string context(buffer, bytesRead);

            //此处后面需要加锁
            
            //写者请求锁
            fs.lockFS();

            if (fs.writeAppendRecords(Pname.c_str(),username.c_str(),context)) {
                reply = "successfully write Append Records";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to write Append Records";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }       

            //写者归还锁
            fs.unlockFS();

        }
        else if (command == "3. read records") {

            reply = "plz enter records Name";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string recordsName(buffer, bytesRead);//

            //此处后面需要加锁

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            if (fs.readRecords(recordsName.c_str()).empty()) {
                reply = "this records is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.readRecords(recordsName.c_str());
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }           

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

        }
        else if (command == "4. release Appointments") {

            reply = "plz enter year";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string year(buffer, bytesRead);//

            reply = "plz enter month";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string month(buffer, bytesRead);//

            reply = "plz enter day";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string day(buffer, bytesRead);//

            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.releaseAppointments (username.c_str(),year,month,day)) {
                reply = "successfully release Appointments";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to release Appointments";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
        else if (command == "5. revocation Appointments") {

            reply = "plz enter year";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string year(buffer, bytesRead);//

            reply = "plz enter month";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string month(buffer, bytesRead);//

            reply = "plz enter day";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string day(buffer, bytesRead);//

            //此处后面需要加锁

            //写者请求锁
            fs.lockFS();

            if (fs.revocationAppointments(username.c_str(), year, month, day)) {
                reply = "successfully revocation Appointments";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to revocation Appointments";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();

        }
        else if (command == "6. list Appointments") {

            //此处后面需要加锁

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            if (fs.listAppointments().empty()) {
                reply = "Appointments list is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.listAppointments();
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

        }
        else if (command == "7. read Appointments") {

            reply = "plz enter year";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string year(buffer, bytesRead);//

            reply = "plz enter month";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string month(buffer, bytesRead);//

            reply = "plz enter day";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string day(buffer, bytesRead);//

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            //此处后面需要加锁
            if (fs.readAppointments(username.c_str(), year, month, day).empty()) {
                reply = "this Appointments is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = year + "-" + month + "-" + day + " " + "appointment\n" + fs.readAppointments(username.c_str(), year, month, day);
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();

        }
    }
}

void PatientMode(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    string username = "";
    string password = "";
    string reply = "";
    while (1) {

        reply = "plz enter user name";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string name(buffer, bytesRead);
        username = name;

        reply = "plz enter password";
        send(clientSocket, reply.c_str(), reply.length(), 0);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string pd(buffer, bytesRead);
        password = pd;

        //读者获得锁
        fs.lockRC();
        if (fs.rc == 0) {
            fs.lockFS();
        }
        fs.rc++;
        fs.unlockRC();

        if (fs.userLogin(PATIENT, username, password)) {
            reply = "patient log in success";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();
            break;
        }
        else {
            reply = "user name or password do not match,plz try again";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();
            return;
        }
    }

    cout << "welcome patient user:" << username << endl;

    while (1) {
        
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        std::string command(buffer, bytesRead);

        //命令反馈

        // 0. 退出登录
        if (command == "0. back to upper directory") {
            cout << "patient user:" << username << " log out" << endl;
            return;
        }
        // 1. 
        else if (command == "1. read my records") {

            //此处后面需要加锁

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            string filename = username + ".txt";
            if (fs.readRecords(filename.c_str()).empty()) {
                reply = "Records is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.readRecords(filename.c_str());
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();
        }
        // 2.
        else if (command == "2. list all Appointments") {

            //此处后面需要加锁

            //读者获得锁
            fs.lockRC();
            if (fs.rc == 0) {
                fs.lockFS();
            }
            fs.rc++;
            fs.unlockRC();

            if (fs.listAppointments().empty()) {
                reply = "Appointments list is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.listAppointments();
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //读者归还锁
            fs.lockRC();
            fs.rc--;
            if (fs.rc == 0) {
                fs.unlockFS();
            }
            fs.unlockRC();
        }
        //3
        else if (command == "3. make my Appointments") {

            reply = "plz enter doctor name";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string d_name(buffer, bytesRead);//

            reply = "plz enter year";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string year(buffer, bytesRead);//

            reply = "plz enter month";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string month(buffer, bytesRead);//

            reply = "plz enter day";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string day(buffer, bytesRead);//

            //写者请求锁
            fs.lockFS();

            if (fs.writeAppointments(username.c_str(),d_name.c_str(),year,month,day)) {
                reply = "Appointments write succssful, plz remember you appointment time.";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "Appointments write fail, this appointment may be full.";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();
        }
        //4
        else if (command == "4. delete my Appointments") {

            reply = "plz enter doctor name";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string d_name(buffer, bytesRead);//

            reply = "plz enter year";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string year(buffer, bytesRead);//

            reply = "plz enter month";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string month(buffer, bytesRead);//

            reply = "plz enter day";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string day(buffer, bytesRead);//

            //写者请求锁
            fs.lockFS();

            if (fs.deleteAppointments(username.c_str(), d_name.c_str(), year, month, day)) {
                reply = "Appointments delete succssful";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "Appointments delete fail";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

            //写者归还锁
            fs.unlockFS();
        }

    }
}

void fsMode(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    string username = "admin";   
    string reply = "";
        
    //获取用户名（不加以验证）
    reply = "plz enter user name";
    send(clientSocket, reply.c_str(), reply.length(), 0);
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::string name(buffer, bytesRead);

    username = name;

    //进入前先请求读锁  

    fs.lockFS();//对文件系统上锁
        
    reply = "welcome to file system mode";
    send(clientSocket, reply.c_str(), reply.length(), 0);

    //命令反馈      
    while (1) {

        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        std::string command(buffer, bytesRead);

        // 0. 退出登录  
        if (command == "0. back to upper directory") {
            cout << "quit file system" << endl;

            //此处需归还权限
            fs.unlockFS();//解锁文件系统
            return;
        }
        // 1
        else if (command == "1. create Directory") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            if (fs.createDirectory(path.c_str(), username.c_str())) {
                reply = "successfully create Directory: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to create Directory: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "2. delete Directory") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            if (fs.deleteDirectory(path.c_str(), username.c_str())) {
                reply = "successfully delete Directory: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to delete Directory: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "3. display Directory") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            if (fs.displayDirectory(path.c_str()).empty()) {
                reply = "this Directory is empty or Path is wrong";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.displayDirectory(path.c_str());
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "4. create File") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            if (fs.createFile(path.c_str(),username.c_str())) {
                reply = "successfully create File: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to create File: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "5. write File") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//

            reply = "plz enter context(type '-1' to finish)";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string context(buffer, bytesRead);

            //此处后面需要加锁
            if (fs.writeFile(path.c_str(),username.c_str(),context,1)) {
                reply = "successfully write file" + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to write file" + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "6. read File") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//
                        

            //此处后面需要加锁
            if (fs.readFile(path.c_str(), username.c_str()).empty()) {
                reply = "this file is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.readFile(path.c_str(), username.c_str());
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "7. delete File") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            if (fs.deleteFile(path.c_str(), username.c_str())) {
                reply = "successfully delete File: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to delete File: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "8. change File Permission") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            reply = "plz enter the generalPermission: N - No Permission, R - Reading Permission, W - Writing Permission";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string p1(buffer, bytesRead);
            char P1 = N;
            if (p1 == "W" || p1 == "w") {
                P1 = W;
            }
            else if (p1 == "R" || p1 == "r") {
                P1 = R;
            }
            else {
                P1 = N;
            }

            reply = "plz enter the groupPermission: N - No Permission, R - Reading Permission, W - Writing Permission";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string p2(buffer, bytesRead);
            char P2 = R;
            if (p2 == "N" || p2 == "n") {
                P2 = N;
            }
            else if (p2 == "W" || p2 == "w") {
                P2 = W;
            }
            else {
                P2 = R;
            }
            
            if (fs.changeFilePermission(username.c_str(),path.c_str(),P1,P2)) {
                reply = "successfully change File Permission: generalPermission - " + p1 + "| groupPermission - " + p2;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to change File Permission to: generalPermission - " + p1 + "| groupPermission - " + p2;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            
        }
        else if (command == "9. change File Owner") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            reply = "plz enter the new owner";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string new_owner(buffer, bytesRead);
            
            if (fs.changeFileOwner(path, username, new_owner)) {
                reply = "successfully change File Owner";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to change File Owner";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

        }
        else if (command == "10. adjust User Group") {

            reply = "plz enter the path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);

            reply = "plz enter the targetUsername";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string targetUsername(buffer, bytesRead);

            reply = "plz enter 1 to add User, enter 0 to remove User";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string addUser(buffer, bytesRead);
            bool adduser;

            if (addUser == "0") {
                adduser = 0;
            }
            else {
                adduser = 1;
            }

            if (fs.adjustUserGroup(path,username, targetUsername, adduser)) {
                reply = "successfully adjust User Group";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to adjust User Group";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

        }
        else if (command == "11. write append File") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//

            reply = "plz enter context(type '-1' to finish)";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string context(buffer, bytesRead);

            //此处后面需要加锁
            if (fs.writeAppendFile(path.c_str(), username.c_str(), context)) {
                reply = "successfully write append file" + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "fail to write append file" + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }

        }
        else if (command == "12. enable File Snapshot") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//


            //此处后面需要加锁
            if (fs.enableFileSnapshot(path,username)) {
                reply = "File Snapshot open: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "File Snapshot open fail: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "13. list File Snapshot") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//


            //此处后面需要加锁
            if (fs.listFileSnapshot(path).empty()) {
                reply = "File Snapshot list is empty";
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = fs.listFileSnapshot(path);
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }
        else if (command == "14. use File Snapshots") {

            reply = "plz enter path";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string path(buffer, bytesRead);//

            reply = "plz enter time";
            send(clientSocket, reply.c_str(), reply.length(), 0);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            std::string time(buffer, bytesRead);//

            //此处后面需要加锁
            if (fs.useFileSnapshots(path,time,username)) {
                reply = "File Snapshot imply succussful: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
            else {
                reply = "File Snapshot imply fail: " + path;
                send(clientSocket, reply.c_str(), reply.length(), 0);
            }
        }         

    }

    

}

void ExitMode(int clientSocket) {
    fs.lockFS();//对文件系统上锁
    fs.saveFileSystem(FILE_NAME);
    fs.unlockFS();
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while (true)
    {
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::string command(buffer, bytesRead);
        if (command == "admin") {
            AdministratorMode(clientSocket);
        }
        else if (command == "doctor") {
            DoctorMode(clientSocket);
        }
        else if (command == "patient") {
            PatientMode(clientSocket);
        }
        else if (command == "fs") {
            fsMode(clientSocket);
        }
        else if (command == "exit") {
            ExitMode(clientSocket);
        }
    }

}

int main() {

    std::cout << "Enter a port number" << std::endl;
    std::getline(std::cin, port_num);

    int serverSocket, clientSocket;


    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Fail to create server socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_family = AF_INET;
    int port = std::stoi(port_num);
    serverAddr.sin_port = htons(port); 
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on server socket" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port " << port << std::endl;


    while (true) {
        socklen_t clientAddrSize = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        std::cout << "Accepted a client connection" << std::endl;

        // a client -> a threads 
        threads.push_back(std::thread(handleClient, clientSocket));
    }

    // Close all client sockets
    for (auto& thread : threads) {
        thread.join();
    }

    // Close the server socket
    close(serverSocket);
    return 0;
}