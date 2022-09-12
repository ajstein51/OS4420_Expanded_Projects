/**
 * AJ Stein
 * CS4420
 * Find 
 * Implemented extra credit of -ls
 */
// includes may or may not need all of them
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <cstring>
#include <deque>
#include <math.h>
using namespace std;

// global var
string startdir, name, size;
// type will be the options for -type, it defaults to P which means nothing just something for me to check for
char type = 'P';
// debug is for the debug option and flag is for the size functions of checking the argv[1] folder if we should print that our or not
bool debug = false, flag = false;

// function
// for extra credit: ls
vector<char> mode(mode_t mode);
void printls(string curdir);

// no args
void printdirnoarg(string curdir);

// for single args in the command line so only a type, name, size
void printcurdirs(string curdir, char type); 
void printname(string curdir, string name);
void printsize(string curdir, string filesize);

// double args, will have a combo of 2 of the following: type, name, size
void printnametype(string curdir, char type, string name);
void printsizetype(string curdir, char type, string filesize);
void printnamesize(string curdir, string name, string filesize);

// triple arg, has all 3 type name and size
void printall(string curdir, char type, string name, string filesize);

int main(int argc, char** argv){
    // // check if we got any filters
    if(argc == 2){
        // get the starting directory
        startdir = argv[1];
        // print the first folder
        cout << startdir << endl;
        printdirnoarg(startdir);
    }
    else{
        for(int i = 0; i < argc; i++){
            string tmp = argv[i];
            // see what we got in terms of args: debug, type, name, size, or ls
            if(tmp.compare("-type") == 0)
                type = *argv[i + 1];
            else if(tmp.compare("-name") == 0)
                name = argv[i + 1];
            else if(tmp.compare("-size") == 0)
                size = argv[i + 1];
            else if(tmp.compare("-d") == 0){
                debug = true;
                cout << "Debug is on\n";
            }
            else if(tmp.compare("-ls") == 0){
                startdir = argv[1];
                printls(startdir);
            }
        }
    }

    // debug
    if(debug)
        cout << "size " << size.empty() << size << " name " << name.empty() << " " << name << endl;

    // single args
    if(type != 'P' && size.empty() && name.empty()){
        startdir = argv[1];
        if(type == 'd')
            cout << startdir << endl;
        printcurdirs(startdir, type);
    }
    else if(type == 'P' && size.empty() && !name.empty()){
        startdir = argv[1];
        printname(startdir, name);
    }
    else if(type == 'P' && !size.empty() && name.empty()){
        startdir = argv[1];
        printsize(startdir, size);
    }

    // double args
    if(type != 'P' && !name.empty() && size.empty()){
        startdir = argv[1];
        printnametype(startdir, type, name);
    }
    else if(type != 'P' && name.empty() && !size.empty()){
        startdir = argv[1];
        printsizetype(startdir, type, size);
    }
    else if(type == 'P' && !name.empty() && !size.empty()){
        startdir = argv[1];
        printnamesize(startdir, name, size);
    }

    // triple args
    if(type != 'P' && !name.empty() && !size.empty()){
        startdir = argv[1];
        printall(startdir, type, name, size);
    }
    return 0;
} // end of main

// EC: ls command option, should be working minus some changes in the dating portion, most of this gotten from sstat example
void printls(string curdir){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        if(flag == false){
            // info on file
            if(lstat(curdir.c_str(), &filestat) == -1)
                perror(curdir.c_str());

            // output:
            // Phsys-inode blocks mode link ownder group size changed path
            cout << filestat.st_ino << "   " << filestat.st_blocks << "   ";
            
            // go through vector and print it out
            vector<char> tmp = mode(filestat.st_mode);
            // print mode
            for(auto& i : tmp)
                cout << i;
            
            // print ownder
            struct passwd *ppass;
            string user = "", group = "";
            if((ppass = getpwuid(filestat.st_uid)) == NULL)
                user = "If broke";
            else
                user = ppass -> pw_name;
            struct group *pgroup;
            if((pgroup = getgrgid(filestat.st_gid)) == NULL)
                group = "If broke";
            else 
                group = pgroup -> gr_name;

            cout << "   " << filestat.st_nlink << "   "  << user << "   " << group << "   " << filestat.st_size << "   " << ctime(&filestat.st_mtime) << "   " << curdir << endl;
        
            
            flag = true;
        }
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());
        
        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output:
                // Phsys-inode blocks mode link ownder group size changed path

                cout << filestat.st_ino << "   " << filestat.st_blocks << "   ";
                // go through vector and print it out
                vector<char> tmp = mode(filestat.st_mode);
                // print mode
                for(auto& i : tmp)
                    cout << i;
                
                // print ownder
                struct passwd *ppass;
                string user = "", group = "";
                if((ppass = getpwuid(filestat.st_uid)) == NULL)
                    user = "If broke";
                else
                    user = ppass -> pw_name;
                struct group *pgroup;
                if((pgroup = getgrgid(filestat.st_gid)) == NULL)
                    group = "If broke";
                else 
                    group = pgroup -> gr_name;

                cout << "   " << filestat.st_nlink << "   "  << user << "   " << group << "   " << filestat.st_size << "   " << ctime(&filestat.st_mtime) << "   " << builtdir << endl;
            }
        }

    } // end of while
    if (closedir(folder) == -1)
        perror("closedir");
}

// gotten from example file sstats
vector<char> mode(mode_t mode){
    vector<char> ret;
    for(int i = 0; i < 10; i++)
        ret.push_back('-');

    if(mode & (S_IFDIR))
        ret[0] = 'd';
    if(mode & (S_IFREG))
        ret[0] = 'r';
    if(mode & (S_IREAD)) 
        ret[1] = 'r';
    if(mode & (S_IWRITE))
        ret[2] = 'w';
    if(mode & (S_IEXEC))
        ret[3] = 'x';
    if(mode & (S_ISUID))
        ret[3] = 's';

    if(mode & (S_IREAD>>3))
        ret[4] = 'r';
    if(mode & (S_IWRITE>>3))
        ret[5] = 'w';
    if(mode & (S_IEXEC>>3))
        ret[6] = 'x';
    if(mode & (S_ISGID))
        ret[6] = 's';
    
    if(mode & (S_IREAD>>6))
        ret[7] = 'r';
    if(mode & (S_IWRITE>>6))
        ret[8] = 'w';
    if(mode & (S_IEXEC>>6))
        ret[9] = 'x';

    return ret;
}

// triple args, print out the folder with every arg option: type, size, name
void printall(string curdir, char type, string name, string filesize){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // get size number
    int size, compareflag = -1;

    // To sum up these 3 or so if and if else statements: I make the given size into an into from string and get rid of the -/+ and c that maybe attached    
    // if we got a block size type and + or -
    if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 1; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // end of if, st_size
    else if((filesize[0] != '+' || filesize[0] != '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 0; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // st_size for checking but itll be ==
    else if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() != 'c'){
        compareflag = 0;
        string tmp;
        for(size_t i = 1; i < filesize.size(); i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if(filesize[0] != '+' && filesize[0] != '-' && filesize.back() != 'c'){
        compareflag = 0;
        size = stoi(filesize);
    } // no c, and no + or -, go on st block

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        // check the first given folder
        if(flag == false){
            // check the given file if its a good size
            // info on file
            if(lstat(curdir.c_str(), &filestat) == -1)
                perror(curdir.c_str());
            if(debug)
                cout << "curdir " << curdir << endl;
            
            // output 
            if(curdir == name){
                string tmp2;
                tmp2.push_back(type);
                if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << curdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << curdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << curdir << endl; 
                        }
                    } // end of else and outputting
                }
                if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << curdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << curdir << endl;  
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << curdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << curdir << endl; 
                        }
                    } // end of else and outputting
                }  
                if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << curdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << curdir << endl; 
                        }
                    }
                    else{
                    if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << curdir << endl; 
                        }
                    } // end of else and outputting
                }   
                if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << curdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << curdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << curdir << endl; 
                        }
                    } // end of else and outputting
                }              
                if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << curdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << curdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << curdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << curdir << endl; 
                        }
                    } // end of else and outputting
                }
            } // end of output
            
            // set flag to true so we dont get in this block again
            flag = true;
        }
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        // output
        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output 
                if(filename == name){
                    string tmp2;
                    tmp2.push_back(type);
                    if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0){
                        if(filesize[0] == '+'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) > size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size > size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else if(filesize[0] == '-'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) < size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size < size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else{
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) == size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size == size)
                                    cout << builtdir << endl; 
                            }
                        } // end of else and outputting
                    }
                    if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0){
                        if(filesize[0] == '+'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) > size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size > size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else if(filesize[0] == '-'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) < size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size < size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else{
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) == size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size == size)
                                    cout << builtdir << endl; 
                            }
                        } // end of else and outputting
                    }  
                    if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0){
                        if(filesize[0] == '+'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) > size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size > size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else if(filesize[0] == '-'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) < size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size < size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else{
                        if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) == size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size == size)
                                    cout << builtdir << endl; 
                            }
                        } // end of else and outputting
                    }   
                    if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0){
                        if(filesize[0] == '+'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) > size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size > size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else if(filesize[0] == '-'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) < size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size < size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else{
                        if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) == size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size == size)
                                    cout << builtdir << endl; 
                            }
                        } // end of else and outputting
                    }              
                    if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0){
                        if(filesize[0] == '+'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) > size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size > size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else if(filesize[0] == '-'){
                            if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) < size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size < size)
                                    cout << builtdir << endl; 
                            }
                        }
                        else{
                        if(compareflag == 0){
                                // no c
                                if(ceil(filestat.st_size / 512) == size)
                                    cout << builtdir << endl; 
                            }
                            else if(compareflag == 1){
                                // has a c
                                if(filestat.st_size == size)
                                    cout << builtdir << endl; 
                            }
                        } // end of else and outputting
                    }
                } // end of output

                // recurse on new folder
                if(S_ISDIR(filestat.st_mode) != 0)
                    printall(builtdir, type, name, filesize);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// double args, size and type
void printsizetype(string curdir, char type, string filesize){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // get size number
    int size = -1, compareflag = -1;
    
    // To sum up these 3 or so if and if else statements: I make the given size into an into from string and get rid of the -/+ and c that maybe attached    
    if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 1; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // end of if, st_size
    else if((filesize[0] != '+' || filesize[0] != '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 0; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() != 'c'){
        compareflag = 0;
        string tmp;
        for(size_t i = 1; i < filesize.size(); i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if(filesize[0] != '+' && filesize[0] != '-' && filesize.back() != 'c'){
        compareflag = 0;
        size = stoi(filesize);
    } // no c, and no + or -, go on st block

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        if(flag == false){
            // check the given file if its a good size
            // info on file
            if(lstat(curdir.c_str(), &filestat) == -1)
                perror(curdir.c_str());
            if(debug)
                cout << "curdir " << curdir << endl;
            // output
            string tmp2;
            tmp2.push_back(type);
            if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } // end of else and outputting
            }
            else if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } // end of else and outputting
            }  
            else if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } // end of else and outputting
            }   
            else if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } // end of else and outputting
            }              
            else if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } // end of else and outputting
            } // end of if
            flag = true;
        }
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output 
                string tmp2;
                tmp2.push_back(type);
                if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } // end of else and outputting
                }
                else if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } // end of else and outputting
                }  
                else if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } // end of else and outputting
                }   
                else if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                    if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } // end of else and outputting
                }              
                else if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } // end of else and outputting
                }

                if(S_ISDIR(filestat.st_mode) != 0)
                    printsizetype(builtdir, type, filesize);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// double args, size and name
void printnamesize(string curdir, string name, string filesize){
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;
   
    // get size number
    int size, compareflag = -1;
    
    if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 1; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // end of if, st_size
    else if((filesize[0] != '+' || filesize[0] != '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 0; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() != 'c'){
        compareflag = 0;
        string tmp;
        for(size_t i = 1; i < filesize.size(); i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if(filesize[0] != '+' && filesize[0] != '-' && filesize.back() != 'c'){
        compareflag = 0;
        size = stoi(filesize);
    } // no c, and no + or -, go on st block
    // debug
    if(debug)
        cout << "\ncompflag  " << compareflag << " " << filesize<<endl;
    
    // // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror ssize\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        if(flag == false){
            // check the given file if its a good size
            // info on file
            if(lstat(curdir.c_str(), &filestat) == -1)
                perror(curdir.c_str());
            if(debug)
                cout << "curdir " << curdir << endl;
            // output
            if(curdir == name){
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << curdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << curdir << endl; 
                    }
                }
                else{
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << curdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << curdir << endl; 
                    }
                } 
            }
            flag = true;
        }
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output
                if(filename == name){
                    if(filesize[0] == '+'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) > size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size > size)
                                cout << builtdir << endl; 
                        }
                    }
                    else if(filesize[0] == '-'){
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) < size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size < size)
                                cout << builtdir << endl; 
                        }
                    }
                    else{
                        if(compareflag == 0){
                            // no c
                            if(ceil(filestat.st_size / 512) == size)
                                cout << builtdir << endl; 
                        }
                        else if(compareflag == 1){
                            // has a c
                            if(filestat.st_size == size)
                                cout << builtdir << endl; 
                        }
                    } 
                }
                if(S_ISDIR(filestat.st_mode) != 0)
                    printnamesize(builtdir, name, filesize);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// double args, name and type
void printnametype(string curdir, char type, string name){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }

    while((entry = readdir(folder)) != NULL){
        // count the first 2 and skip those cause its . and ..
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output 
                string tmp2;
                tmp2.push_back(type);
                if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0 && name == filename)
                    cout << builtdir << endl;
                else if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0 && name == filename)
                    cout << builtdir << endl;
                else if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0 && name == filename)
                    cout << builtdir << endl;
                else if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0 && name == filename)
                    cout << builtdir << endl;
                else if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0 && name == filename)
                cout << builtdir << endl;

                if(S_ISDIR(filestat.st_mode) != 0)
                    printnametype(builtdir, type, name);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// single args, size
void printsize(string curdir, string filesize){
    // st_size / 512 for ==, then round it up or down and compare it 
    // + is greater
    // - less
    // none ==
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;
   
    // get size number
    int size = -1, compareflag = -1;
    
    if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 1; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // end of if, st_size
    else if((filesize[0] != '+' || filesize[0] != '-') && filesize.back() == 'c'){
        compareflag = 1;
        string tmp;
        for(size_t i = 0; i < filesize.size() - 1; i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if((filesize[0] == '+' || filesize[0] == '-') && filesize.back() != 'c'){
        compareflag = 0;
        string tmp;
        for(size_t i = 1; i < filesize.size(); i++)
            tmp.push_back(filesize[i]);
        
        size = stoi(tmp);
        // debug
        if(debug)
            cout << tmp << " " << size;
    } // no c, st_blocks
    else if(filesize[0] != '+' && filesize[0] != '-' && filesize.back() != 'c'){
        compareflag = 0;
        size = stoi(filesize);
    } // no c, and no + or -, go on st block

    // debug
    if(debug)
        cout << "\ncompflag  " << compareflag << " " << filesize<<endl;
    // cout << "\nThis is size: " << size << endl;
    
    // // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror ssize\n";
        perror(curdir.c_str());
        exit(-1);
    }

    while((entry = readdir(folder)) != NULL){
        if(flag == false){
            // check the given file if its a good size
            // info on file
            if(lstat(curdir.c_str(), &filestat) == -1)
                perror(curdir.c_str());
            if(debug)
                cout << "curdir " << curdir << endl;
            // output
            if(filesize[0] == '+'){
                if(compareflag == 0){
                    // no c
                    if(ceil(filestat.st_size / 512) > size)
                        cout << curdir << endl; 
                }
                else if(compareflag == 1){
                    // has a c
                    if(filestat.st_size > size)
                        cout << curdir << endl; 
                }
            }
            else if(filesize[0] == '-'){
                if(compareflag == 0){
                    // no c
                    if(ceil(filestat.st_size / 512) < size)
                        cout << curdir << endl; 
                }
                else if(compareflag == 1){
                    // has a c
                    if(filestat.st_size < size)
                        cout << curdir << endl; 
                }
            }
            else if(filesize[0] != '-' && filesize[0] != '+'){
               if(compareflag == 0){
                    // no c
                    if(ceil(filestat.st_size / 512) == size)
                        cout << curdir << endl;  
                }
                else if(compareflag == 1){
                    // has a c
                    if(filestat.st_size == size)
                        cout << curdir << endl;  
                }
            } // end of else and outputting
            flag = true;
        }
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output
                if(filesize[0] == '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) > size)
                            cout << builtdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size > size)
                            cout << builtdir << endl; 
                    }
                }
                else if(filesize[0] == '-'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) < size)
                            cout << builtdir << endl; 
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size < size)
                            cout << builtdir << endl; 
                    }
                }
                else if(filesize[0] != '-' && filesize[0] != '+'){
                    if(compareflag == 0){
                        // no c
                        if(ceil(filestat.st_size / 512) == size)
                            cout << builtdir << endl;  
                    }
                    else if(compareflag == 1){
                        // has a c
                        if(filestat.st_size == size)
                            cout << builtdir << endl; 
                    }
                } // end of else and outputting

                if(S_ISDIR(filestat.st_mode) != 0)
                    printsize(builtdir, filesize);
            }
        }

    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// single args, name
void printname(string curdir, string name){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        // if its not .. or .
        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                if(filename == name)
                    cout << builtdir << endl; 

                if(S_ISDIR(filestat.st_mode) != 0)
                    printname(builtdir, name);
            }
        }
        
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// single args, type
void printcurdirs(string curdir, char type){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        // count the first 2 and skip those cause its . and ..
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

            // output 
        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                string tmp2;
                tmp2.push_back(type);
                if(tmp2.compare("d") == 0 && S_ISDIR(filestat.st_mode) != 0){
                    cout << builtdir << endl;
                }
                else if(tmp2.compare("f") == 0 && S_ISREG(filestat.st_mode) != 0)
                    cout << builtdir << endl;
                else if(tmp2.compare("l") == 0 && S_ISLNK(filestat.st_mode) != 0)
                    cout << builtdir << endl;
                else if(tmp2.compare("b") == 0 && S_ISBLK(filestat.st_mode) != 0)
                    cout << builtdir << endl;
                else if(tmp2.compare("c") == 0 && S_ISCHR(filestat.st_mode) != 0)
                cout << builtdir << endl;

                if(S_ISDIR(filestat.st_mode) != 0)
                    printcurdirs(builtdir, type);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

// no arg
void printdirnoarg(string curdir){
    // init
    DIR *folder;
    struct dirent *entry;
    struct stat filestat;

    // open the dir we are to print
    if ((folder = opendir(curdir.c_str())) == NULL) {
        cout << "\nPerror printcurdir\n";
        perror(curdir.c_str());
        exit(-1);
    }
    while((entry = readdir(folder)) != NULL){
        // built dir
        string builtdir = curdir;
        builtdir.append("/");

        // get just this current file
        string filename = entry -> d_name;

        // append on filename
        builtdir.append(filename);

        // info on file
        if(lstat(builtdir.c_str(), &filestat) == -1)
            perror(builtdir.c_str());

        // if its not .. or .
        if(filename.compare("..") != 0){
            if(filename.size() == 1 && filename[0] == '.'){
                cout << "";
            }
            else{
                // output 
                cout << builtdir << endl;

                // recurse if file
                if(S_ISDIR(filestat.st_mode) != 0)
                    printdirnoarg(builtdir);
            }
        }
    } // end of while

    if (closedir(folder) == -1)
        perror("closedir");
}

/**
 * To explain the huge pile of code, each function is essentially the same
 * and build ontop of a 'formula' per say. Each one of these functions has
 * simular aspects in that they declare initial structs of folder, entry and filestat
 * then we open the starting folder that we were given in the argv. Following that we 
 * loop through the folder we are at. Checking in the first if statement the starting folder
 * Skipping the first 2 files in the folder cause we dont want to deal with . and .. following that
 * we go through and check the user options that we may or may not have gotten (name, size, type)
 * which we output according to. Each time we build a new string that is the 'absolutepath' of the 
 * file we are currently at. If that file is a directory we call the functin we are at again with that
 * folder to go through and do what we are currently doing with it (recurse on it and print out whatever)
 */