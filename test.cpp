#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <cmath>
#include <algorithm>
#include <sched.h>

using namespace std;

// void *thread0(void *dummy);
// void *thread1(void *dummy);
// void *thread2(void *dummy);

bool* result = nullptr;
int arrange = 0;
int field = 0;

int current = 0;
int thread_num = 0;

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


void *thread(void* i) {
    // pthread_rwlock_rdlock(&ready);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程 
    // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置立即取消 
    int start = *(int*) i;
    start++;
    start *= field;

    int end = start + field;
    if (end >= arrange) {
        end = arrange;
    }
    if (*(int*) i == thread_num-2) {
        end = arrange;
    }

    int th_cur = -1;

    while (1) {

        pthread_mutex_lock(&mutex1);

        // cout << "begin wait" << endl;

        pthread_cond_wait(&cond, &mutex1);

        // cout << "end wait" << endl;

        pthread_rwlock_rdlock(&rwlock);
        if (th_cur != current) {
            th_cur = current;
            int loc = start;
            while (loc%th_cur != 0 && loc < end) {
                loc++;
            }
            // cout << start << ":" << loc << ":" << end << endl;
            while (loc < end) {
                result[loc] = false;
                // cout << "filter " << loc << endl;
                loc += th_cur;
            }
        }
        pthread_rwlock_unlock(&rwlock);
        // cout << "here4" << endl;

        pthread_mutex_unlock(&mutex1);
    }
    return nullptr;

}

void *master(void* i) {
    // cout << "master begin" << endl;
    for (int start = 2; start < field; start++) {
        // cout << "master in for" << endl;
        if (result[start]) {
            // cout << "master here1" << endl;
            pthread_mutex_lock(&mutex1);
// cout << "master here2" << endl;
            pthread_rwlock_wrlock(&rwlock);
            current = start;
            pthread_rwlock_unlock(&rwlock);
// cout << "master here3" << endl;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex1);
// cout << "master here4" << endl;
            pthread_rwlock_rdlock(&rwlock);
            int loc = start+start;
            while (loc < field) {
                result[loc] = false;
                loc += start;
            }
            pthread_rwlock_unlock(&rwlock);
            // cout << "master here5" << endl;

        }
    }
    // cout << "master end" << endl;
    return nullptr;
}

int main()
{
    // pthread_rwlock_init(&ready, NULL);
    int n = 0;
    cin >> n;
    if (n < 0) return 1;

    arrange = (int) pow(2, n)*1000;


    result = new bool[arrange];
    fill(result, result+arrange, 1);

    thread_num = (int) ceil(sqrt(arrange)/2);

    int* domain = new int[thread_num];
    for (int i = 0; i < thread_num; i++) {
        domain[i] = i;
    }
    field = (int) (arrange/thread_num);

    pthread_t* tids = new pthread_t[n];

    pthread_t tid_main;


    
    for (int i = 0; i < thread_num-1; i++) {
        pthread_create(&tids[i], nullptr, thread, &domain[i]);
    }
    sleep(1);

    pthread_create(&tid_main, nullptr, master, nullptr);


// cout << "join" << endl;
    pthread_join(tid_main, nullptr);
// cout << "after join" << endl;
//     // for (int i = 0; i < thread_num-1; i--) {
//     //     pthread_cancel(tids[i]);
//     // }
// cout << "after cancel" << endl;

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex1);
    pthread_cond_destroy(&cond);

    for (int i = 2; i < arrange; i++) {
        if (result[i])
        cout << i << endl;
    }
    
    return 0;
}


