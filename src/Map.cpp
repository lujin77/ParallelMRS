/* 
 * File:   Map.cpp
 * Author: yndx
 * 
 * Created on 2012年12月20日, 下午12:52
 */

#include "Map.h"
#include "PipePool.h"
#include "IpcMgr.h"

int set_nonblock(int sockfd) { // set socket to non blocking
    int arg, i;

    if ((arg = fcntl(sockfd, F_GETFL, NULL)) < 0) {
        printf("error getting socket flag for fd %i: fcntl(..., F_GETFL): %i\n", sockfd, errno);
        return -1;
    }
    // set O_NONBLOCK flag
    arg |= O_NONBLOCK;
    if ((i = fcntl(sockfd, F_SETFL, arg)) < 0) {
        printf("error setting socket flag for fd %i: fcntl(..., F_SETFL): %i\n", sockfd, errno);
        return -1;
    }
    return i;
}

typedef struct {
    FILE * fp;
    int id;
} map_reader_t;

//int shuffle(char * str) {
//    char key[MAX_STR_LEN];
//    char * item;
//    int index;
//    int slot;
//    get_key(str, key);
//    slot = BKDRHash(key) % REDUCE_NUM;
//    //fprintf(stderr, "str: %s send to slot:%d\n", str, slot);
//    //dispatch to mapper
//    /*
//    while (PipePool::getInst()->_pipes4reduce[slot]->get_free_item(item, index) != 0) {
//        fprintf(stderr, "[NOTICE] Map::shuffle() | reducer[%d] pipe is full, "
//                "wait 1 sec to dispatch!\n", slot);
//        sleep(PIPE_SLEEP_TIME);
//    }
//    strcpy(item, str);
//    PipePool::getInst()->_pipes4reduce[slot]->push_item(index);
//     * */
//}

void *map_recieve(void * argv) {
    map_reader_t * p_reader = (map_reader_t *) argv;
    FILE * fp = p_reader->fp;
    int id = p_reader->id;
    char r_buf[MAX_STR_LEN];
    char key[MAX_STR_LEN];
    r_buf[0] = '\0';
    key[0] = '\0';
    long count = 0;
    while (fgets(r_buf, MAX_STR_LEN, fp) != NULL) {
        //fprintf(stderr, "[DEBUG] Map::recieve() | mapper[%d] recieve: %s", id, r_buf);
        //fprintf(stdout, "%s", r_buf); //print map's result on screen
        //shuffle(r_buf);
        PipePool::getInst()->_mediator->push_back_r(r_buf); //send to shuffle's pipe
        r_buf[0] = '\0';
        key[0] = '\0';
        ++count;
        //        if (count % 1000 == 0)
        //                fprintf(stderr, "map processed lines: %ld\n", count);
    }
    fprintf(stderr, "[TRACE] Map::recieve() | mapper[%d] recieve data all complete!\n", id, r_buf);
}

Map::Map(char * map_bin, int index) : ThreadBase(false), _index(index) {
    strcpy(_map_bin, map_bin);
}

Map::~Map() {
}

void Map::Run() {
    int ret;
    pid_t pid;
    if ((pid = fork()) == 0) { // 子进程
        //fprintf(stderr,"[MAP] clild_process: %d\n", getpid());
        //redirect child process's stdio
        IpcMgr::getInst()->redirect_child_io(_index, PROC_MAP);
        /* 使用exec执行命令 */
        ret = execl("/bin/sh", "sh", "-c", _map_bin, (char *) 0);
        if (0 != ret) {
            fprintf(stderr, "[ERROR] Map::Run() | invoke subprocess failed!\n");
            return;
        }
    } else { // 父进程
        //fprintf(stderr,"[MAP] parent_process: %d\n", getpid());
        IpcMgr::getInst()->close_useless_in_parent_r(_index, PROC_MAP);
        /* 现在可向pipe_parent2child[1]中写数据，并从pipe_child2parent[0]中读结果 */
        int pipe_child2parent_r = IpcMgr::getInst()->get_pipe(_index, PROC_MAP, CHILD2PARENT, PIPE_READ);
        FILE * fp;
        fp = fdopen(pipe_child2parent_r, "r");
        if (fp == NULL) {
            fprintf(stderr, "[ERROR] Map::Run() | fdopen pipe_child2parent faild!\n");
            return;
        }

        //create reciever
        map_reader_t reader;
        reader.fp = fp;
        reader.id = _index;
        pthread_t thr_id;
        ret = pthread_create(&thr_id, NULL, map_recieve, &reader);
        if (ret != 0) {
            fprintf(stderr, "[ERROR] Map::Run() | create recieve thread faild!\n");
            return;
        }

        int pipe_parent2child_w = IpcMgr::getInst()->get_pipe(_index, PROC_MAP, PARENT2CHILD, PIPE_WRITE);
        //write data from pipe
        int slot;
        char * w_buf;
        do {
            //fprintf(stderr, "try to read one line from safepipe\n");
            PipePool::getInst()->_pipes4map[_index]->wait_pop_front(w_buf, slot);
            //fprintf(stderr, "read one line from safepipe success\n");
            if (!strcmp(w_buf, END_PIPE)) {
                //fprintf(stderr, "exit safepipe\n");
                break;
            }
            //PipePool::getInst()->_mediator->push_back_r(w_buf);
            write(pipe_parent2child_w, w_buf, strlen(w_buf));
            PipePool::getInst()->_pipes4map[_index]->notify_used_end(slot);
            //std::cout << "thread[" << _index << "] process str:" << w_buf << "\n";
        } while (1);
        //        printf("[TRACE] Map::Run() | 0\n");
        //        strcpy(w_buf, "end of file\n");
        //        write(pipe_parent2child[1], w_buf, strlen(w_buf));
        //        w_buf[0] = EOF;
        //        write(pipe_parent2child[1], w_buf, 1);
        //printf("[TRACE] Map::Run() | 1, pipe_id: %d\n", pipe_parent2child[1]);
        int ret;
        ret = close(pipe_parent2child_w); // 关闭写管道
        //printf("close mapper[%d]'s pipe_parent2child[1]->%d ret=%d\n", _index, pipe_parent2child_w, ret);
        //printf("[TRACE] Map::Run() | 2 thr_id:%ld\n", _index);
        /* 读取pipe_child2parent[0]中的剩余数据 */
        pthread_join(thr_id, NULL);
        fclose(fp);
        close(pipe_child2parent_r); // 关闭读管道
        /* 使用wait系列函数等待子进程退出并取得退出代码 */
        int status;
        waitpid(pid, &status, 0);
        if (status != 0) {
            fprintf(stderr, "[WARNING] Map::Run() | some error happened in subprocess!\n");
        }
        fprintf(stderr, "[TRACE] Map::Run() | mapper[%d] task complete!\n", _index);
    }
}



