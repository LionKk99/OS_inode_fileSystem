#include "filesystem.h"

fileSystem::fileSystem() {

    // 检查并创建 backup 文件夹
    std::filesystem::path backupDir("backup");
    if (!std::filesystem::exists(backupDir)) {
        std::filesystem::create_directory(backupDir);
    }

    if (loadFileSystem(FILE_NAME)) {
        return;
    }
    // 初始化 inode 位图，所有 inode 默认空闲
    memset(inodeBitmap, 0, sizeof(inodeBitmap));  // 所有 inode 默认空闲
    memset(blockBitmap, 0, sizeof(blockBitmap));  // 所有 block 默认空闲
    

    // 顺序初始化每个 inode 的 inodeID 并设置 blockIDs 为 -1
    for (int i = 0; i < INODE_NUMBER; ++i) {
        inodeMem[i].inodeID = i;  // 给每个 Inode 分配一个唯一的 inodeID
        for (int j = 0; j < BLOCKS_PER_INODE; ++j) {
            inodeMem[i].blockIDs[j] = -1;  // 初始化所有 blockIDs 为 -1
        }
    }

    // 初始化根目录
    Inode* rootInode = &inodeMem[0];  // 根目录的 inode
    rootInode->inodeID = 0;
    rootInode->fileType = 0;  // 目录类型
    rootInode->fileSize = 0;
    rootInode->createdTime = time(nullptr);  // 设置根目录的创建时间
    rootInode->modifiedTime = time(nullptr);  // 设置根目录的修改时间
    strcpy(rootInode->creator, "admin");  // 设置根目录的创建者为 "admin"
    rootInode->permission[0] = N;  // 设置根目录的其他用户权限
    rootInode->permission[1] = W;  // 设置根目录的用户组权限为可读写
    rootInode->groupNum = 1;  // 根目录用户组只有一个成员
    strcpy(rootInode->group[0], "admin");  // 设置根目录的用户组为 "admin"

    // 创建根目录的目录块，并分配 block 0
    DirectoryBlock* rootDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[0]);
    rootInode->blockIDs[0] = 0;  // 将根目录的第一个 blockID 指向 block 0
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        rootDirBlock->inodeID[i] = -1;  // 初始化目录块为空
    }
    // 设置根目录目录块中的第一个条目为根目录本身
    strcpy(rootDirBlock->fileName[0], "/");
    rootDirBlock->inodeID[0] = 0;  // 将根目录的 inodeID 设置为 0

    // 更新位图，标记根目录使用了 inode 0 和 block 0
    inodeBitmap[0] |= 1;  // inode 位图的第一位设置为 1
    blockBitmap[0] |= 1;  // block 位图的第一位设置为 1

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
        - FS
            - filename
                - time_1.txt
     */

    //后面可加科室的区分
    createDirectory("user", "admin");
    createDirectory("records", "admin");
    createDirectory("appointments", "admin");
    createDirectory("FS", "admin");

    createFile("user/admin.txt", "admin");
    createFile("user/doctor.txt", "admin");
    createFile("user/patient.txt", "admin");

    writeFile("user/admin.txt", "admin", "admin,123456\n",1);//写入管理用户的账号密码 账号密码之中不允许出现,\
      
    // 输出文件系统初始化成功的消息
    std::cout << "systeam init the first time" << std::endl;
    
    std::cout << displayDirectory("/") << std::endl;
}

int fileSystem::allocateInode() { //分配时自动标记
    // 遍历 inodeBitmap，查找空闲的 inode
    for (int i = 0; i < INODE_NUMBER; ++i) {
        int byteIndex = i / 8;   // 确定位图的字节位置
        int bitIndex = i % 8;    // 确定字节内的具体位位置

        // 如果当前位是 0，说明 inode 是空闲的
        if ((inodeBitmap[byteIndex] & (1 << bitIndex)) == 0) {
            // 标记该 inode 为已分配
            inodeBitmap[byteIndex] |= (1 << bitIndex);

            return i;  // 返回分配的 inode ID
        }
    }

    return -1;  // 如果没有空闲的 inode，返回 -1
}

int fileSystem::allocateBlock() {
    // 遍历整个块位图，找到第一个未使用的块
    for (int i = 0; i < BLOCK_NUMBER; ++i) {
        // 检查该块是否已分配
        if ((blockBitmap[i / 8] & (1 << (i % 8))) == 0) { // 如果该块没有被分配
            // 更新位图，标记该块为已分配
            blockBitmap[i / 8] |= (1 << (i % 8));  // 设置对应位为1

            // 返回这个空闲块的ID
            return i;
        }
    }

    // 如果没有找到空闲的块，返回-1表示失败
    return -1;
}

bool fileSystem::createDirectory(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        return false;  // 路径不能为空
    }

    // 2. 获取路径的各个部分
    std::vector<std::string> pathParts = splitPath(path);

    if (pathParts.empty()) {
        return false;  // 空路径
    }

    // 3. 获取父路径（除了最后一个部分）
    std::string parentPath = "";
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        parentPath += "/" + pathParts[i];  // 将除最后一个元素外的路径拼接成父路径
    }

    // 4. 获取新目录的名字（最后一个部分）
    std::string newDirName = pathParts.back();

    // 5. 查找父目录的 inode
    Inode* parentInode = findInodeByPath(parentPath.c_str());
    if (!parentInode) {
        return false;  // 父目录不存在
    }

    // 6. 确保父目录是有效目录
    if (parentInode->fileType != 0) {
        return false;  // 父 inode 不是目录
    }

    // 7. 检查是否已有同名的目录
    DirectoryBlock* parentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[parentInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (parentDirBlock->inodeID[i] != -1 && strcmp(parentDirBlock->fileName[i], newDirName.c_str()) == 0) {
            return false;  // 目录名已存在
        }
    }

    // 8. 在父目录中创建新目录
    bool dirCreated = false;
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (parentDirBlock->inodeID[i] == -1) {  // 找到一个空位置
            // 调用 allocateInode 来分配一个新的 inode
            int newInodeID = allocateInode();
            if (newInodeID == -1) {
                return false;  // 没有空闲的 inode
            }

            // 创建新目录的 inode 信息
            Inode* newInode = &inodeMem[newInodeID];
            newInode->fileType = 0;  // 目录类型
            newInode->fileSize = 0;
            newInode->createdTime = time(nullptr);
            newInode->modifiedTime = time(nullptr);
            strcpy(newInode->creator, user);  // 设置创建者为传入的用户
            newInode->permission[0] = N;     // 设置其他用户无权限
            newInode->permission[1] = W;     // 设置用户组权限为读写

            // 为新目录分配一个块
            int newBlockID = allocateBlock();
            if (newBlockID == -1) {
                return false;  // 没有空闲的块
            }
            newInode->blockIDs[0] = newBlockID;  // 将新分配的块ID保存到inode

            // 初始化新目录块为一个空的目录块
            DirectoryBlock* newDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[newBlockID]);
            for (int j = 0; j < ENTRY_NUMBER; ++j) {
                newDirBlock->inodeID[j] = -1;  // 初始化为空
            }

            // 在父目录中添加新目录
            parentDirBlock->inodeID[i] = newInodeID;
            strcpy(parentDirBlock->fileName[i], newDirName.c_str());

            dirCreated = true;
            break;
        }
    }

    if (!dirCreated) {
        return false;  // 父目录没有空余位置
    }

    return true;
}

bool fileSystem::createFile(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        return false;  // 路径不能为空
    }

    // 2. 获取路径的各个部分
    std::vector<std::string> pathParts = splitPath(path);

    if (pathParts.empty()) {
        return false;  // 空路径
    }

    // 3. 获取父路径（除了最后一个部分）
    std::string parentPath = "";
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        parentPath += "/" + pathParts[i];  // 将除最后一个元素外的路径拼接成父路径
    }

    // 4. 获取新文件的名字（最后一个部分）
    std::string newFileName = pathParts.back();

    // 5. 查找父目录的 inode
    Inode* parentInode = findInodeByPath(parentPath.c_str());
    if (!parentInode) {
        return false;  // 父目录不存在
    }

    // 6. 确保父目录是有效目录
    if (parentInode->fileType != 0) {
        return false;  // 父 inode 不是目录
    }

    // 7. 检查是否已有同名的文件
    DirectoryBlock* parentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[parentInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (parentDirBlock->inodeID[i] != -1 && strcmp(parentDirBlock->fileName[i], newFileName.c_str()) == 0) {
            return false;  // 文件名已存在
        }
    }

    // 8. 在父目录中创建新文件
    bool fileCreated = false;
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (parentDirBlock->inodeID[i] == -1) {  // 找到一个空位置
            // 调用 allocateInode 来分配一个新的 inode
            int newInodeID = allocateInode();
            if (newInodeID == -1) {
                return false;  // 没有空闲的 inode
            }

            // 创建新文件的 inode 信息
            Inode* newInode = &inodeMem[newInodeID];
            newInode->fileType = 1;  // 文件类型设置为 1
            newInode->fileSize = 0;
            newInode->createdTime = time(nullptr);
            newInode->modifiedTime = time(nullptr);
            strcpy(newInode->creator, user);  // 设置创建者为传入的用户
            newInode->permission[0] = N;  // 设置其他用户无权限
            newInode->permission[1] = W;  // 设置用户组权限为读写
            newInode->blockNum = 4;

            // 为新文件分配四个块并清空它们
            for (int j = 0; j < 4; j++) {
                int newBlockID = allocateBlock();
                if (newBlockID == -1) {
                    // 如果无法分配足够的块，需要回收已分配的资源
                    for (int k = 0; k < j; k++) {  // 回收之前已分配的块
                        freeBlock(newInode->blockIDs[k]);
                        newInode->blockIDs[k] = -1;
                    }
                    freeInode(newInodeID);  // 回收inode
                    return false;  // 返回失败
                }
                newInode->blockIDs[j] = newBlockID;  // 将新分配的块ID保存到inode
                FileBlock* newBlock = reinterpret_cast<FileBlock*>(&blockMem[newBlockID]);
                memset(newBlock->content, 0, BLOCK_SIZE);  // 清空新分配的块
            }

            // 在父目录中添加新文件
            parentDirBlock->inodeID[i] = newInodeID;
            strcpy(parentDirBlock->fileName[i], newFileName.c_str());

            fileCreated = true;
            break;
        }
    }

    if (!fileCreated) {
        return false;  // 父目录没有空余位置
    }

    return true;
}



bool fileSystem::deleteDirectory(const char* path, const char* user) {//删除时，将文件所使用的块的内容清空
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        return false;  // 路径不能为空
    }    

    // 2. 查找要删除的目录的 inode
    Inode* targetInode = findInodeByPath(path);
    if (!targetInode) {
        return false;  // 目录不存在
    }

    // 3. 确保目标是一个目录
    if (targetInode->fileType != 0) {
        return false;  // 目标不是一个目录
    }

    // 4. 检查权限（根据实际情况处理）
    if (strcmp(targetInode->creator, user) != 0) {
        return false;  // 用户无权删除该目录
    }

    // 5. 检查目录是否为空
    DirectoryBlock* dirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[targetInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (dirBlock->inodeID[i] != -1) {
            return false;  // 目录不为空，不能删除
        }
    }

    

    // 6. 查找并更新父目录的 DirectoryBlock
   
    //获取路径的各个部分
    std::vector<std::string> pathParts = splitPath(path);
    // 获取父路径（除了最后一个部分）
    std::string parentPath = "";
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        parentPath += "/" + pathParts[i];  // 将除最后一个元素外的路径拼接成父路径
    }    
    // 获取删除目录的名字（最后一个部分）
    std::string targetDirName = pathParts.back();

    Inode* parentInode = findInodeByPath(parentPath.c_str());
    DirectoryBlock* parentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[parentInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (strcmp(parentDirBlock->fileName[i], targetDirName.c_str()) == 0) {  
            parentDirBlock->inodeID[i] = -1;
            strcpy(parentDirBlock->fileName[i], "nan");  // 设置为 "nan"
            break;
        }
    }

    // 7. 释放 inode 和相关 block
    freeInode(targetInode->inodeID);
    freeBlock(targetInode->blockIDs[0]);

    return true;
}
/*
bool fileSystem::writeFile(const char* path, const char* user, const std::string& context) {
    
    // 手动加锁
    
    // 检查数据大小是否符合限制
    if (context.size() > 4092) {
        std::cout << "Error: Data exceeds the maximum file size limit of 4092 bytes." << std::endl;
        
        return false;
    }

    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        std::cout << "Error: Path is invalid." << std::endl;
        
        return false;
    }

    // 2. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(path);
    if (!fileInode) {
        std::cout << "Error: File does not exist." << std::endl;
        
        return false;
    }

    // 3. 确保目标是一个文件
    if (fileInode->fileType != 1) {  // 假设文件类型为1
        std::cout << "Error: Target is not a file." << std::endl;
        
        return false;
    }

    // 4. 检查用户权限
    if (strcmp(fileInode->creator, user) != 0 && fileInode->permission[1] != W) {
        std::cout << "Error: User does not have write permission." << std::endl;
        
        return false;
    }

    // 5. 写入数据到文件
    size_t bytesWritten = 0;
    for (int i = 0; i < 4 && bytesWritten < context.size(); ++i) {
        int blockID = fileInode->blockIDs[i];
        if (blockID == -1) continue;  // 如果blockID为-1，跳过

        FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);
        size_t bytesToWrite = std::min(context.size() - bytesWritten, (size_t)1023);  // 最多写入1023字节

        memcpy(block->content, context.c_str() + bytesWritten, bytesToWrite);
        block->content[bytesToWrite] = '\0';  // 添加终止符
        bytesWritten += bytesToWrite;

        if (bytesWritten >= context.size()) break;  // 如果已写完所有数据，停止
    }

    // 6. 更新文件的修改时间和大小
    fileInode->modifiedTime = time(nullptr);
    fileInode->fileSize = std::min(static_cast<unsigned int>(context.size()), 4092u);  // 更新文件大小，不超过4092字节
   
    
    return true;
}
*/

//一个文件最多分配16个fileblock
bool fileSystem::writeFile(const char* path, const char* user, const std::string& context,bool flag = 1) {

    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        std::cout << "Error: Path is invalid." << std::endl;
        return false;
    }

    // 2. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(path);
    if (!fileInode) {
        std::cout << "Error: File does not exist." << std::endl;
        return false;
    }

    // 检查数据大小是否符合限制
    if (context.size() > 1023 * 16) {
        std::cout << "Error: Data exceeds the maximum file size limit of 16368 bytes." << std::endl;
        return false;
    }

    // 3. 确保目标是一个文件
    if (fileInode->fileType != 1) {  // 假设文件类型为1
        std::cout << "Error: Target is not a file." << std::endl;
        return false;
    }

    // 4. 检查用户权限
    if (strcmp(fileInode->creator, user) != 0 && fileInode->permission[1] != W && !isAdmin(user)){
        std::cout << "Error: User does not have write permission." << std::endl;
        return false;
    }

    size_t bytesWritten = 0;
    int blockIDs[16];  // 用于存储所有的块ID，包括直接块和间接块
    std::fill(std::begin(blockIDs), std::end(blockIDs), -1);  // 初始化 blockIDs 数组为 -1

    // 获取所有块ID
    int blockIndex = 0;
    for (int i = 0; i < 4 && i < fileInode->blockNum; ++i) {
        blockIDs[blockIndex++] = fileInode->blockIDs[i];
    }

    // 如果存在单级间接块，则将其中的块ID加载到blockIDs数组中
    if (fileInode->singleIndirectBlock != -1) {
        FileBlock* indirectBlock = reinterpret_cast<FileBlock*>(&blockMem[fileInode->singleIndirectBlock]);
        for (int i = 0; i < (fileInode->blockNum - 4) && blockIndex < 16; ++i) {
            int indirectBlockID = -1;
            memcpy(&indirectBlockID, &indirectBlock->content[i * sizeof(int)], sizeof(int));
            if (indirectBlockID != -1) {
                blockIDs[blockIndex++] = indirectBlockID;
            }
            
        }
    }

    // 判断是否需要扩容
    bool needsExpansion = false;
    if ( context.size() > fileInode->blockNum * 1023) {
        needsExpansion = true;
    }

    if (needsExpansion) {
        // 扩容时分配额外的块
        int expansionBlockCount = (((context.size() - fileInode->blockNum * 1023) - 1)/ 1023) + 1;
        int remainingBytes = context.size();

        // 分配额外的块，并将其写入到 blockIDs 中
        for (int i = 0; i < expansionBlockCount; ++i) {
            int newBlockID = allocateBlock();
            if (newBlockID == -1) {
                std::cout << "Error: Failed to allocate new block." << std::endl;
                return false;
            }

            // 写入新分配的块ID
            blockIDs[blockIndex++] = newBlockID;
            

            // 如果是第一次扩容，需要更新单级间接块
            if (fileInode->singleIndirectBlock == -1) {
                int newIndirectBlockID = allocateBlock();
                if (newIndirectBlockID == -1) {
                    std::cout << "Error: Failed to allocate block for single indirect block." << std::endl;
                    return false;
                }

                // 初始化新分配的间接块为0 
                FileBlock* indirectBlock = reinterpret_cast<FileBlock*>(&blockMem[newIndirectBlockID]);
                memset(indirectBlock->content, 0, BLOCK_SIZE);

                // 更新 inode 的 singleIndirectBlock
                fileInode->singleIndirectBlock = newIndirectBlockID;
            }

            // 通过 singleIndirectBlock 追加块ID
            FileBlock* indirectBlock = reinterpret_cast<FileBlock*>(&blockMem[fileInode->singleIndirectBlock]);
            int blockIDToWrite = blockIDs[blockIndex - 1];
            int blockOffset = blockIndex - 5;  // 假设单级间接块只能存储最多12个块ID（已使用4个直接块）

            // 将新的块ID写入单级间接块
            memcpy(&indirectBlock->content[blockOffset * sizeof(int)], &blockIDToWrite, sizeof(int));
        }
    }

    // 清空已分配块的内容
    for (int i = 0; i < blockIndex; ++i) {
        int blockID = blockIDs[i];
        if (blockID != -1) {
            FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);
            memset(block->content, 0, BLOCK_SIZE);  // 清空块内容
        }
    }

    // 写入数据
    while (bytesWritten < context.size()) {
        for (int i = 0; i < blockIndex && bytesWritten < context.size(); ++i) {
            int blockID = blockIDs[i];
            FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);

            size_t bytesToWrite = std::min(context.size() - bytesWritten, (size_t)1023);  // 最多写入1023字节
            memcpy(block->content, context.c_str() + bytesWritten, bytesToWrite);
            block->content[bytesToWrite] = '\0';  // 添加终止符
            bytesWritten += bytesToWrite;
        }
    }

    // 5. 更新文件的修改时间和大小
    fileInode->modifiedTime = time(nullptr);
    fileInode->fileSize = std::min(static_cast<unsigned int>(context.size()), 1023 * 16u);  // 更新文件大小，不超过16368字节
    fileInode->blockNum = blockIndex;


    //如果开启文件快照功能还应该写入快照文件
    if ( (fileInode->FS==1) && (flag==1) ) {
        std::string time = getCurrentDateTime();
        
        //找到空的操作空间,写入时间，操作符号
        int current = -1;
        for (int i = 0; i < MAX_FS; i++) {
            if (fileInode->FSoperation[i] == ' ') {
                current = i;
                strncpy(fileInode->FStime[i], time.c_str(), sizeof(fileInode->FStime[i]) - 1);
                fileInode->FStime[i][sizeof(fileInode->FStime[i]) - 1] = '\0'; // 确保以 '\0' 结尾
                fileInode->FSoperation[i] = 'W';
                break;
            }
        }
        if (current == -1) {
            std::cout << "this file snapshots number reach limit" << std::endl;
            return true;
        }
        

        //获取文件名
        std::vector<std::string> pathParts = splitPath(path);
        std::string targetFileName = pathParts.back();
        std::string targetPath = "/FS/" + targetFileName;       

        //创建文件
        std::string targetFilePath = "/FS/" + targetFileName + "/" + time + ".txt";
        createFile(targetFilePath.c_str(), user);

        //写入全量内容        
        writeFile(targetFilePath.c_str(), user, context,1);
    }
    
    return true;
}
/*
bool fileSystem::writeAppendFile(const char* path, const char* user, const std::string& context) {
    
    
    
    // 检查路径有效性和空内容
    if (path == nullptr || strlen(path) == 0 || context.empty()) {
        std::cerr << "Invalid path or empty content." << std::endl;
        
        return false;
    }

    // 找到对应的 inode
    Inode* fileInode = findInodeByPath(path);
    if (!fileInode) {
        std::cerr << "File does not exist." << std::endl;
        
        return false;
    }

    // 确认是文件并且用户有写权限
    if (fileInode->fileType != 1 || (strcmp(fileInode->creator, user) != 0 && fileInode->permission[1] != W)) {
        std::cerr << "No write permission or not a file." << std::endl;
        
        return false;
    }

    // 计算当前文件的总大小和需要的块数
    size_t currentSize = fileInode->fileSize;
    size_t newSize = currentSize + context.size();
    if (newSize > 4092) {  // 限制文件最大大小为4092字节
        std::cerr << "Exceeding file size limit." << std::endl;
        
        return false;
    }

    // 确定起始块和偏移量
    int startBlock = currentSize / 1023;
    int offset = currentSize % 1023;

    // 开始追加内容
    size_t bytesWritten = 0;
    while (bytesWritten < context.size()) {
        if (startBlock >= 4) {  // 最多只有4个块
            std::cerr << "Not enough blocks available." << std::endl;
            
            return false;
        }

        int blockID = fileInode->blockIDs[startBlock];
        if (blockID == -1) {  // 如果块未分配
            blockID = allocateBlock();
            if (blockID == -1) {
                std::cerr << "Failed to allocate block." << std::endl;
                
                return false;
            }
            fileInode->blockIDs[startBlock] = blockID;
        }

        FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);
        size_t spaceInBlock = 1023 - offset;  // 当前块剩余空间
        size_t toWrite = std::min(spaceInBlock, context.size() - bytesWritten);

        memcpy(block->content + offset, context.c_str() + bytesWritten, toWrite);
        block->content[offset + toWrite] = '\0';  // 维持字符串结尾
        bytesWritten += toWrite;
        offset = 0;  // 下一个块从0开始
        startBlock++;
    }

    // 更新文件大小
    fileInode->fileSize = newSize;
    fileInode->modifiedTime = time(nullptr);

    
    return true;
}
*/
bool fileSystem::writeAppendFile(const char* path, const char* user, const std::string& context) {
    
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        std::cout << "Error: Path is invalid." << std::endl;
        return false;
    }

    // 2. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(path);
    if (!fileInode) {
        std::cout << "Error: File does not exist." << std::endl;
        return false;
    }

    // 检查数据大小是否符合限制
    if (context.size() > 1023 * 16) {
        std::cout << "Error: Data exceeds the maximum file size limit of 16368 bytes." << std::endl;
        return false;
    }

    // 3. 确保目标是一个文件
    if (fileInode->fileType != 1) {  // 假设文件类型为1
        std::cout << "Error: Target is not a file." << std::endl;
        return false;
    }

    // 4. 检查用户权限
    if (strcmp(fileInode->creator, user) != 0 && fileInode->permission[1] != W && !isAdmin(user)) {
        std::cout << "Error: User does not have write permission." << std::endl;
        return false;
    }

    //如果开启文件快照功能应该写入快照文件
    if (fileInode->FS == 1) {
        std::string time = getCurrentDateTime();

        //找到空的操作空间,写入时间，操作符号
        int current = -1;
        for (int i = 0; i < MAX_FS; i++) {
            if (fileInode->FSoperation[i] == ' ') {
                current = i;
                strncpy(fileInode->FStime[i], time.c_str(), sizeof(fileInode->FStime[i]) - 1);
                fileInode->FStime[i][sizeof(fileInode->FStime[i]) - 1] = '\0'; // 确保以 '\0' 结尾
                fileInode->FSoperation[i] = 'A';
                break;
            }
        }
        if (current == -1) {
            std::cout << "this file snapshots number reach limit" << std::endl;
            return true;
        }


        //获取文件名
        std::vector<std::string> pathParts = splitPath(path);
        std::string targetFileName = pathParts.back();
        std::string targetPath = "/FS/" + targetFileName;

        //创建文件
        std::string targetFilePath = "/FS/" + targetFileName + "/" + time + ".txt";
        createFile(targetFilePath.c_str(), user);

        //写入增量内容        
        writeFile(targetFilePath.c_str(), user, context,1);
    }

    // 读取之前的内容
    std::string existingContent = readFile(path, user);
    if (existingContent.empty() && !context.empty()) {
        std::cout << "File is empty, write with provided content." << std::endl;
    }

    // 聚合成新内容
    std::string newContent = existingContent + context;
    // 重新写入
    return writeFile(path, user, newContent, 0);
}

/*
std::string fileSystem::readFile(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        std::cerr << "Error: Path is invalid." << std::endl;
        return ""; // 返回空字符串
    }

    // 2. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(path);

    if (fileInode->fileSize == 0) {
        std::cout << "this file is empty" << std::endl;
        return "";
    }
    if (!fileInode) {
        std::cerr << "Error: File does not exist." << std::endl;
        return ""; // 文件不存在时返回空字符串
    }

    // 3. 确保目标是一个文件
    if (fileInode->fileType != 1) { // 假设文件类型为1
        std::cerr << "Error: Target is not a file." << std::endl;
        return "";
    }

    // 4. 检查用户权限
    if (strcmp(fileInode->creator, user) != 0 && fileInode->permission[0] == N) {
        std::cerr << "Error: User does not have read permission." << std::endl;
        return "";
    }

    // 5. 读取文件数据
    std::stringstream content;
    for (int i = 0; i < fileInode->blockNum; i++) {
        int blockID = fileInode->blockIDs[i];
        if (blockID == -1) continue; // 跳过未分配的块

        FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);
        content << block->content; // 将块内容追加到字符串流
    }

    // 返回读取到的内容
    return content.str();
}
*/
std::string fileSystem::readFile(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        std::cerr << "Error: Path is invalid." << std::endl;
        return ""; // 返回空字符串
    }

    // 2. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(path);
    if (!fileInode) {
        std::cerr << "Error: File does not exist." << std::endl;
        return ""; // 文件不存在时返回空字符串
    }

    // 如果文件大小为0，直接返回提示
    if (fileInode->fileSize == 0) {
        //std::cout << "This file is empty." << std::endl;
        return "";
    }

    // 3. 确保目标是一个文件
    if (fileInode->fileType != 1) { // 假设文件类型为1
        std::cerr << "Error: Target is not a file." << std::endl;
        return "";
    }

    // 4. 检查用户权限
    if (strcmp(fileInode->creator, user) != 0 && fileInode->permission[0] == N && !isAdmin(user)) {
        std::cerr << "Error: User does not have read permission." << std::endl;
        return "";
    }

    // 5. 读取文件数据
    std::stringstream content;

    // 用于存储完整的块ID，包括直接块和间接块
    int blockIDs[16];
    std::fill(std::begin(blockIDs), std::end(blockIDs), -1);  // 初始化 blockIDs 数组为 -1
    int blockIndex = 0;

    // 获取直接块
    for (int i = 0; i < 4 && i < fileInode->blockNum; ++i) {
        blockIDs[blockIndex++] = fileInode->blockIDs[i];
    }

    // 如果存在单级间接块，加载间接块中的ID
    if (fileInode->singleIndirectBlock != -1) {
        FileBlock* indirectBlock = reinterpret_cast<FileBlock*>(&blockMem[fileInode->singleIndirectBlock]);
        for (int i = 0; i < (fileInode->blockNum - 4) && blockIndex < 16; ++i) {
            int indirectBlockID;
            memcpy(&indirectBlockID, &indirectBlock->content[i * sizeof(int)], sizeof(int));
            if (indirectBlockID != -1) {
                blockIDs[blockIndex++] = indirectBlockID;
            }
        }
    }

    // 读取所有块的内容
    for (int i = 0; i < fileInode->blockNum && i < 16; ++i) {
        int blockID = blockIDs[i];
        if (blockID == -1) continue;  // 跳过未分配的块

        FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[blockID]);
        content << block->content; // 将块内容追加到字符串流
    }

    // 返回读取到的内容
    return content.str();
}
/*
bool fileSystem::deleteFile(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        return false;  // 路径不能为空
    }

    // 2. 查找要删除的文件的 inode
    Inode* targetInode = findInodeByPath(path);
    if (!targetInode) {
        return false;  // 文件不存在
    }

    // 3. 确保目标是一个文件
    if (targetInode->fileType != 1) {
        return false;  // 目标不是一个文件
    }

    // 4. 检查权限
    if (strcmp(targetInode->creator, user) != 0) {
        return false;  // 用户无权删除该文件
    }

    // 5. 查找并更新父目录的 DirectoryBlock
    std::vector<std::string> pathParts = splitPath(path);
    std::string parentPath = "";
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        parentPath += "/" + pathParts[i];
    }
    std::string targetFileName = pathParts.back();

    Inode* parentInode = findInodeByPath(parentPath.c_str());
    DirectoryBlock* parentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[parentInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (strcmp(parentDirBlock->fileName[i], targetFileName.c_str()) == 0) {
            parentDirBlock->inodeID[i] = -1;
            strcpy(parentDirBlock->fileName[i], "nan");  // 标记为"nan"或其他空标记
            break;
        }
    }

    // 6. 释放 inode 和相关的四个 block
    for (int i = 0; i < 4; i++) { // 每个文件都分配了四个 block
        if (targetInode->blockIDs[i] != -1) {
            freeBlock(targetInode->blockIDs[i]);
            targetInode->blockIDs[i] = -1;
        }
    }
    freeInode(targetInode->inodeID);

    return true;
}
*/

bool fileSystem::deleteFile(const char* path, const char* user) {
    // 1. 确保路径有效
    if (path == nullptr || strlen(path) == 0) {
        return false;  // 路径不能为空
    }

    // 2. 查找要删除的文件的 inode
    Inode* targetInode = findInodeByPath(path);
    if (!targetInode) {
        return false;  // 文件不存在
    }

    // 3. 确保目标是一个文件
    if (targetInode->fileType != 1) {
        return false;  // 目标不是一个文件
    }

    // 4. 检查权限
    if (strcmp(targetInode->creator, user) != 0) {
        return false;  // 用户无权删除该文件
    }

    // 5. 查找并更新父目录的 DirectoryBlock
    std::vector<std::string> pathParts = splitPath(path);
    std::string parentPath = "";
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        parentPath += "/" + pathParts[i];
    }
    std::string targetFileName = pathParts.back();

    Inode* parentInode = findInodeByPath(parentPath.c_str());
    if (!parentInode) {
        return false;  // 父目录不存在
    }

    DirectoryBlock* parentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[parentInode->blockIDs[0]]);
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (strcmp(parentDirBlock->fileName[i], targetFileName.c_str()) == 0) {
            parentDirBlock->inodeID[i] = -1;
            strcpy(parentDirBlock->fileName[i], "nan");  // 标记为"nan"或其他空标记
            break;
        }
    }

    // 6. 释放 inode 和相关的块
    // 释放直接分配的块
    for (int i = 0; i < 4; ++i) {  // 每个文件最多分配了4个直接块
        if (targetInode->blockIDs[i] != -1) {
            freeBlock(targetInode->blockIDs[i]);
            targetInode->blockIDs[i] = -1;
        }
    }

    // 如果存在单级间接块，释放其中的块
    if (targetInode->singleIndirectBlock != -1) {
        FileBlock* indirectBlock = reinterpret_cast<FileBlock*>(&blockMem[targetInode->singleIndirectBlock]);
        // 释放间接块中存储的每个块ID
        for (int i = 0; i < (targetInode->blockNum - 4); ++i) {
            int blockID;
            memcpy(&blockID, &indirectBlock->content[i * sizeof(int)], sizeof(int));
            if (blockID != -1) {
                freeBlock(blockID);  // 释放间接块指向的块
            }
        }
        // 释放间接块本身
        freeBlock(targetInode->singleIndirectBlock);
        targetInode->singleIndirectBlock = -1;
    }

    // 释放 inode
    freeInode(targetInode->inodeID);

    return true;
}



std::vector<std::string> fileSystem::splitPath(const char* path) {
    std::vector<std::string> pathParts;
    std::string pathStr(path);

    // 如果路径是空的，直接返回空的 vector
    if (pathStr.empty()) {
        return pathParts;
    }

    // 去掉路径开头的 '/'
    if (pathStr[0] == '/') {
        pathStr = pathStr.substr(1);
    }

    // 根据 '/' 分割路径
    std::stringstream ss(pathStr);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            pathParts.push_back(part);  // 将每个部分添加到结果中
        }
    }

    return pathParts;
}

Inode* fileSystem::findInodeByPath(const char* path) {
    // 将路径拆分为多个部分
    std::vector<std::string> pathParts = splitPath(path);

    Inode* currentInode = &inodeMem[0];  // 从根目录开始，根目录的 inode 是 0

    for (const auto& part : pathParts) {
        bool found = false;

        // 查找当前目录中的文件/子目录
       
        DirectoryBlock* currentDirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[currentInode->blockIDs[0]]);

        // 遍历当前目录块中的所有文件名
        for (int i = 0; i < ENTRY_NUMBER; ++i) {
            if (currentDirBlock->inodeID[i] != -1 && strcmp(currentDirBlock->fileName[i], part.c_str()) == 0) {
                // 找到匹配的文件/目录
                currentInode = &inodeMem[currentDirBlock->inodeID[i]];
                found = true;
                break;
            }
        }

        if (!found) {
            return nullptr;  // 如果没有找到路径部分，返回空
        }
    }

    return currentInode;  // 返回找到的 inode
}

std::string fileSystem::displayDirectory(const char* path) {
    // 1. 查找路径对应的 Inode
    Inode* inode = findInodeByPath(path);
    if (!inode) {
        return "Directory not found.\n";  // 路径不存在
    }

    // 2. 确保路径是一个目录
    if (inode->fileType != 0) {
        return "The specified path is not a directory.\n";
    }

    // 3. 获取目录块
    DirectoryBlock* dirBlock = reinterpret_cast<DirectoryBlock*>(&blockMem[inode->blockIDs[0]]);

    // 4. 构建目录内容字符串
    std::string directoryListing = "Directory contents of " + std::string(path) + ":\n";
    for (int i = 0; i < ENTRY_NUMBER; ++i) {
        if (dirBlock->inodeID[i] != -1) {  // 如果该目录项存在
            directoryListing += "- " + std::string(dirBlock->fileName[i]) + "\n";
        }
    }

    return directoryListing;
}


void fileSystem::freeInode(int inodeID) {
    int byteIndex = inodeID / 8;
    int bitIndex = inodeID % 8;
    inodeBitmap[byteIndex] &= ~(1 << bitIndex);  // 清除位图中对应的位
}

void fileSystem::freeBlock(int blockID) {
    int byteIndex = blockID / 8;
    int bitIndex = blockID % 8;
    blockBitmap[byteIndex] &= ~(1 << bitIndex);  // 清除位图中对应的位
}

bool fileSystem::saveFileSystem(const std::string& filePath) {
    // 打开文件以写入数据
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Unable to open file for saving the file system." << std::endl;
        return false;  // 打开文件失败
    }

    // 1. 保存 inode 位图
    outFile.write(reinterpret_cast<char*>(inodeBitmap), sizeof(inodeBitmap));
    if (!outFile) {
        std::cerr << "Error: Failed to write inode bitmap to file." << std::endl;
        return false;
    }

    // 2. 保存块位图
    outFile.write(reinterpret_cast<char*>(blockBitmap), sizeof(blockBitmap));
    if (!outFile) {
        std::cerr << "Error: Failed to write block bitmap to file." << std::endl;
        return false;
    }

    // 3. 保存 inode 数组
    outFile.write(reinterpret_cast<char*>(inodeMem), sizeof(inodeMem));
    if (!outFile) {
        std::cerr << "Error: Failed to write inode array to file." << std::endl;
        return false;
    }

    // 4. 保存目录块（blockMem）
    outFile.write(reinterpret_cast<char*>(blockMem), sizeof(blockMem));
    if (!outFile) {
        std::cerr << "Error: Failed to write block array to file." << std::endl;
        return false;
    }

    // 保存成功
    outFile.close();
    std::cout << "File system successfully saved to " << filePath << std::endl;
    return true;
}

bool fileSystem::loadFileSystem(const std::string& filePath) {
    // 打开文件以读取数据
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Unable to open file for loading the file system." << std::endl;
        return false;  // 打开文件失败
    }

    // 1. 读取 inode 位图
    inFile.read(reinterpret_cast<char*>(inodeBitmap), sizeof(inodeBitmap));
    if (!inFile) {
        std::cerr << "Error: Failed to read inode bitmap from file." << std::endl;
        return false;
    }

    // 2. 读取块位图
    inFile.read(reinterpret_cast<char*>(blockBitmap), sizeof(blockBitmap));
    if (!inFile) {
        std::cerr << "Error: Failed to read block bitmap from file." << std::endl;
        return false;
    }

    // 3. 读取 inode 数组
    inFile.read(reinterpret_cast<char*>(inodeMem), sizeof(inodeMem));
    if (!inFile) {
        std::cerr << "Error: Failed to read inode array from file." << std::endl;
        return false;
    }

    // 4. 读取目录块（blockMem）
    inFile.read(reinterpret_cast<char*>(blockMem), sizeof(blockMem));
    if (!inFile) {
        std::cerr << "Error: Failed to read block array from file." << std::endl;
        return false;
    }

    // 成功读取文件系统
    inFile.close();
    std::cout << "File system successfully loaded from " << filePath << std::endl;
    return true;
}


bool fileSystem::saveBackUp() {
    // 获取当前时间戳作为备份文件名
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::string backupFileName = "backup/" + std::to_string(time_t_now) + ".dat";

    // 打开文件并保存文件系统
    std::ofstream backupFile(backupFileName, std::ios::binary);
    if (!backupFile.is_open()) {
        std::cerr << "Failed to open backup file for writing." << std::endl;
        return false;
    }

    // 保存 inode 数据
    backupFile.write(reinterpret_cast<char*>(inodeMem), sizeof(inodeMem));
    backupFile.write(inodeBitmap, sizeof(inodeBitmap));
    backupFile.write(blockBitmap, sizeof(blockBitmap));
    backupFile.write(reinterpret_cast<char*>(blockMem), sizeof(blockMem));

    backupFile.close();
    std::cout << "Backup saved to " << backupFileName << std::endl;

    // 更新备份文件名列表
    updateBackupFileList();

    return true;
}

void fileSystem::updateBackupFileList() {
    // 读取备份文件夹中的所有文件
    std::filesystem::path backupDir("backup");
    std::vector<std::string> backupFiles;

    // 检查备份文件夹是否存在
    if (!std::filesystem::exists(backupDir)) {
        std::cerr << "Backup directory does not exist!" << std::endl;
        return;
    }

    // 遍历备份文件夹，收集文件路径
    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file()) {
            backupFiles.push_back(entry.path().string());
        }
    }

    // 按照文件创建时间排序（从最旧到最新）
    std::sort(backupFiles.begin(), backupFiles.end(),
        [](const std::string& a, const std::string& b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });

    // 如果备份文件数量超过最大数量，删除最旧的文件
    while (backupFiles.size() > MAX_BACKUPFILE_SIZE) {
        std::filesystem::remove(backupFiles.front());
        backupFiles.erase(backupFiles.begin());
    }

    // 将备份文件名存入固定大小的字符串数组
    for (size_t i = 0; i < backupFiles.size() && i < MAX_BACKUPFILE_SIZE; ++i) {
        backupFileName[i] = backupFiles[i];  // 将文件名存储到 backupFileName 数组
    }

    // 如果有空余位置，清空未使用的文件名
    for (size_t i = backupFiles.size(); i < MAX_BACKUPFILE_SIZE; ++i) {
        backupFileName[i].clear();  // 清空未使用的字符串
    }
}
/*
void fileSystem::listBackUp() {
    // 读取备份文件夹中的所有文件
    std::filesystem::path backupDir("backup");
    std::vector<std::string> backupFiles;

    // 检查备份文件夹是否存在
    if (!std::filesystem::exists(backupDir)) {
        std::cerr << "Backup directory does not exist!" << std::endl;
        return;
    }

    // 遍历备份文件夹，收集文件路径
    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file()) {
            backupFiles.push_back(entry.path().string());
        }
    }

    // 按照文件创建时间排序（从最旧到最新）
    std::sort(backupFiles.begin(), backupFiles.end(),
        [](const std::string& a, const std::string& b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });

    // 输出备份文件名
    std::cout << "Backup files:" << std::endl;
    size_t count = 0;
    for (const auto& file : backupFiles) {
        if (count >= MAX_BACKUPFILE_SIZE) break;  // 超过最大备份文件数量
        std::cout << file << std::endl;
        ++count;
    }
}
*/
std::string fileSystem::listBackUp() {
    // 读取备份文件夹中的所有文件
    std::filesystem::path backupDir("backup");
    std::vector<std::string> backupFiles;

    // 检查备份文件夹是否存在
    if (!std::filesystem::exists(backupDir)) {
        return "Backup directory does not exist!";
    }

    // 遍历备份文件夹，收集文件路径
    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file()) {
            backupFiles.push_back(entry.path().string());
        }
    }

    // 按照文件创建时间排序（从最旧到最新）
    std::sort(backupFiles.begin(), backupFiles.end(),
        [](const std::string& a, const std::string& b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });

    // 拼接备份文件名到字符串
    std::stringstream result;
    result << "Backup files:" << std::endl;
    size_t count = 0;
    for (const auto& file : backupFiles) {
        if (count >= MAX_BACKUPFILE_SIZE) break;  // 超过最大备份文件数量
        result << file << std::endl;
        ++count;
    }

    return result.str();
}

bool fileSystem::loadBackUp(const std::string& filePath){
    std::ifstream backupFile(filePath, std::ios::binary);
    if (!backupFile.is_open()) {
        std::cerr << "Failed to open backup file for loading." << std::endl;
        return false;
    }

    // 加载 inode 数据
    backupFile.read(reinterpret_cast<char*>(inodeMem), sizeof(inodeMem));
    backupFile.read(inodeBitmap, sizeof(inodeBitmap));
    backupFile.read(blockBitmap, sizeof(blockBitmap));
    backupFile.read(reinterpret_cast<char*>(blockMem), sizeof(blockMem));

    backupFile.close();
    std::cout << "File system loaded from " << filePath << std::endl;

    return true;
}

fileSystem::~fileSystem() {
    saveFileSystem(FILE_NAME);
}



bool fileSystem::createUser(int usertype, std::string username, std::string password) {
    // 检查用户名和密码合法性，不能包含 ',' 和 '\\'
    if (username.find(',') != std::string::npos || username.find('\\') != std::string::npos) {
        std::cout << "Invalid username: cannot contain ',' or '\\'." << std::endl;
        return false;
    }

    if (password.find(',') != std::string::npos || password.find('\\') != std::string::npos) {
        std::cout << "Invalid password: cannot contain ',' or '\\'." << std::endl;
        return false;
    }

    /*
    if (usertype == ADMIN) {
        if (isAdmin(username)){
            std::cout << "This user name already exists" << std::endl;
            return false;
        }
    }
    else if (usertype == DOCTOR) {
        if (isDoctor(username)) {
            std::cout << "This user name already exists" << std::endl;
            return false;
        }
    }
    else if (usertype == PATIENT) {
        if (isPatient(username)) {
            std::cout << "This user name already exists" << std::endl;
            return false;
        }
    }
    else {
        std::cout << "Invalid usertype" << std::endl;
        return false;
    }
    */

    //使用户名唯一
    if (isUser(username)) {
        std::cout << "This user name already exists" << std::endl;
        return false;
    }

    std::string context = username + "," + password + "\n";

    // 根据用户类型，选择写入不同的文件
    if (usertype == ADMIN) {
        writeAppendFile("user/admin.txt", "admin", context);
    }
    else if (usertype == DOCTOR) {
        writeAppendFile("user/doctor.txt", "admin", context);
    }
    else if (usertype == PATIENT) {
        writeAppendFile("user/patient.txt", "admin", context);
        creatRecords(username.c_str());
    }
    else {
        std::cout << "Invalid usertype" << std::endl;
        return false;
    }

    std::cout << "User "+ username + " Create Successful" << std::endl;
    return true;
}

bool fileSystem::userLogin(int usertype, std::string username, std::string password) {
    std::string fileName;
    // 根据用户类型选择对应的文件
    if (usertype == ADMIN) {
        fileName = "user/admin.txt";
    }
    else if (usertype == DOCTOR) {
        fileName = "user/doctor.txt";
    }
    else if (usertype == PATIENT) {
        fileName = "user/patient.txt";
    }
    else {
        std::cout << "Invalid usertype" << std::endl;
        return false;
    }

    // 读取文件内容
    std::string fileContent = readFile(fileName.c_str(), "admin");
    if (fileContent.empty()) {
        std::cout << "user file is empty: " << fileName << std::endl;
        return false;
    }

    // 逐行处理文件内容
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名
            std::string filePassword = line.substr(pos + 1); // 提取密码

            // 如果用户名和密码匹配
            if (fileUsername == username && filePassword == password) {
                std::cout << "Login successful!" << std::endl;
                return true;  // 登录成功
            }
        }
    }

    std::cout << "Invalid username or password!" << std::endl;
    return false;  // 登录失败
}
bool fileSystem::changePassword(int usertype, const std::string& username, const std::string& newPassword) {
    // 检查用户名和密码合法性，不能包含 ',' 和 '\\'
    if (username.find(',') != std::string::npos || username.find('\\') != std::string::npos) {
        std::cout << "Invalid username: cannot contain ',' or '\\'." << std::endl;
        return false;
    }

    if (newPassword.find(',') != std::string::npos || newPassword.find('\\') != std::string::npos) {
        std::cout << "Invalid password: cannot contain ',' or '\\'." << std::endl;
        return false;
    }

    // 如果是管理员用户并且尝试修改 "admin" 的密码，拒绝操作
    if (usertype == ADMIN && username == "admin") {
        std::cout << "Error: Cannot change the password for super admin user." << std::endl;
        return false;
    }

    // 根据用户类型选择文件
    std::string fileName;
    if (usertype == ADMIN) {
        fileName = "user/admin.txt";
    }
    else if (usertype == DOCTOR) {
        fileName = "user/doctor.txt";
    }
    else if (usertype == PATIENT) {
        fileName = "user/patient.txt";
    }
    else {
        std::cout << "Invalid usertype" << std::endl;
        return false;
    }

    // 读取文件内容
    std::string fileContent = readFile(fileName.c_str(), "admin");
    if (fileContent.empty()) {
        std::cout << "user file is empty: " << fileName << std::endl;
        return false;
    }

    // 用于保存修改后的文件内容
    std::stringstream updatedContent;
    bool userFound = false;

    // 遍历数据每一行，将每一行写入，遇到对应的账号队密码进行替换
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名
            std::string filePassword = line.substr(pos + 1); // 提取密码

            // 如果用户名匹配，修改密码
            if (fileUsername == username) {
                filePassword = newPassword;  // 修改密码
                userFound = true;
            }

            // 将修改后的用户名和密码添加到新的内容中
            updatedContent << fileUsername << "," << filePassword << "\n";
        }
    }

    // 如果找到了匹配的用户名，更新文件
    if (userFound) {
        // 使用 writeFile 写回文件
        return writeFile(fileName.c_str(), "admin", updatedContent.str(),1);
    }
    else {
        std::cout << "User " << username << " not found." << std::endl;
        return false;  // 没有找到该用户
    }
}



bool fileSystem::changeFilePermission(const std::string& username, const std::string& path, char generalPermission, char groupPermission) {
    Inode* fileInode = findInodeByPath(path.c_str());  // 查找文件的 Inode

    // 检查文件是否存在
    if (!fileInode) {
        std::cerr << "Error: File not found." << std::endl;
        return false;
    }

    // 检查用户是否有权限更改文件权限
    if (strcmp(fileInode->creator, username.c_str()) != 0 && !isAdmin(username)) {
        std::cerr << "Error: You do not have permission to change file permissions." << std::endl;
        return false;
    }

    // 更改权限
    fileInode->permission[0] = generalPermission;  // 一般用户权限
    fileInode->permission[1] = groupPermission;    // 用户组权限

    std::cout << "File permissions updated successfully." << std::endl;
    return true;
}

bool fileSystem::isAdmin(const std::string& username) {
    // 读取管理员文件内容
    std::string fileContent = readFile("user/admin.txt", "admin");
    if (fileContent.empty()) {
        std::cerr << "admin  file is empty" << std::endl;
        return false;
    }

    // 逐行处理文件内容
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名

            // 如果用户名匹配
            if (fileUsername == username) {
                return true;  // 用户是管理员
            }
        }
    }

    // 没有找到匹配的用户名
    return false;  // 用户不是管理员
}

bool fileSystem::isDoctor(const std::string& username) {
    // 读取医生文件内容
    std::string fileContent = readFile("user/doctor.txt", "admin");
    if (fileContent.empty()) {
        //std::cerr << "doctor file is empty" << std::endl;
        return false;
    }

    // 逐行处理文件内容
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名

            // 如果用户名匹配
            if (fileUsername == username) {
                return true;  // 用户是医生
            }
        }
    }

    // 没有找到匹配的用户名
    return false;  // 用户不是医生
}

bool fileSystem::isPatient(const std::string& username) {
    // 读取病人文件内容
    std::string fileContent = readFile("user/patient.txt", "admin");
    if (fileContent.empty()) {
        std::cerr << "patient file is empty" << std::endl;
        return false;
    }

    // 逐行处理文件内容
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名

            // 如果用户名匹配
            if (fileUsername == username) {
                return true;  // 用户是病人
            }
        }
    }

    // 没有找到匹配的用户名
    return false;  // 用户不是病人
}

bool fileSystem::deleteUser(const std::string& operatorName, int deletingUserType, const std::string& usernameToDelete) {
    // 1. 确保操作者是管理员
    if (!isAdmin(operatorName)) {
        std::cout << "Error: Only administrators can delete users." << std::endl;
        return false;
    }

    // 2. 禁止删除名为 "admin" 的管理员
    if (usernameToDelete == "admin") {
        std::cout << "Error: Cannot delete the super admin user." << std::endl;
        return false;
    }

    // 3. 只有 admin 用户可以删除其他管理员
    if (deletingUserType == ADMIN) {
        if (operatorName != "admin") {
            std::cout << "Error: Only the super admin user can delete other admins." << std::endl;
            return false;
        }
    }

    // 4. 根据删除的用户类型，选择文件路径
    std::string fileName;
    if (deletingUserType == ADMIN) {
        fileName = "user/admin.txt";
    }
    else if (deletingUserType == DOCTOR) {
        fileName = "user/doctor.txt";
    }
    else if (deletingUserType == PATIENT) {
        fileName = "user/patient.txt";
        deleteRecords(usernameToDelete.c_str());//删除其记录文件
    }
    else {
        std::cout << "Invalid usertype" << std::endl;
        return false;
    }

    // 5. 读取文件内容
    std::string fileContent = readFile(fileName.c_str(), "admin");
    if (fileContent.empty()) {
        std::cout << "user file is empty: " << fileName << std::endl;
        return false;
    }

    // 6. 用于保存更新后的文件内容
    std::stringstream updatedContent;
    bool userFound = false;

    // 7. 逐行处理文件内容，删除目标用户
    std::stringstream ss(fileContent);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(',');  // 查找用户名和密码的分隔符
        if (pos != std::string::npos) {
            std::string fileUsername = line.substr(0, pos);  // 提取用户名
            if (fileUsername == usernameToDelete) {
                userFound = true;  // 找到要删除的用户
                continue;  // 跳过此行，表示删除该用户
            }
            updatedContent << line << "\n";  // 保留其他用户
        }
    }

    // 8. 如果找到了匹配的用户名，更新文件
    if (userFound) {
        // 如果删除后的内容为空，也写入空内容
        if (updatedContent.str().empty()) {            
            // 如果文件为空，可以直接清空 block 内容
            Inode* fileInode = findInodeByPath(fileName.c_str());
            if (fileInode) {
                // 清空所有块的内容
                for (int i = 0; i < BLOCKS_PER_INODE; ++i) {
                    if (fileInode->blockIDs[i] != -1) {
                        FileBlock* block = reinterpret_cast<FileBlock*>(&blockMem[fileInode->blockIDs[i]]);
                        memset(block->content, 0, BLOCK_SIZE);  // 清空该块的内容                        
                    }
                }
            }
            fileInode->fileSize = 0;
        }
        else {
            // 否则，写入更新后的内容
            writeFile(fileName.c_str(), "admin", updatedContent.str(),1);
        }
        return true;
    }
    else {
        std::cout << "User " << usernameToDelete << " not found." << std::endl;
        return false;  // 没有找到该用户
    }
}

bool fileSystem::isUser(const std::string& username) {
    return isAdmin(username) || isDoctor(username) || isPatient(username);
}

bool fileSystem::changeFileOwner(const std::string& filePath, const std::string& oper, const std::string& newOwner) {
    // 1. 查找文件对应的 inode
    Inode* fileInode = findInodeByPath(filePath.c_str());
    if (!fileInode) {
        std::cerr << "Error: File not found!" << std::endl;
        return false;  // 文件不存在
    }

    std::string oldOwner = fileInode->creator;

    // 2. 验证原拥有者是否与 inode 的 creator 匹配
    if ((oper != oldOwner) && !isAdmin(oper)) {
        std::cerr << "Error: Original owner mismatch and oper is not a admin!" << std::endl;
        return false;  // 原拥有者不匹配
    }

    // 3. 验证新拥有者是否是有效用户   
    if (!isUser(newOwner)) {
        std::cerr << "Error: Invalid new owner!" << std::endl;
        return false;  // 新拥有者无效
    }

    // 4. 更改文件的拥有者
    strcpy(fileInode->creator, newOwner.c_str());  // 更新文件的创建者为新拥有者     

    std::cout << filePath << "Owner changed successfully :" << "from" << oldOwner << "to" << newOwner << std::endl;
    return true;
}

// 调整文件的用户组，增加或删除用户
bool fileSystem::adjustUserGroup(const std::string& filePath, const std::string& operatorName, const std::string& targetUsername, bool addUser) {
    // 1. 查找文件的 inode
    Inode* fileInode = findInodeByPath(filePath.c_str());
    if (!fileInode) {
        std::cerr << "Error: File not found!" << std::endl;
        return false;  // 文件不存在
    }

    // 2. 验证操作者权限（管理员或文件的拥有者）
    if ( !isAdmin(operatorName) && fileInode->creator != operatorName) {
        std::cerr << "Error: Only administrators or the file owner can modify the user group!" << std::endl;
        return false;
    }

    // 3. 验证目标用户是否有效
    if (!isUser(targetUsername)) {
        std::cerr << "Error: Invalid target user!" << std::endl;
        return false;  // 目标用户无效
    }

    // 4. 根据 addUser 参数决定是增加用户还是删除用户
    if (addUser) {
        // 5. 添加用户到用户组
        bool userAdded = false;
        for (int i = 0; i < MAX_GROUP_NUM; ++i) {
            if (fileInode->group[i][0] == '\0') {  // 找到空的位置
                strcpy(fileInode->group[i], targetUsername.c_str());
                userAdded = true;
                break;
            }
        }

        if (!userAdded) {
            std::cout << "Error: User group is full!" << std::endl;
            return false;  // 用户组已满
        }

        std::cout << "User " << targetUsername << " added to the group." << std::endl;

    }
    else {
        // 6. 从用户组中删除用户
        bool userFound = false;
        for (int i = 0; i < MAX_GROUP_NUM; ++i) {
            if (fileInode->group[i] == targetUsername) {  // 找到目标用户                
                // 删除用户
                fileInode->group[i][0] = '\0';  // 清空该位置
                userFound = true;
                std::cout << "User " << targetUsername << " removed from the group." << std::endl;
                break;
            }
        }

        if (!userFound) {
            std::cout << "Error: User not found in the group!" << std::endl;
            return false;  // 未找到该用户
        }
    }    

    return true;
}

bool fileSystem::creatRecords(const char* username) {    
    //直接创建
    //路径：/records/user.txt
    std::string path = "/records/" + std::string(username) + ".txt"; 
    return createFile(path.c_str(), "admin"); 
    
}

bool fileSystem::deleteRecords(const char* username) {
    //直接删除文件/records/user.txt
    std::string path = "/records/" + std::string(username) + ".txt";
    return deleteFile(path.c_str(), "admin");
}

bool fileSystem::writeAppendRecords(const char* Pname, const char* Dname, const std::string& context) {
    //写入目标文件/records/Pname.txt,先判断Dname是否为医生，权限直接用admin，写入时要写入Dname作为记录
    //写入格式 Time "\n" + Dname + ":\n" + context
    if (!isDoctor(Dname)) {
        return 0;
    }
    if (!isPatient(Pname)) {
        return 0;
    }
    std::string path = "/records/" + std::string(Pname) + ".txt";
    std::string header = "\n" + getCurrentDateTime() + "\n" + std::string(Dname) + ":\n";
    std::string toWrite = header + context + "\n";
    return writeAppendFile(path.c_str(), "admin", toWrite);
}

std::string fileSystem::listRecords() {
    std::string path = "/records/";
    return displayDirectory(path.c_str());
}

std::string fileSystem::readRecords(const char* recordsName) {
    std::string path = "/records/" + std::string(recordsName);
    return readFile(path.c_str(), "admin");
}

bool fileSystem::releaseAppointments(const char* Dname, std::string year, std::string month, std::string day) {
    
    if (!isDoctor(Dname)) {
        return 0;
    }

    std::ostringstream path;
    path << "/appointments/" << Dname << "-" // Assuming appointments are stored in /appointments directory
        << std::setw(4) << std::setfill('0') << year << "-" // Formats year as four digits
        << std::setw(2) << std::setfill('0') << month << "-" // Formats month as two digits
        << std::setw(2) << std::setfill('0') << day << ".txt"; // Formats day as two digits and adds file extension

    std::string fullPath = path.str(); // Converts the ostringstream to string
   
    return createFile(fullPath.c_str(), Dname); // 

}
bool fileSystem::revocationAppointments(const char* Dname, std::string year, std::string month, std::string day) {
    
    if (!isDoctor(Dname)) {
        return 0;
    }

    std::ostringstream path;
    path << "/appointments/" << Dname << "-" // Assuming appointments are stored in /appointments directory
        << std::setw(4) << std::setfill('0') << year << "-" // Formats year as four digits
        << std::setw(2) << std::setfill('0') << month << "-" // Formats month as two digits
        << std::setw(2) << std::setfill('0') << day << ".txt"; // Formats day as two digits and adds file extension

    std::string fullPath = path.str(); // Converts the ostringstream to string

    return deleteFile(fullPath.c_str(), Dname); // 
}


bool fileSystem::writeAppointments(const char* Pname, const char* Dname, std::string year, std::string month, std::string day) {
    /*
    预约写入逻辑：
    先读文件，如果人数（行数，因为每人一行）超过8人，那么不予以写入；
    如果没有超过则允许写入；同时检查患者是否已经预约，避免重复预约。
    */

    // 构造医生的预约文件路径
    std::ostringstream path;
    path << "/appointments/" << Dname << "-"
        << std::setw(4) << std::setfill('0') << year << "-"
        << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day << ".txt";

    std::string fullPath = path.str(); // 转换为字符串形式

    // 1. 读取现有预约文件内容
    std::string fileContent = readFile(fullPath.c_str(), Dname);
    if (fileContent.empty()) {
        // 如果文件不存在，初始化空文件，直接写入
        return writeFile(fullPath.c_str(), Dname, std::string(Pname) + "\n");
    }

    // 2. 计算当前文件中的预约人数（每行一个人）并检查是否已有相同的患者
    int appointmentCount = 0;
    std::stringstream ss(fileContent);
    std::string line;
    bool alreadyBooked = false;  // 标记是否已预约
    while (std::getline(ss, line)) {
        appointmentCount++;
        if (line == Pname) {  // 如果已经有相同名字的患者，设置标记为 true
            alreadyBooked = true;
            break;
        }
    }

    // 3. 如果患者已经预约，提示已经预约成功
    if (alreadyBooked) {
        std::cout << "Patient " << Pname << " has already made an appointment with Dr. " << Dname << " on "
            << year << "-" << std::setw(2) << std::setfill('0') << month << "-"
            << std::setw(2) << std::setfill('0') << day << "." << std::endl;
        return false;  // 返回 false，表示已经预约
    }

    // 4. 如果预约人数超过8人，则拒绝写入
    if (appointmentCount >= 8) {
        std::cout << "Error: Maximum appointment limit reached (8 people)." << std::endl;
        return false;
    }

    // 5. 否则，继续写入新的预约
    std::string newAppointment = std::string(Pname) + "\n"; // 在患者名称后加上换行符
    return writeAppendFile(fullPath.c_str(), Dname, newAppointment); // 使用追加模式写入
}



bool fileSystem::deleteAppointments(const char* Pname, const char* Dname, std::string year, std::string month, std::string day) {
    
    /*
        读取相应文件，如未成功，返回0，此预约文件不存在；
        如果成功读取，就每一行遍历，查看是否有Pname，如果没有就说明该病人未预约；
        如果有Pname，就将文件中那一行以外的内容记录下来，然后重新写入；
    */
    
    // 1. 构建文件路径
    std::ostringstream path;
    path << "/appointments/" << Dname << "-"
        << std::setw(4) << std::setfill('0') << year << "-"
        << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day << ".txt";

    std::string fullPath = path.str(); // 转换为字符串形式

    // 2. 读取预约文件内容
    std::string fileContent = readFile(fullPath.c_str(), Dname);
    if (fileContent.empty()) {
        //std::cerr << "Appointment file does not exist or is empty." << std::endl;
        return false;  // 文件不存在或为空
    }

    // 3. 逐行遍历文件内容，查找病人是否已预约
    std::stringstream ss(fileContent);
    std::string line;
    std::stringstream updatedContent;
    bool appointmentFound = false;

    while (std::getline(ss, line)) {
        if (line == Pname) {
            // 找到病人的预约，跳过此行，表示删除该预约
            appointmentFound = true;
            continue;  // 不将该行写入更新后的内容
        }
        updatedContent << line << "\n";  // 保留其他行
    }

    if (!appointmentFound) {
        std::cout << "Error: Patient " << Pname << " has not made an appointment." << std::endl;
        return false;  // 没有找到该病人的预约
    }

    // 4. 更新文件内容，删除指定病人的预约
    return writeFile(fullPath.c_str(), Dname, updatedContent.str());
}

std::string fileSystem::listAppointments() {
    std::string path = "/appointments/";
    return displayDirectory(path.c_str());
}

std::string fileSystem::readAppointments(const char* Dname, std::string year, std::string month, std::string day) {
    
    std::ostringstream path;
    path << "/appointments/" << Dname << "-"
        << std::setw(4) << std::setfill('0') << year << "-"
        << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day << ".txt";

    std::string fullPath = path.str(); // 转换为字符串形式

    return readFile(fullPath.c_str(), Dname);
}

std::string fileSystem::getCurrentDateTime() {
    // 获取当前时间
    std::time_t t = std::time(0);  // 获取当前时间戳
    std::tm now;
    localtime_r(&t,&now);  // 使用 localtime_s 替代 localtime

    // 使用 stringstream 构造格式化日期字符串
    std::stringstream ss;
    ss << (now.tm_year + 1900) << "-"    // 年
        << (now.tm_mon + 1) << "-"         // 月（注意：tm_mon 是从0开始的）
        << now.tm_mday << "_"              // 日
        << now.tm_hour << ":"              // 时
        << now.tm_min << ":"               // 分
        << now.tm_sec;                     // 秒

    return ss.str();  // 返回格式化后的字符串
}

bool fileSystem::enableFileSnapshot(const std::string& filePath, const std::string& operatorName) {
    // 1. 查找文件的 inode
    Inode* fileInode = findInodeByPath(filePath.c_str());
    if (!fileInode) {
        std::cout << "Error: File not found!" << std::endl;
        return false;  // 文件不存在
    }

    // 2. 验证操作者权限（管理员或文件的拥有者）
    if (!isAdmin(operatorName) && fileInode->creator != operatorName) {
        std::cout << "Error: Only administrators or the file owner can enable File Snapshot!" << std::endl;
        return false;
    }

    std::string time = getCurrentDateTime();

    if (fileInode->FS == 1) {
        return 1;
    }
    //设置为开启,并作为第一次文件快照（全量存储）
    fileInode->FS = 1;
    strncpy(fileInode->FStime[0], time.c_str(), sizeof(fileInode->FStime[0]) - 1);
    fileInode->FStime[0][sizeof(fileInode->FStime[0]) - 1] = '\0'; // 确保以 '\0' 结尾
    fileInode->FSoperation[0] = 'W';


    //在FS下创建一个，该文件名的文件夹

    //获取文件名
    std::vector<std::string> pathParts = splitPath(filePath.c_str());    
    std::string targetFileName = pathParts.back();
    std::string targetPath = "/FS/" + targetFileName;

    //创建文件夹
    if (!createDirectory(targetPath.c_str(), "admin")) {
        std::cout << "This file has opened a file snapshot" << std::endl;
        return false;
    }

    //创建文件
    std::string targetFilePath = "/FS/" + targetFileName +"/"+ time+ ".txt";
    createFile(targetFilePath.c_str(), operatorName.c_str());

    //写入全量内容
    std::string context = readFile(filePath.c_str(), operatorName.c_str());
    writeFile(targetFilePath.c_str(), operatorName.c_str(), context);
    
    return true;
}

std::string fileSystem::listFileSnapshot(const std::string& filePath) {
    //获取快照文件夹
    std::vector<std::string> pathParts = splitPath(filePath.c_str());
    std::string targetFileName = pathParts.back();
    std::string targetPath = "/FS/" + targetFileName;

    return displayDirectory(targetPath.c_str());
}

bool fileSystem::useFileSnapshots(const std::string& filePath, const std::string& time, const std::string& operatorName) {
    
    //将文件快照中的内容拷贝到源文件夹    
    
    // 1. 查找文件的 inode
    Inode* fileInode = findInodeByPath(filePath.c_str());
    if (!fileInode) {
        std::cout << "Error: File not found!" << std::endl;
        return false;  // 文件不存在
    }

    // 2. 验证操作者权限（管理员或文件的拥有者）
    if (!isAdmin(operatorName) && fileInode->creator != operatorName) {
        std::cout << "Error: Only administrators or the file owner can enable File Snapshot!" << std::endl;
        return false;
    }

    std::vector<std::string> pathParts = splitPath(filePath.c_str());
    std::string targetFileName = pathParts.back();

    //增量过程
    
    // 找到time对应的是第几次操作
    
    int op = 0;
    for (int i = 0; i < MAX_FS ; i++) {
        if (strcmp(fileInode->FStime[i], time.c_str()) == 0) {
            op = i;
            break;
        }
    }
    // 找到最新一次写操作
    int newW = 0;
    for (int j = op; j >= 0; j--) {
        if (fileInode->FSoperation[j] == 'W') {
            newW = j;
            break;
        }
    }
    //从最新一次写操作依次读取后面的全部内容

    std::string context;
    std::string time_;
    for (int z = newW; z <= op; z++) {
        time_ = fileInode->FStime[z];
        std::string targetFilePath = "/FS/" + targetFileName + "/" + time_ + ".txt";
        context = context + readFile(targetFilePath.c_str(), operatorName.c_str());
    }

    // 写入源文件       

    return writeFile(filePath.c_str(), operatorName.c_str(), context, 0);
    
}

void fileSystem::lockFS() {
    FS.lock();    
}

void fileSystem::unlockFS() {
    FS.unlock();   
}

void fileSystem::lockRC() {
    RC.lock();
}

void fileSystem::unlockRC() {
    RC.unlock();
}