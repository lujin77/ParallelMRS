/* 
 * File:   main.cpp
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午12:48
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include "Timer.h"
#include "JobTracker.h"
#include "SafePipe.h"
#include "DumpBuff.h"
#include "Config.h"
#include "Global.h"

using namespace std;

void *produce(void *args) {
    cout << "start producer ...\n";
    SafePipe * pipe = (SafePipe *) args;
    char * str;
    int index;
    for (int i = 0; i < 10; ++i) {
        while (1 == pipe->get_free_item(str, index)) {
            cout << "sleeping ...\n";
            sleep(1);
        }
        snprintf(str, 256, "this is a test %d", i);
        pipe->push_item(index);
        cout << "produce at index: " << index << ", value=" << str << "\n";
    }
    pipe->get_free_item(str, index);
    strcpy(str, "EOF");
    pipe->push_item(index);
}

void *consume(void *args) {
    cout << "start comsumer ...\n";
    SafePipe * pipe = (SafePipe *) args;
    char * str;
    int index;
    while (1) {
        pipe->wait_pop_front(str, index);
        if (!strcmp(str, "EOF"))
            break;
        cout << "consume value[" << index << "], num = " << str << "\n";
        pipe->notify_used_end(index);
    }
}

int main(int argc, char** argv) {

//    char key[MAX_STR_LEN];
//    char value[MAX_STR_LEN];
//    char str[] = "hello world this is a test";
//    //get_key(str, key, ' ', 2);
//    split_key_value(str, key, value, ' ', 2);
//    cout << "key: " << key << "|" << endl;
//    cout << "value: " << value << "|" << endl;
//    return 0;
    
    //    SafePipe * pipe = new SafePipe(3);
    //    pipe->init();
    //    pthread_t producer, consumer;
    //    pthread_create(&producer, NULL, produce, pipe);
    //    pthread_create(&consumer, NULL, consume, pipe);
    //    pthread_join(producer, NULL);
    //    pthread_join(consumer, NULL);
    //    cout << "[NOTICE] all is done" << endl;
    //    delete pipe;

   
    //load config
    int map_num, reduce_num;
    std::string input_file, output_dir, map_bin, reduce_bin;
    if (argc != 2) {
        std::cerr << "[WARNING] usage of ParallelMRS: ./ParallelMRS config.txt\n";
        return -1;
    } else {
        Config configSettings(argv[1]);
        map_num = configSettings.Read("map_num", 0);
        reduce_num = configSettings.Read("reduce_num", 0);
        map_bin = configSettings.Read("map_bin", map_bin);
        reduce_bin = configSettings.Read("reduce_bin", reduce_bin);
        input_file = configSettings.Read("input_file", input_file);
        output_dir = configSettings.Read("output_dir", output_dir);
        //        std::cout << "map_num: " << g_map_num << endl;
        //        std::cout << "reduce_num: " << g_reduce_num << endl;
        //        std::cout << "map_bin: " << map_bin << endl;
        //        std::cout << "reduce_bin: " << reduce_bin << endl;
        //        std::cout << "input_file: " << input_file << endl;
        //        std::cout << "output_dir: " << g_save_path << endl;
    }

    Timer timer;
    timer.start_timer("total");
//    JobTracker * tracker = new JobTracker("word_10MB.txt", "./output",
//            "python mapper.py", "python reducer.py", 1, 1);
    JobTracker * tracker = new JobTracker(input_file.c_str(), output_dir.c_str(),
            map_bin.c_str(), reduce_bin.c_str(), map_num, reduce_num);
    tracker->init();
    tracker->run();
    delete tracker;
    timer.end_timer();
    
    /*
    int port;
    std::string ipAddress;
    std::string username;
    std::string password;
    const char ConfigFile[] = "test.cfg";
    Config configSettings(ConfigFile);

    port = configSettings.Read("port", 0);
    ipAddress = configSettings.Read("ipAddress", ipAddress);
    username = configSettings.Read("username", username);
    password = configSettings.Read("password", password);
    std::cout << "port:" << port << std::endl;
    std::cout << "ipAddress:" << ipAddress << std::endl;
    std::cout << "username:" << username << std::endl;
    std::cout << "password:" << password << std::endl;
     * */
    return 0;
}
