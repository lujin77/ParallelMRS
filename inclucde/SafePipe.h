/* 
 * File:   SafePipe.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午1:20
 */

#ifndef __SAFEPIPE_H__
#define	__SAFEPIPE_H__

#include "Global.h"

class SafePipe {
public:

    /**
     * @brief 构造函数
     * @param 无
     * @return 无
     */
    SafePipe(int maxnum, int line_size = MAX_STR_LEN) :
    _queue(NULL), _flag(NULL), _front(0), _tail(0), _length(0),
    _maxnum(maxnum), _line_size(line_size) {

    }

    /**
     * @brief 析构函数
     * @param 无
     * @return 无
     */
    ~SafePipe() {
        for (int i = 0; i < _maxnum; ++i) {
            if (NULL != _queue[i]) {
                delete [] _queue[i];
            }
        }

        if (NULL != _queue) {
            delete[] _queue;
        }

        if (NULL != _flag) {
            delete[] _flag;
        }

        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cond);
    }

    /*
     *@brief 初始化
     *@return 返回是否成功
     *@retval 非0 不成功
     *@retval  0 成功
     **/
    int init() {
        int ret = 0;
        _queue = new (std::nothrow) char*[_maxnum];
        if (NULL == _queue) {
            return -1;
        }

        for (int i = 0; i < _maxnum; ++i) {
            _queue[i] = new (std::nothrow) char[_line_size];
            if (NULL == _queue[i]) {
                return -1;
            }
        }

        _flag = new (std::nothrow) int[_maxnum];
        if (NULL == _flag) {
            return -1;
        }

        for (int i = 0; i < _maxnum; i++) {
            _flag[i] = 0;
        }

        ret = pthread_mutex_init(&_mutex, NULL);
        if (0 != ret) {
            return -1;
        }

        ret = pthread_cond_init(&_cond, NULL);
        if (0 != ret) {
            return -1;
        }

        return 0;
    }

    /*
     *@brief 从队列中得到空的元素，用于生产者获取空间保存数据
     *@param[out] item 保存新得到元素的地址
     *@param[out] index 得到的元素所在数组中的位置
     *@return
     *@retval 1  队列满
     *@retval  0 成功
     **/
    int get_free_item(char *& item, int &index) {
        //printf("get_free_item, ");
        pthread_mutex_lock(&_mutex);
        if (!full() && (_flag[_tail] == 0)) {
            //printf("queue is not full\n");
            item = _queue[_tail]; ///< 得到队尾元素
            index = _tail; ///< 标记得到的元素的位置
            _flag[index] = 0; ///<表明数据暂时不可用

            _tail = (_tail + 1) % _maxnum;
            ++_length;

            pthread_mutex_unlock(&_mutex);
            return 0;
        } else {
            //printf("queue is full, length = %d, _tail=%d\n", _length, _tail);
            pthread_mutex_unlock(&_mutex);
            return 1;
        }
    }

    /*
     *@brief 处理完数据，通知队列，表明该数据可以被消费者处理
     *@param[in] index 数据所在位置
     *@retval  0 成功
     **/
    int push_item(int index) {
        pthread_mutex_lock(&_mutex);
        _flag[index] = 1; ///<表明数据可用,通知消费者进行进行处理
        pthread_cond_signal(&_cond);
        pthread_mutex_unlock(&_mutex);
        return 0;
    }

    /*
     *@brief 消费者处理完数据，通知队列，表明该数据可以被生产者处理
     *@param[in] index 数据所在位置
     *@retval  0 成功
     **/
    int notify_used_end(int index) {
        pthread_mutex_lock(&_mutex);
        _flag[index] = 0; ///<表明数据可用,通知生产者进行进行处理
        pthread_mutex_unlock(&_mutex);
        return 0;
    }

    /*
     *@brief 判断队列是否为满
     *@retval true为空
     *@retval false 不为空
     **/
    bool full() {
        return (_length == _maxnum);
    }

    /*
     *@brief 判断队列是否为空
     *@retval true为空
     *@retval false 不为空
     **/
    bool empty() {
        return (_length == 0);
    }

    /*
     *@brief 压出队首元素到item中
     *@param[out] item 保存待压出的数据的地址
     *@param[out] index 保存压出的数据的所在位置
     *@return 返回是否成功
     *@retval 非0 不成功
     *@retval  0 成功
     **/
    int wait_pop_front(char *& item, int &index) {
        pthread_mutex_lock(&_mutex);
        while (empty() || (_flag[_front] == 0)) {
            //fprintf(stderr,"_flag[_front] = %d\n", _flag[_front]);
            pthread_cond_wait(&_cond, &_mutex);
        }

        item = _queue[_front];
        index = _front;

        _front = (_front + 1) % _maxnum;
        --_length;

        pthread_mutex_unlock(&_mutex);

        return 0;
    }
    
    //for testing
    int show_state() {
        std::cout << "front: " << _front << std::endl;
        std::cout << "tail: " << _tail << std::endl;
        std::cout << "length: " << _length << std::endl;
        std::cout << "flag[front]: " << _flag[_front] << std::endl ;
    }

private:
    char ** _queue; ///<数据队列
    int * _flag;
    int _front;
    int _tail;
    int _length;
    pthread_mutex_t _mutex; ///<互斥信号量
    pthread_cond_t _cond; ///<条件变量

    int _maxnum; ///<最大个数
    int _line_size;
    bool _is_init;
};

#endif	/* SAFEPIPE_H */

