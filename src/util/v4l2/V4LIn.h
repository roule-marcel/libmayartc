/*
 * V4LIn.h
 *
 *  Created on: 5 août 2014
 *      Author: jfellus
 */

#ifndef V4LIN_H_
#define V4LIN_H_

#include <pthread.h>
#include <semaphore.h>

struct buffer;


class V4LIn {
public:
	unsigned char* data;
	int w, h;
	bool bStarted;

public:
	V4LIn(const char* devname);
	virtual ~V4LIn();
	void start();
	void stop();
	void readFrame();
private:
	int  fd;
	struct buffer *buffers;
	unsigned int n_buffers;
	pthread_t th_async;
	unsigned char* tmpdata;
	pthread_mutex_t mut;
	sem_t sem;

public:
	void _readFrame_async();
protected:
	void open_device(const char* dev_name);
};




#endif /* V4LIN_H_ */
