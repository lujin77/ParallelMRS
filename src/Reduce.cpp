/* 
 * File:   Reduce.cpp
 * Author: syani
 * 
 * Created on 2012年12月21日, 下午2:00
 */

#include "Reduce.h"
#include "PipePool.h"
#include "IpcMgr.h"
#include "Timer.h"

typedef struct {
    FILE * fp;
    int id;
} reduce_reader_t;

void *reduce_recieve(void * argv) {
    reduce_reader_t * p_reader = (reduce_reader_t *) argv;
    FILE * fp = p_reader->fp;
    int id = p_reader->id;
    char r_buf[MAX_STR_LEN];
    char key[MAX_STR_LEN];
    r_buf[0] = '\0';
    key[0] = '\0';
    while (fgets(r_buf, MAX_STR_LEN, fp) != NULL) {
        //fprintf(stderr, "[DEBUG] Reduce::recieve() | reducer[%d] recieve: %s", id, r_buf);
        PipePool::getInst()->_pipes4dump[id]->append(r_buf);
        r_buf[0] = '\0';
        key[0] = '\0';
    }
    //确保所有数据都正确dump
    PipePool::getInst()->_pipes4dump[id]->dump();
    fprintf(stderr, "[TRACE] Reduce::recieve() | reducer[%d] recieve data all complete!\n", id, r_buf);
}

Reduce::Reduce(char * reduce_bin, int index) : ThreadBase(false), _index(index) {
    strcpy(_reduce_bin, reduce_bin);
}

Reduce::~Reduce() {
}

void Reduce::Run() {
    /**sort reducer's input pipe in memory*/
    char timer_str[256];
    snprintf(timer_str, 256, "shuffle_sort-%d", _index);
    Timer timer;
    timer.start_timer(timer_str);
    PipePool::getInst()->_mediator->sort_r(_index);
    timer.end_timer();

    /**create subprocess to complete task*/
    int ret;
    pid_t pid;
    if ((pid = fork()) == 0) { // 子进程
        //fprintf(stderr,"[REDUCE] clild_process: %d\n", getpid());
        //redirect child process's stdio
        IpcMgr::getInst()->redirect_child_io(_index, PROC_REDUCE);
        /* 使用exec执行命令 */
        ret = execl("/bin/sh", "sh", "-c", _reduce_bin, (char *) 0);
        if (0 != ret) {
            fprintf(stderr, "[ERROR] Reduce::Run() | invoke subprocess failed!\n");
            return;
        }
    } else { // 父进程
        //fprintf(stderr,"[REDUCE] parent_process: %d\n", getpid());
        IpcMgr::getInst()->close_useless_in_parent_r(_index, PROC_REDUCE);
        /* 现在可向pipe_parent2child[1]中写数据，并从pipe_child2parent[0]中读结果 */
        int pipe_child2parent_r = IpcMgr::getInst()->get_pipe(_index, PROC_REDUCE, CHILD2PARENT, PIPE_READ);
        FILE * fp;
        fp = fdopen(pipe_child2parent_r, "r");
        if (fp == NULL) {
            fprintf(stderr, "[ERROR] Reduce::Run() | fdopen pipe_child2parent faild!\n");
            return;
        }

        //create reciever
        reduce_reader_t reader;
        reader.fp = fp;
        reader.id = _index;
        pthread_t thr_id;
        ret = pthread_create(&thr_id, NULL, reduce_recieve, &reader);
        if (ret != 0) {
            fprintf(stderr, "[ERROR] Reduce::Run() | create recieve thread faild!\n");
            return;
        }

        int pipe_parent2child_w = IpcMgr::getInst()->get_pipe(_index, PROC_REDUCE, PARENT2CHILD, PIPE_WRITE);
        //write data from pipe
        int ret;
        char w_buf[MAX_STR_LEN];
        do {
            ret = PipePool::getInst()->get_reduce_task_r(_index, w_buf);
            if (ret != 0) {
                break;
            }
            //fprintf(stdout, "reduce[%d] %s", _index, w_buf);
            //PipePool::getInst()->_pipes4dump[_index]->append(w_buf);
            write(pipe_parent2child_w, w_buf, strlen(w_buf));
        } while (1);

        ret = close(pipe_parent2child_w); // 关闭写管道
        /* 读取pipe_child2parent[0]中的剩余数据 */
        pthread_join(thr_id, NULL);
        fclose(fp);
        close(pipe_child2parent_r); // 关闭读管道
        /* 使用wait系列函数等待子进程退出并取得退出代码 */
        int status;
        waitpid(pid, &status, 0);
        if (status != 0) {
            fprintf(stderr, "[WARNING] Reduce::Run() | some error happened in subprocess!\n");
        }
        fprintf(stderr, "[TRACE] Reduce::Run() | reducer[%d] task complete!\n", _index);
    }
}

