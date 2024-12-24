#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono> 
#include <thread> 
#include <fstream>


#define BUFFER_SIZE 1024*16

using namespace std;


void PatientMode(int clientSocket) {

    char buffer[BUFFER_SIZE];

    //发送给服务器想要进入patient操作页面
    std::string message;
    message = "patient";
    send(clientSocket, message.c_str(), message.length(), 0);

    string username;
    string command;

    string d_Name_3, day_3, month_3, year_3;//case 3
    string d_Name_4, day_4, month_4, year_4;//case 4

    int bytesRead;
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    //patient登录流程
    while (1)
    {
        //接受"please enter username"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入username
        username = message;//保存用户名
        send(clientSocket, message.c_str(), message.length(), 0);//发送给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接受"please enter password"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入密码
        send(clientSocket, message.c_str(), message.length(), 0);//发给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接收“登录成功”
        if (std::string(buffer, bytesRead) == "patient log in success")
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            system("clear");
            break;
        }
        else
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            sleep(2);
            system("clear");
            return;
        }
    }

    while (1) {
        cout << "-----------PatientMode------------" << endl;
        cout << "1. read my records" << endl;        
        cout << "2. list all Appointments" << endl;
        cout << "3. make my Appointments" << endl;
        cout << "4. delete my Appointments" << endl;
        cout << "0. back to upper directory" << endl;
        cout << "----------------------------------" << endl;

        string command;

        int choice = -1;
        cin >> choice;
        switch (choice) {
        case 1:
            command = "1. read my records";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 2:
            command = "2. list all Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }                       
            break;

        case 3:
            command = "3. make my Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter doctor Name"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> d_Name_3;
            send(clientSocket, d_Name_3.c_str(), d_Name_3.length(), 0);

            //接收 "plz enter year"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> year_3;
            send(clientSocket, year_3.c_str(), year_3.length(), 0);

            //接收 "plz enter month"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> month_3;
            send(clientSocket, month_3.c_str(), month_3.length(), 0);

            //接收 "plz enter day"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> day_3;
            send(clientSocket, day_3.c_str(), day_3.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 4:
            command = "4. delete my Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter doctor Name"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> d_Name_4;
            send(clientSocket, d_Name_4.c_str(), d_Name_4.length(), 0);

            //接收 "plz enter year"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> year_4;
            send(clientSocket, year_4.c_str(), year_4.length(), 0);

            //接收 "plz enter month"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> month_4;
            send(clientSocket, month_4.c_str(), month_4.length(), 0);

            //接收 "plz enter day"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> day_4;
            send(clientSocket, day_4.c_str(), day_4.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            
            break;

        case 0:
            command = "0. back to upper directory";
            send(clientSocket, command.c_str(), command.length(), 0);
            return;

        default:
            cout << "input invalid, plz try again" << endl;
            break;
        }

    }

}

void DoctorMode(int clientSocket) {

    char buffer[BUFFER_SIZE];

    //发送给服务器想要进入doctor操作页面
    std::string message;
    message = "doctor";
    send(clientSocket, message.c_str(), message.length(), 0);

    string username;
    string command;

    string Pname_2, context_2; // case 2
    string recordsName_3;// case 3
    string year_4, month_4, day_4;// case 4
    string year_5, month_5, day_5;// case 5
   

    int bytesRead;
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    //doctor登录流程
    while (1)
    {
        //接受"please enter username"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入username
        username = message;//保存用户名
        send(clientSocket, message.c_str(), message.length(), 0);//发送给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接受"please enter password"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入密码
        send(clientSocket, message.c_str(), message.length(), 0);//发给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接收“登录成功”
        if (std::string(buffer, bytesRead) == "doctor log in success")
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            system("clear");
            break;
        }
        else
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            system("clear");
            sleep(2);
            return;
        }
    }

    while (1) {
        cout << "-----------DoctorMode------------" << endl;
        cout << "1. list records" << endl;
        cout << "2. write append records" << endl;
        cout << "3. read records" << endl;      
        cout << "4. release Appointments" << endl;
        cout << "5. revocation Appointments" << endl;        
        cout << "6. list Appointments" << endl; 
        cout << "7. read Appointments" << endl; 
        cout << "0. back to upper directory" << endl;
        cout << "---------------------------------" << endl;

        string command;
        string line;

        int choice = -1;
        cin >> choice;
        switch (choice) {
        case 1:
            command = "1. list records";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }           
            break;   

        case 2:
            command = "2. write append records";
            send(clientSocket, command.c_str(), command.length(), 0);

            
            //接收 "plz enter patient username"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> Pname_2;
            send(clientSocket, Pname_2.c_str(), Pname_2.length(), 0);

            //接收 plz enter context(type '-1' to finish)
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            context_2.clear();
           
            while (getline(cin, line)) {
                if (line == "-1") break; // 遇到特殊标志停止输入
                context_2 += line + "\n";  // 将每行内容加入 context_2
            }
            send(clientSocket, context_2.c_str(), context_2.length(), 0);            


            //接收结果
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 3:
            command = "3. read records";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter records Name"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> recordsName_3;
            send(clientSocket, recordsName_3.c_str(), recordsName_3.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 4:
            command = "4. release Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter year"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> year_4;
            send(clientSocket, year_4.c_str(), year_4.length(), 0);

            //接收 "plz enter month"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> month_4;
            send(clientSocket, month_4.c_str(), month_4.length(), 0);

            //接收 "plz enter day"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> day_4;
            send(clientSocket, day_4.c_str(), day_4.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 5:
            command = "5. revocation Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter year"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> year_5;
            send(clientSocket, year_5.c_str(), year_5.length(), 0);

            //接收 "plz enter month"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> month_5;
            send(clientSocket, month_5.c_str(), month_5.length(), 0);

            //接收 "plz enter day"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> day_5;
            send(clientSocket, day_5.c_str(), day_5.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 6:
            command = "6. list Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);
            
            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 7:
            command = "7. read Appointments";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter year"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> year_5;
            send(clientSocket, year_5.c_str(), year_5.length(), 0);

            //接收 "plz enter month"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> month_5;
            send(clientSocket, month_5.c_str(), month_5.length(), 0);

            //接收 "plz enter day"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> day_5;
            send(clientSocket, day_5.c_str(), day_5.length(), 0);

            //接收string
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                cout << endl;
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 0:
            command = "0. back to upper directory";
            send(clientSocket, command.c_str(), command.length(), 0);
            return;

        default:
            cout << "input invalid, plz try again" << endl;
            break;
        }

    }

}

void AdministratorMode(int clientSocket) {
    char buffer[BUFFER_SIZE];

    //发送给服务器想要进入admin操作页面
    std::string message;
    message = "admin";
    send(clientSocket, message.c_str(), message.length(), 0);

    string username;
    string command;
    string userType, newUserName, newUserPW; // case 1
    string d_userType, d_UserName;// case 2
    string backup_file_path;//case 5
    string userType_7, UserName_7, newPW_7;//case 6


    int bytesRead;
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    //admin登录流程
    while (1)
    {
        //接受"please enter username"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入username
        username = message;//保存用户名
        send(clientSocket, message.c_str(), message.length(), 0);//发送给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接受"please enter password"
        if (bytesRead > 0) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
        }
        cin >> message;//输入密码
        send(clientSocket, message.c_str(), message.length(), 0);//发给服务器
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        //接收“登录成功”
        if (std::string(buffer, bytesRead) == "admin log in success")
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            system("clear");
            break;
        }
        else
        {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            sleep(2);
            system("clear");
            return;
        }
    }

    //admin功能选择界面
    while (1) {
        cout << "-----------AdministratorMode------------" << endl;
        cout << "1. create user" << endl;
        cout << "2. delete user" << endl;
        cout << "3. create backup" << endl;
        cout << "4. list backup" << endl;
        cout << "5. load backup" << endl;        
        cout << "6. change user password" << endl;
        cout << "0. back to upper directory" << endl;      
        cout << "----------------------------------------" << endl;
                

        string command;
        
        int choice = -1;
        cin >> choice;
        switch (choice) {
            case 1:
                command = "1. create user";
                send(clientSocket, command.c_str(), command.length(), 0);

                //接收 "plz enter new user type : admin -- 0, doctor -- 1, patient -- 2"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }                
                cin >> userType;
                send(clientSocket, userType.c_str(), userType.length(), 0);
                //接收 plz enter username
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }               
                cin >> newUserName;
                send(clientSocket, newUserName.c_str(), newUserName.length(), 0);
                //接收 plz enter user password
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }                
                cin >> newUserPW;
                send(clientSocket, newUserPW.c_str(), newUserPW.length(), 0);
                //接收"successfully create a new user:"/"fail to create a new user"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                break;
            case 2:
                command = "2. delete user";
                send(clientSocket, command.c_str(), command.length(), 0);

                //接收 "plz enter delete user type : admin -- 0, doctor -- 1, patient -- 2"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> d_userType;
                send(clientSocket, d_userType.c_str(), d_userType.length(), 0);
                //接收 plz enter delete username
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> d_UserName;
                send(clientSocket, d_UserName.c_str(), d_UserName.length(), 0);
                
                //接收"successfully delete a  user:"/"fail to create a delete user"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                break;                

            case 3:
                command = "3. create backup";
                send(clientSocket, command.c_str(), command.length(), 0);                               

                //接收"successfully create a backup:"/"fail to create a backup"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }                
                break;

            case 4:
                command = "4. list backup";
                send(clientSocket, command.c_str(), command.length(), 0);

                //接收"string"/"fail to create a backup"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }                
                break;

            case 5:
                command = "5. load backup";
                send(clientSocket, command.c_str(), command.length(), 0);

                //接收 plz enter backup file path
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> backup_file_path;
                send(clientSocket, backup_file_path.c_str(), backup_file_path.length(), 0);

                //接收"successfully load a backup:"/"fail to load a backup"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                break;

            
            case 6:
                command = "6. change user password";
                send(clientSocket, command.c_str(), command.length(), 0);
                
                //接收 "plz enter user type : admin -- 0, doctor -- 1, patient -- 2"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> userType_7;
                send(clientSocket, userType_7.c_str(), userType_7.length(), 0);
                //接收 plz enter username
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> UserName_7;
                send(clientSocket, UserName_7.c_str(), UserName_7.length(), 0);
                //接收 plz enter user new password
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                cin >> newPW_7;
                send(clientSocket, newPW_7.c_str(), newPW_7.length(), 0);
                //接收"successfully change user password:"/"fail to change user password"
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead) << std::endl;
                }
                break;

            case 0:
                command = "0. back to upper directory";
                send(clientSocket, command.c_str(), command.length(), 0);
                return;

            default:
                cout << "input invalid, plz try again" << endl;
                break;
        }

    }

}


void fsMode(int clientSocket) {
    char buffer[BUFFER_SIZE];

    //发送给服务器想要进入fs操作页面
    std::string message;
    message = "fs";
    send(clientSocket, message.c_str(), message.length(), 0);

    int bytesRead;
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);    
    string name;
  
    
    //fsMode登录流程

    //接受"please enter username"
    if (bytesRead > 0) {
        std::cout << std::string(buffer, bytesRead) << std::endl;
    }
    cin >> message;//输入username        
    send(clientSocket, message.c_str(), message.length(), 0);//发送给服务器

    //接收"welcome to file system mode"
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    while (1) {
        if (bytesRead > 0 && (std::string(buffer, bytesRead)== "welcome to file system mode")) {
            std::cout << std::string(buffer, bytesRead) << std::endl;
            break;
        }
    }

    //fs功能选择界面
    while (1) {
        cout << "----------------FileSystem--------------" << endl;
        cout << "1. create Directory" << endl;
        cout << "2. delete Directory" << endl;
        cout << "3. display Directory" << endl;
        cout << "4. create File" << endl;
        cout << "5. write File" << endl;
        cout << "6. read File" << endl;
        cout << "7. delete File" << endl;
        cout << "8. change File Permission" << endl;
        cout << "9. change File Owner" << endl;
        cout << "10. adjust User Group" << endl;
        cout << "11. write appened File" << endl;
        cout << "12. enable File Snapshot" << endl;
        cout << "13. list File Snapshot" << endl;
        cout << "14. use File Snapshots" << endl;
        cout << "0. back to upper directory" << endl;
        cout << "----------------------------------------" << endl;

        string command;
        string line;

        string path_1;//case 1
        string path_2;//case 2
        string path_3;//case 3
        string path_4;//case 4
        string path_5,context_5;//case 5
        string path_6;//case 6
        string path_7;//case 7
        string path_8, generalPermission, groupPermission;//case 8
        string path_9, newOwner;//case 9
        string path_10, targetUsername, addUser;//case 10
        string path_11, context_11;//case 11
        string path_12;
        string path_13;
        string path_14, time_14;

        int choice = -1;
        cin >> choice;

        switch (choice) {
        case 1:
            command = "1. create Directory";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_1;
            send(clientSocket, path_1.c_str(), path_1.length(), 0);

            //接收"successfully create Directory:"/"fail to create Directory:"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 2:
            command = "2. delete Directory";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_2;
            send(clientSocket, path_2.c_str(), path_2.length(), 0);

            //接收"successfully delete Directory:"/"fail to delete Directory:"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 3:
            command = "3. display Directory";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_3;
            send(clientSocket, path_3.c_str(), path_3.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 4:
            command = "4. create File";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_4;
            send(clientSocket, path_4.c_str(), path_4.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 5:
            command = "5. write File";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_5;
            send(clientSocket, path_5.c_str(), path_5.length(), 0);

            //接收 plz enter context(type '-1' to finish)
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            context_5.clear();

            while (getline(cin, line)) {
                if (line == "-1") break; // 遇到特殊标志停止输入
                context_5 += line + "\n";  // 将每行内容加入 context_5
            }
            send(clientSocket, context_5.c_str(), context_5.length(), 0);


            //接收结果
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 6:
            command = "6. read File";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_6;
            send(clientSocket, path_6.c_str(), path_6.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 7:
            command = "7. delete File";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_7;
            send(clientSocket, path_7.c_str(), path_7.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 8:
            command = "8. change File Permission";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_8;
            send(clientSocket, path_8.c_str(), path_8.length(), 0);

            //接收 "plz enter the generalPermission"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> generalPermission;
            send(clientSocket, generalPermission.c_str(), generalPermission.length(), 0);


            //接收 "plz enter the groupPermission"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> groupPermission;
            send(clientSocket, groupPermission.c_str(), groupPermission.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 9:
            command = "9. change File Owner";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_9;
            send(clientSocket, path_9.c_str(), path_9.length(), 0);

            //接收 "plz enter the new owner"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> newOwner;
            send(clientSocket, newOwner.c_str(), newOwner.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 10:
            command = "10. adjust User Group";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_10;
            send(clientSocket, path_10.c_str(), path_10.length(), 0);

            //接收 "plz enter the targetUsername"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> targetUsername;
            send(clientSocket, targetUsername.c_str(), targetUsername.length(), 0);


            //接收 "plz enter the addUser"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> addUser;
            send(clientSocket, addUser.c_str(), addUser.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 11:
            command = "11. write append File";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_11;
            send(clientSocket, path_11.c_str(), path_11.length(), 0);

            //接收 plz enter context(type '-1' to finish)
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            context_11.clear();
            line.clear();
            while (getline(cin, line)) {
                if (line == "-1") break; // 遇到特殊标志停止输入
                context_11 += line + "\n";  // 将每行内容加入 context_11
            }
            send(clientSocket, context_11.c_str(), context_11.length(), 0);


            //接收结果
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 12:
            command = "12. enable File Snapshot";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_12;
            send(clientSocket, path_12.c_str(), path_12.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }

            break;

        case 13:
            command = "13. list File Snapshot";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_13;
            send(clientSocket, path_13.c_str(), path_13.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;
        case 14:
            command = "14. use File Snapshots";
            send(clientSocket, command.c_str(), command.length(), 0);

            //接收 "plz enter the path"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> path_14;
            send(clientSocket, path_14.c_str(), path_14.length(), 0);

            //接收 "plz enter the time"
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            cin >> time_14;
            send(clientSocket, time_14.c_str(), time_14.length(), 0);

            //接收result
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                std::cout << std::string(buffer, bytesRead) << std::endl;
            }
            break;

        case 0:
            command = "0. back to upper directory";
            send(clientSocket, command.c_str(), command.length(), 0);            
            return;


        default:
            cout << "input invalid, plz try again" << endl;
            break;
        }
    }
}

void Welcome(int clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        system("clear");
        cout << "Medical Data Management System" << endl;
        cout << "1. Patient log in" << endl;
        cout << "2. Doctor log in" << endl;
        cout << "3. Administrator log in" << endl;
        cout << "4. File System Mode" << endl;
        cout << "0. Exit System" << endl;
        cout << endl;

        int choice = -1;
        cin >> choice;

        switch (choice) {
        case 0:
            exit(0);
        case 1:
            PatientMode(clientSocket);
            break;
        case 2:
            DoctorMode(clientSocket);
            break;
        case 3:
            AdministratorMode(clientSocket);
            break;
        case 4:
            fsMode(clientSocket);
            break;
        default:
            cout << "Invalid option. Please enter a valid choice (0-3)." << endl;
            sleep(3);  
            break;
        }
    }
}


int main() {
    int clientSocket;
    char buffer[BUFFER_SIZE];
        
    
    std::string port_num;
    std::cout << "Enter a port number" << std::endl;
    std::getline(std::cin, port_num);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1) {
        std::cerr << "Fail to create client socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;    
    int port = std::stoi(port_num);
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Fail to connect server" << std::endl;
        return 1;
    }

    Welcome(clientSocket);

    close(clientSocket);
    return 0;

}