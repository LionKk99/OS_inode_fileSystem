# OS_inode_fileSystem
SCUT OS_final_HW

## Background/ Target 
The goal of this project is to design a medical information 
management system with client-server separation based on the I-Node 
file system. 

## Functions Implemented 
The development of this project mainly includes three parts: I-Node 
file system development, medical information management system 
interface development, and client-server development based on 
SOCKCT. 

### The functions implemented in the I-Node file system include: 

• Creating/deleting/displaying directories 
• Creating/deleting/reading/writing/appending files 

• Persistent storage of file systems 

• Storage/use/list of available backup files 

• Open/use/list of available snapshots of file snapshots 

• Changing file permissions 

• Changing file owners 

• Changing file user groups 

### The medical information management system interface includes: 

• User login 

• Creating/reading/writing/deleting/displaying patient records 
 
• Doctors publishing/cancelling/reading/displaying appointments 

• Patients making/deleting/viewing available appointments 

• Administrators create users/delete users/change user passwords 

### SOCKCT's client-server functions include: 

• Using sockets to receive/send information on the client and server 

• Calling the medical information management system interface 

• Calling the file system interface 

• Create threads to meet the needs of multiple clients accessing at the same time 

• Using semaphores to complete mutual exclusion/synchronization operations to ensure the correctness of the file system (reader-writer problem) 

### Additional technical details： 
• Using indirect pointers to manage I-Node Limitations 
