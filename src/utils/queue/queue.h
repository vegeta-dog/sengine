#ifndef _UTILS_QUEUE_H_
#define _UTILS_QUEUE_H_

#include <queue>
#include <mutex>
namespace ThreadSafeQueue
{
    template <typename T>
    class queue
    {
    public:
        queue()=default;
        ~queue()=default;

        void push(const T &data)
        {
            rwMutex.lock();
            que.push(data);
            rwMutex.unlock();
        };

        /**
         * @brief 返回队列首部元素
         * 
         * @return T 
         */
        T front()
        {
            T ret;
            rwMutex.lock();
            ret = que.front();
            rwMutex.unlock();
            return ret;
        };


        void pop()
        {
            rwMutex.lock();
            que.pop();
            rwMutex.unlock();
        };

        /**
         * @brief 返回队首元素并pop
         * 
         * @return T 队首元素
         */
        T getFront()
        {
            T ret;
            rwMutex.lock();
            ret = que.front();
            que.pop();
            rwMutex.unlock();
            return ret;
        }



        /**
         * @brief 返回队列是否为空
         * 
         * @return true 
         * @return false 
         */
        bool empty()
        {
            bool ret;
            rwMutex.lock();
            ret = que.empty();
            rwMutex.unlock();

            return ret;
        };
        /**
         * @brief 返回队列大小（不可靠）
         *
         * @return unsigned int
         */
        unsigned int size()
        {
            return que.size();
        }

    private:
        std::queue<T> que;
        std::mutex rwMutex; // 读写锁
    };
}
#endif