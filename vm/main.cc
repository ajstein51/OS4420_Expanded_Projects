/**
 * AJ Stein
 * CS4420
 * Virtual Memory
 */
// includes
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <string>
#include <queue>
#include <string>
#include <list>
#include <tuple>
#include <deque>
using namespace std;

// structs
struct pages{
    // pages values
    int disk, loc; 
    string type, framenum;
};
struct frames{
    int inuse, dirty, first_use, last_use, obj, least;
};

// functions
void setup();
void fifo();
void lru();
void randswap();
void lfu();
void printout();
void extraprintout(int mapped);

// vars
int frameamount = 0, pageamount = 0, taken = 0, recovered = 0, missed = 0, referenced = 0;
bool debug = false;
vector<string> input;
vector<pages> page;
vector<frames> frame;


int main(int argc, char** argv){
    // declarations
    ifstream ins;               // get input from file
    string algorith, file;          
    if(argc > 2){              // Load vars
        algorith = argv[1];         // get LRU or FIFO
        file = argv[2];         // get the file
    } // end of if
    else{
        cout << "Not enough arguments given" << endl;
        exit(-1);
    }
    ins.open(file);             // try to open the file
    if(!ins){                   // error check the file
        cout << "\nError in opening file\n";
        exit(-1);
    }
    // load the vecot of pair with the contents in the file
    int flag = 0;
    while(!ins.eof()){
        string tmp;
        getline(ins, tmp);
        // cout << "\nthis is tmp " << tmp[0];
        // parse the line
        if(std::isdigit(tmp[0]) && frameamount == 0)
            frameamount = stoi(tmp), flag++;
        else if(std::isdigit(tmp[0]) && flag == 1)
            pageamount = stoi(tmp), flag++;
        else
            if(tmp[0] != '#')   // check if its a comment 
                input.push_back(tmp);
    } // end of while/inputting
    
    // check the input
    // cout << "\nframe " << frameamount << endl << "page " << pageamount << endl;
    // for(auto& i : input)
    //     cout << i << endl;
    
    // setup the vector of structs
    setup();

    // run the algo
    if(algorith.compare("fifo") == 0 || algorith.compare("FIFO") == 0)
        fifo();
    else if(algorith.compare("lru") == 0 || algorith.compare("LRU") == 0)
        lru();
    else if (algorith.compare("random") == 0 || algorith.compare("RANDOM") == 0)
        randswap();
    else
        lfu();

    // algo should be done and we just print out what we got
    printout();

    return 0;
} // end of main

// function def
//********************************************************************************  
// set up a vector of structs with pages amount of structs, and default values
void setup(){
    // load a vector of structs
    for(int i = 0; i < pageamount; i++){
        pages alg;
        alg.loc = i;
        alg.disk = 0;
        alg.type = "Unmapped";
        alg.framenum = "(unassigned)";
        page.push_back(alg);
    }

    for(int i = 0; i < frameamount; i++){
        frames f;
        f.inuse = 0;
        f.dirty = 0;
        f.first_use = 0;
        f.last_use = 0;
        f.obj = -1;
        frame.push_back(f);
    }
    // leave the function
    return;
}
//********************************************************************************
void lru(){
    // for checking recovered
    vector<int> rec;
    rec.resize(pageamount);
    
    int counter = 0;
    // counter is the pages index
    for (size_t i = 0; i < input.size(); i++)
    {
        // check if we got a printout
        if (input[i][0] == 'p')
        {
            cout << "********************************************************************************\n";
            printout();
            cout << "********************************************************************************\n";
            counter -= 1;
        }
        else if (input[i][0] == 'd')
        {
            debug = true;
            cout << "\nDebug on\n";
            counter -= 1;
        }
        else if (input[i][0] == 'n')
        {
            debug = false;
            cout << "\nDebug off\n";
            counter--;
        }
        else if(!input[i].empty())
        {
            // check the size and get the size
            int index = -1, flag = 0;
            if (input[i].size() == 3)
                index = input[i][2] - '0'; // get the page location
            else if(input[i].size() > 3)
            {
                // its a double digit
                string tmp;
                for (size_t j = 2; j < input[i].size(); j++)
                {
                    tmp.push_back(input[i][j]);
                }
                index = stoi(tmp);
                if (debug == true)
                {
                    cout << "\ntmp " << tmp;
                    cout << "\nindex " << index;
                }
            } // end of else
            // first x amount
            if (counter < frameamount)
            {
                page[index].type = "Mapped";
                page[index].framenum = to_string(counter);
                frame[counter].obj = index; // mark the page num
                frame[counter].first_use = counter + 1, frame[counter].last_use = counter + 1;
                frame[counter].inuse = 1; // mark the frame in use
                // stk.push(make_pair(counter, counter));

                // counter
                missed++;
                // recovered
                if (input[i][0] == 'w')
                    rec[counter] = 1, frame[counter].dirty = 1;
            }
            // check if its already in the frame
            // if its in the frame up the last use by 1
            for (int j = 0; j < frameamount; j++)
            {
                if (frame[j].obj == index) // index is page, obj stores the frames page its on
                {
                    // counters
                    frame[j].last_use = counter + 1;
                    // its a duplicate so we mark it as recovered
                    page[index].disk = 0;

                    // set the flag
                    flag = 1;
                }
            }
            if(counter >= frameamount){
                // check if its in the frame already
                if(flag == 0){
                    // its not in the frame
                    // search for the lowest obj cause thatll be the lru
                    int min = frame[0].last_use, lru_idx = 0, last_page = -1;
                    for(int j = 1; j < frameamount; j++){
                        if(frame[j].last_use < min){
                            lru_idx = j; // get the frame
                            min = frame[j].last_use;  // update lowest last use
                            last_page = frame[j].obj; // get the last page
                        }
                    } // end of for j
                    if(last_page == -1)
                        last_page = stoi(page[0].framenum);
                    // update the old page
                    page[last_page].type = "Taken";
                    page[last_page].framenum = "(unassigned)";

                    // update new page
                    page[index].type = "Mapped";
                    page[index].framenum = to_string(lru_idx);

                    // update the frame storage
                    frame[lru_idx].obj = index;
                    frame[lru_idx].first_use = counter + 1, frame[lru_idx].last_use = counter + 1;
                
                    missed++;
                    taken++;
                    // frames recovered from disk, if we wrote to the page and then later rewrote/read to the page
                    // the recover goes up
                    if (rec[index] == 1)
                        recovered++;
                    if (input[i][0] == 'w' && rec[index] == 0)
                        rec[index] = 1;
                
                }  // end of if 
            }
        }
        counter++;
    } // end of for
    // print out stack
    if (debug == true)
    {
        cout << "\nframe\n";
        for(int i = 0; i < frameamount; i++)
            cout << "Frame: " << i << " Page " << frame[i].obj << endl;
    }
    return;
} // end of function
//********************************************************************************  
// go through input and get the type going from unmapped, mapped, and taken
void fifo(){
    // first = location, second = frame
    queue<pair<int, int>> stk;

    // for checking recovered
    vector<int> rec;
    rec.resize(pageamount);

    int counter = 0; // quick fix for replacing i
    // i is the pages index
    for(size_t i = 0; i < input.size(); i++){
        // check if we got a printout
        if (input[i][0] == 'p')
        {
            cout << "********************************************************************************\n";
            printout();
            cout << "********************************************************************************\n";
            counter -= 1;
        }
        else if (input[i][0] == 'd')
        {
            debug = true;
            cout << "\nDebug on\n";
            counter -= 1;
        }
        else if (input[i][0] == 'n')
        {
            debug = false;
            cout << "\nDebug off\n";
            counter--;
        }
        else if(!input[i].empty())
        {
            // check the size and get the size
            int index = 0, inflag = 0;
            if (input[i].size() == 3){
                index = input[i][2] - '0'; // get the page location
            }
            else if(input[i].size() > 3){
                // its a double digit
                string tmp;
                for(size_t j = 2; j < input[i].size(); j++){
                    tmp.push_back(input[i][j]);
                }
                index = stoi(tmp);
                if(debug == true){
                    cout << "\ntmp " << tmp;
                    cout << "\nindex " << index;
                }
            } // end of else
            // check if its already in the frame
            for(int j = 0; j < frameamount; j++){
                if(frame[j].obj == index){
                    // flag for if we put it in or not
                    inflag = 1;
                    // counters
                    frame[j].last_use = counter + 1;
                    // its a duplicate so we mark it as recovered
                    page[index].disk = 0;
                }
            }
            // first x amount
            if((signed)counter < frameamount){
                page[index].type = "Mapped";
                page[index].framenum = to_string(counter);
                frame[counter].obj = index; // mark the page num
                frame[counter].first_use = counter, frame[counter].last_use = counter;
                frame[counter].inuse = 1; // mark the frame inuse
                stk.push(make_pair(counter, counter));

                // counter
                missed++;
                // recovered
                if(input[i][0] == 'w')
                    rec[counter] = 1, frame[counter].dirty = 1;
            }
            if(inflag == 0 && (signed)counter >= frameamount){
                if(debug)
                    cout << "\nThis is index " << index << endl;
                // its not already in the frame
                // get the pair
                pair<int, int> p;
                p = stk.front();
                stk.pop();

                // update old page values
                page[p.first].type = "Taken";
                page[p.first].framenum = "(unassigned)";

                // update new page values
                page[index].type = "Mapped";
                page[index].framenum = to_string(p.second); // frame
                // for ondisk, we mark it as 1 if we steal the frame to write because the frame already has a page
                if(frame[p.second].obj != -1)
                    page[index].disk = 1;
                
                // update frame obj
                frame[p.second].obj = index;

                // push the new values to the stack
                stk.push(make_pair(index, p.second));

                // counters
                frame[p.second].first_use = counter + 1, frame[p.second].last_use = counter + 1;
                missed++;
                taken++;
                // frames recovered from disk, if we wrote to the page and then later rewrote/read to the page
                // the recover goes up
                if(rec[index] == 1)
                    recovered++;
                if(input[i][0] == 'w' && rec[index] == 0)
                    rec[index] = 1;
                if(input[i][0] == 'w')
                    frame[p.second].dirty = 1;
                else
                    frame[p.second].dirty = 0;
            } // end of if
        } // end of else
        counter++;
    } // end of for
        // print out stack
        if (debug == true)
        {
            cout << "\nQueue\n";
            queue<pair<int, int>> tmp = stk;
            while (!tmp.empty())
            {
                cout << "\tindex\t" << tmp.front().first << " frame\t" << tmp.front().second << endl;
                tmp.pop();
            }
        }
    return;
} // end of function  fifo
//********************************************************************************
// rand: pick a random frame for the page
void randswap(){
    // first = location, second = frame
    queue<pair<int, int>> stk;

    // for checking recovered
    vector<int> rec;
    rec.resize(pageamount);

    int counter = 0;
    // counter is the pages index
    for (size_t i = 0; i < input.size(); i++){
        // check if we got a printout
        if (input[i][0] == 'p')
        {
            cout << "********************************************************************************\n";
            printout();
            cout << "********************************************************************************\n";
            counter--;
        }
        else if (input[i][0] == 'd')
        {
            debug = true;
            cout << "\nDebug on\n";
            counter--;
        }
        else if (input[i][0] == 'n')
        {
            debug = false;
            cout << "\nDebug off\n";
            counter--;
        }
        else if(!input[i].empty())
        {
            // check the size and get the size
            int index = -1, inflag = 0;
            if (input[i].size() == 3)
                index = input[i][2] - '0'; // get the page location
            else if(input[i].size() > 3){ // its a double
                // its a double digit
                string tmp;
                for (size_t j = 2; j < input[i].size(); j++)
                {
                    tmp.push_back(input[i][j]);
                }
                index = stoi(tmp);
                if (debug == true)
                {
                    cout << "\ntmp " << tmp;
                    cout << "\nindex " << index;
                }
            } // end of else
            // check if its already in the frame
            for (int j = 0; j < frameamount; j++)
            {
                if (frame[j].obj == index)
                {
                    // flag for if we put it in or not
                    inflag = 1;
                    // counters
                    frame[j].last_use = counter + 1;
                    // its a duplicate so we mark it as recovered
                    page[index].disk = 0;
                }
            }
            // first x amount
            if (counter < frameamount)
            {
                page[index].type = "Mapped";
                page[index].framenum = to_string(counter);
                frame[counter].obj = index; // mark the page num
                frame[counter].first_use = counter + 1, frame[i].last_use = counter + 1;
                frame[counter].inuse = 1; // mark the frame inuse
                stk.push(make_pair(counter, counter));

                // counter
                missed++;
                // recovered
                if (input[i][0] == 'w')
                    rec[counter] = 1, frame[counter].dirty = 1;
            }
            if(inflag == 0 && (signed)i >= frameamount){
                // pick a random frame then follow the usual flow
                // get the pair
                pair<int, int> p;
                p = stk.front();
                stk.pop();
                p.second = rand() % ((frameamount - 1) - 0 + 1) + 0; // get the random frame for this;
                cout << "\nfirst " << p.first << " sec " << p.second << " index " << index;
                // update old page values
                page[p.first].type = "Taken";
                page[p.first].framenum = "(unassigned)";

                // update new page values
                page[index].type = "Mapped";
                page[index].framenum = to_string(p.second); // frame
                // for ondisk, we mark it as 1 if we steal the frame to write because the frame already has a page
                if (frame[p.second].obj != -1)
                    page[index].disk = 1;

                // update frame obj
                frame[p.second].obj = index;

                // push the new values to the stack
                stk.push(make_pair(index, p.second));

                // counters
                frame[p.second].first_use = counter + 1, frame[p.second].last_use = counter + 1;
                missed++;
                taken++;
                // frames recovered from disk, if we wrote to the page and then later rewrote/read to the page
                // the recover goes up
                if (rec[index] == 1)
                    recovered++;
                if (input[i][0] == 'w' && rec[index] == 0)
                    rec[index] = 1;
                if (input[i][0] == 'w')
                    frame[p.second].dirty = 1;
                else
                    frame[p.second].dirty = 0;
            } // end of if
        } // else
        counter++;
    } // end of for
    // print out stack
    if (debug == true)
    {
        cout << "\nQueue\n";
        queue<pair<int, int>> tmp = stk;
        while (!tmp.empty())
        {
            cout << "\tindex\t" << tmp.front().first << " frame\t" << tmp.front().second << endl;
            tmp.pop();
        }
    }
    return;
} // end of function
//********************************************************************************
void lfu(){
    // first = location, second = frame
    queue<pair<int, int>> stk;

    // for checking recovered
    vector<int> rec;
    rec.resize(pageamount);

    int counter = 0;
    // counter is the pages index
    for (size_t i = 0; i < input.size(); i++)
    {
        // check if we got a printout
        if (input[i][0] == 'p')
        {
            cout << "********************************************************************************\n";
            printout();
            cout << "********************************************************************************\n";
            counter--;
        }
        else if (input[i][0] == 'd')
        {
            debug = true;
            cout << "\nDebug on\n";
            counter--;
        }
        else if (input[i][0] == 'n')
        {
            debug = false;
            cout << "\nDebug off\n";
            counter--;
        }
        else if(!input[i].empty())
        {
            // check the size and get the size
            int index = -1, inflag = 0;
            if (input[i].size() == 3)
                index = input[i][2] - '0'; // get the page location
            else if(input[i].size() > 3)
            { // its a double
                string tmp;
                for (size_t j = 2; j < input[i].size(); j++)
                {
                    tmp.push_back(input[i][j]);
                }
                index = stoi(tmp);
                if (debug == true)
                {
                    cout << "\ntmp " << tmp;
                    cout << "\nindex " << index;
                }
            } // end of else
            // check if its already in the frame
            for (int j = 0; j < frameamount; j++)
            {
                if (frame[j].obj == index)
                {
                    // flag for if we put it in or not
                    inflag = 1;
                    // counters
                    frame[j].last_use = counter + 1;
                    // its a duplicate so we mark it as recovered
                    page[index].disk = 0;
                }
            }
            // first x amount
            if ((signed)i < frameamount)
            {
                page[index].type = "Mapped";
                page[index].framenum = to_string(counter);
                frame[counter].obj = index; // mark the page num
                frame[counter].first_use = counter, frame[counter].last_use = counter;
                frame[counter].inuse = 1; // mark the frame inuse
                frame[counter].least = 1;
                stk.push(make_pair(counter, counter));

                // counter
                missed++;
                // recovered
                if (input[i][0] == 'w')
                    rec[counter] = 1, frame[counter].dirty = 1;
            }
            if (inflag == 0 && counter >= frameamount)
            {
                // pick a random frame then follow the usual flow
                // get the pair
                pair<int, int> p;
                p = stk.front();
                stk.pop();
                // get the least used frame by going through frame[].least
                int min = 1000000;
                for (size_t j = 0; j < frame.size(); j++){
                    if (frame[j].least < min){ 
                        min = frame[j].least, p.second = j;
                        // cout << "\nframe " << frame[j].least << " i " << i;
                    }
                }
                // increment the least used
                frame[p.second].least++;

                // update old page values
                // cout << "\nfirst " << p.first << " sec " << p.second << " index " << index;
                page[p.first].type = "Taken";
                page[p.first].framenum = "(unassigned)";

                // update new page values
                page[index].type = "Mapped";
                page[index].framenum = to_string(p.second); // frame
                // for ondisk, we mark it as 1 if we steal the frame to write because the frame already has a page
                if (frame[p.second].obj != -1)
                    page[index].disk = 1;

                // update frame obj
                frame[p.second].obj = index;

                // push the new values to the stack
                stk.push(make_pair(index, p.second));

                // counters
                frame[p.second].first_use = counter + 1, frame[p.second].last_use = counter + 1;
                missed++;
                taken++;
                // frames recovered from disk, if we wrote to the page and then later rewrote/read to the page
                // the recover goes up
                if (rec[index] == 1)
                    recovered++;
                if (input[i][0] == 'w' && rec[index] == 0)
                    rec[index] = 1;
                if (input[i][0] == 'w')
                    frame[p.second].dirty = 1;
                else
                    frame[p.second].dirty = 0;
            } // end of if
        }
        counter++;
    }     // end of for
    // print out stack
    if (debug == true)
    {
        cout << "\nQueue\n";
        queue<pair<int, int>> tmp = stk;
        while (!tmp.empty())
        {
            cout << "\tindex\t" << tmp.front().first << " frame\t" << tmp.front().second << endl;
            tmp.pop();
        }
        cout << "\nFrame with least recently used variable";
        for (auto &i : frame)
            cout << "\nLeast: " << i.least;
    }
    return;
}
//********************************************************************************
// output
void printout(){
    //*********************************************************
    // get values for prinout
    // amount of pages used
    int mapped = 0;
    for(auto& i : page)
        if(i.type.compare("Unmapped") != 0)
            mapped++;
    int written = 0;
    // bandaid fix for written to disk
    if(recovered != 0){
        // look at how many ondisk 1's we got
        for(size_t i = 0; i < page.size(); i++)
            if(page[i].disk == 1)
                written++;
    }
    else
        // mark all ondisk as 0
        for(size_t i = 0; i < page.size(); i++)
            page[i].disk = 0;
    // get refferenced
    for(size_t i = 0; (unsigned)i < input.size(); i++){
        if(!input[i].empty()){
            if(input[i][0] == 'r' || input[i][0] == 'w')
                referenced++;
        }
    }
    //*********************************************************
    // output printing section
    // Frames
    cout << "\nFrames\n";
    int counter = 0;
    for(auto& i : frame){
        cout << "\t" << counter << " inuse:\t" << i.inuse << " dirty:\t" << i.dirty << " first_use:\t" << i.first_use << " last_use:\t" << i.last_use << endl;
        counter++;
    }

    // Pages
    cout << "\nPages\n";

    int counter1 = 0;
    for(auto& i : page){
        cout << "\t" << counter1 << " type:\t" << i.type << " ondisk:\t" << i.disk << " framenum:\t" << i.framenum << endl;
        counter1++;
    }
    cout << "\nPages Referenced\t" << referenced << endl;
    cout << "Pages Mapped\t" << mapped << endl;
    cout << "Pages Missed\t"  << missed << endl;
    cout << "Frames Taken\t"  << taken << endl;
    cout << "Frames written on disk\t" << written << endl;
    cout << "Frames recovered from disk\t" << recovered << endl;

    // format
    cout << endl;

    extraprintout(mapped);
    return;
}
void extraprintout(int mapped){
    // lines on top
    for(int i = 0; i < pageamount + 5; i++)
        cout << " -";

    // inbetwen line
    cout << "\nReference ";
    char topletter = 'A', lastletter = 'A';
    int counter = 0;
    for(int i = 0; i < referenced; i++){
        if(counter == mapped){
            topletter -= 1;
            lastletter = topletter;
            topletter = 65,counter = 0;
        }
        cout << topletter << " ";
        topletter++, counter++;
    }
    // line on bottom
    cout << endl;
    for (int i = 0; i < pageamount + 5; i++)
        cout << " -";
    
    // char printout
    char letter1 = 65, flag = 0;
    for (int i = 0; i < frameamount; i++){
        cout << endl << i << "         "; // the number

        // rest of the line
        for(int j = 0; j < frameamount - 1; j++){
            if(flag == 0){
                letter1 += i;
                cout << letter1;
                flag = 2;
            }
            if (flag == 1){
                letter1 += i;
                for (int k = 0; k < i + 1; k++)
                    cout << " ";
                cout << letter1;
                flag++;
            }
          
            // rest of the letters
            for(int k = 0; k < frameamount + frameamount; k++)
                cout << " ";
            if(j == 0)
                letter1 += frameamount;
            cout << letter1;
            letter1 += frameamount;
            if(letter1 > lastletter){
                int difference = (int)letter1 - (int)(lastletter);
                letter1 = 65 + difference - 1;
            }
            
        }
        letter1 = 65;
        flag = 1;
    }

    cout << endl;
    cout << "I cant figure out the spacing of this but its close\n";
    return;
}