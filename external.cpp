#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <QDebug>
#include <string.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>

#define NUM_THREADS 7
//#define MY_MQ_NAME "/my_mq"

//static struct mq_attr my_mq_attr;
//static mqd_t my_mq;


struct proc_data {
    double temp;
    double down_temp;
    double up_temp;
} temps;

const std::size_t bufsize = 8999;


float calcDT(float parent) {
    return (parent + temps.temp) / 2.0;
}

float calcUT(float child_a, float child_b) {
    return (temps.temp + child_a + child_b) / 3.0;
}

int main(int argc, char *argv[])
{



    QCoreApplication a(argc, argv);
    char sendBuf[bufsize];
    char parentbuf[bufsize];
    char childLbuf[bufsize];
    char childRbuf[bufsize];
    double initT, parentT, childLT, childRT;
    int id;
    mqd_t mq, parent, left_child, right_child;

    // Mailbox ID's
    const char * mailArray[] = {"/70", "/71", "/72", "/73", "/74", "/75", "/76"};

    // check if ID is within range
    if (atoi(argv[1]) < 7 && atoi(argv[1]) >= 0) {
        id = atoi(argv[1]);
        initT = atof(argv[2]);
    }
    else
        perror("ID is out of bounds.");

    mq = mq_open(mailArray[id], O_RDONLY | O_CREAT | O_EXCL, 0666, NULL);

    if (mq == -1)
        perror("Error creating mailbox");
    else
        qDebug() << "created mailboxes" << endl;

    temps.temp = initT;

    bool done = false;
    while (!done) {
        // do work

        switch (id) {
            case 0:
                left_child = mq_open(mailArray[1],   O_RDONLY);
                right_child = mq_open(mailArray[2],  O_RDONLY);

                //send to child
                sprintf(sendBuf, "%f", temps.temp);
                if (mq_send(left_child, sendBuf, strlen(sendBuf)+1, 0) != -1
                        || mq_send(right_child, sendBuf, strlen(sendBuf)+1, 0) != -1)
                    qDebug() << "P0 sent to P1 and P2";
                else
                       qDebug() << "P0 ERROR SENDING TO CHILD";//perror("Error writing to child.");

                //if receiving from child
                if (mq_receive(left_child, childLbuf, bufsize, NULL) && mq_receive(right_child, childRbuf, bufsize, NULL)) {
                    childLT = atof(childLbuf);
                    childRT = atof(childRbuf);
                    temps.up_temp = calcUT(childRT, childLT);

                    qDebug() << "P0 received from P1 & P2";

                    //done case
                    if (temps.up_temp - temps.temp < 0.1)
                        done = true;
                    else
                        temps.temp = temps.up_temp;
                }

                break;

            case 1:
                parent = mq_open(mailArray[0],  O_RDONLY);
                left_child = mq_open(mailArray[3],   O_RDONLY);
                right_child = mq_open(mailArray[4],   O_RDONLY);

                // If receiving from parent
                if (mq_receive(parent, parentbuf, bufsize, NULL)) {
                    qDebug() << "received from parent.";
                    parentT = atof(parentbuf);
                    temps.down_temp = calcDT(parentT);

                    sprintf(sendBuf, "%f", temps.down_temp);

                    // Send down_temp to children
                    if (mq_send(left_child, sendBuf , strlen(sendBuf), 0) == -1)
                        perror("Error writing to left child");
                    if (mq_send(right_child, sendBuf, strlen(sendBuf), 0) == -1)
                        perror("Error writing to right child");

                    temps.temp = temps.down_temp;
                }
                // If receiving from children
                else if (mq_receive(left_child, childLbuf, bufsize, NULL) && mq_receive(right_child, childRbuf, bufsize, NULL)) {
                    childLT = atof(childLbuf);
                    childRT = atof(childRbuf);

                    temps.up_temp = calcUT(childLT, childRT);

                    sprintf(sendBuf, "%f", temps.up_temp);

                    // Send up_temp to parent
                    if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
                        perror("Error writing to parent");

                    temps.temp = temps.up_temp;
                }
                break;
//        case 2:
//            parent = mq_open(mailArray[0],  O_NONBLOCK | O_RDONLY);
//            left_child = mq_open(mailArray[5],  O_NONBLOCK | O_RDONLY);
//            right_child = mq_open(mailArray[6],  O_NONBLOCK | O_RDONLY);

//            // If receiving from parent
//            if (mq_receive(parent, parentbuf, bufsize, NULL)) {
//                parentT = atof(parentbuf);
//                temps.down_temp = calcDT(parentT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send down_temp to children
//                if (mq_send(left_child, sendBuf , strlen(sendBuf), 0) == -1)
//                    perror("Error writing to left child");
//                if (mq_send(right_child, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to right child");

//                temps.temp = temps.down_temp;
//            }

//            // If receiving from children
//            else if (mq_receive(left_child, childLbuf, bufsize, NULL) && mq_receive(right_child, childRbuf, bufsize, NULL)) {
//                childLT = atof(childLbuf);
//                childRT = atof(childRbuf);

//                temps.up_temp = calcUT(childLT, childRT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send up_temp to parent
//                if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to parent");

//                temps.temp = temps.up_temp;
//            }
//            break;
//        case 3:
//            parent = mq_open(mailArray[1],  O_NONBLOCK | O_RDONLY);

//            // Receive from parent
//            if (mq_receive(parent, parentbuf, bufsize, NULL)) {
//                parentT = atof(parentbuf);
//                temps.down_temp = calcDT(parentT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send down_temp to parent
//                if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to parent");

//                temps.temp = temps.down_temp;
//            }
//            break;
//        case 4:
//            parent = mq_open(mailArray[1],  O_NONBLOCK | O_RDONLY);

//            // Receive from parent
//            if (mq_receive(parent, parentbuf, bufsize, NULL)) {
//                parentT = atof(parentbuf);
//                temps.down_temp = calcDT(parentT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send down_temp to parent
//                if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to parent");

//                temps.temp = temps.down_temp;
//            }
//            break;

//        case 5:
//            parent = mq_open(mailArray[2],  O_NONBLOCK | O_RDONLY);

//            // Receive from parent
//            if (mq_receive(parent, parentbuf, bufsize, NULL)) {
//                parentT = atof(parentbuf);
//                temps.down_temp = calcDT(parentT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send down_temp to parent
//                if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to parent");

//                temps.temp = temps.down_temp;
//            }
//            break;

//        case 6:
//            parent = mq_open(mailArray[2],  O_NONBLOCK | O_RDONLY);

//            // Receive from parent
//            if (mq_receive(parent, parentbuf, bufsize, NULL)) {
//                parentT = atof(parentbuf);
//                temps.down_temp = calcDT(parentT);

//                sprintf(sendBuf, "%f", temps.down_temp);

//                // Send down_temp to parent
//                if (mq_send(parent, sendBuf, strlen(sendBuf), 0) == -1)
//                    perror("Error writing to parent");

//                temps.temp = temps.down_temp;
//            }
//            break;
        }
    }



    // received end message from parent, sent it to children if they exist
    // unlink my queue

    // close all queues I opened (including mine)mq


    //for (int i = 0; i < 7; i++) {
        mq_close(mq);
        //mq_unlink("/70" "/71" | "/72" | "/73" | "/74" | "/75" | "/76");
    //}
}

