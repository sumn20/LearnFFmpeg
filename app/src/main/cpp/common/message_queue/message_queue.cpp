#include "./message_queue.h"
#include "./handler.h"

#define LOG_TAG "MessageQueue"

/******************* MessageQueue class *******************/
MessageQueue::MessageQueue() {
	init();
}

MessageQueue::MessageQueue(const char* queueNameParam) {
	init();
	queueName = queueNameParam;
}

void MessageQueue::init() {
	int initLockCode = pthread_mutex_init(&mLock, nullptr);
	int initConditionCode = pthread_cond_init(&mCondition, nullptr);
	mNbPackets = 0;
	mFirst = nullptr;
	mLast = nullptr;
	mAbortRequest = false;
}

MessageQueue::~MessageQueue() {
	LOGCATI("%s ~PacketQueue ....", queueName);
	flush();
	pthread_mutex_destroy(&mLock);
	pthread_cond_destroy(&mCondition);
}

int MessageQueue::size() {
	pthread_mutex_lock(&mLock);
	int size = mNbPackets;
	pthread_mutex_unlock(&mLock);
	return size;
}

void MessageQueue::flush() {
	LOGCATI("\n %s flush .... and this time the queue size is %d \n", queueName, size());
	MessageNode *curNode, *nextNode;
	Message *msg;
	pthread_mutex_lock(&mLock);
	for (curNode = mFirst; curNode != nullptr; curNode = nextNode) {
		nextNode = curNode->next;
		msg = curNode->msg;
		if (nullptr != msg) {
			delete msg;
		}
		delete curNode;
		curNode = nullptr;
	}
	mLast = nullptr;
	mFirst = nullptr;
	mNbPackets = 0;
	pthread_mutex_unlock(&mLock);
}

int MessageQueue::enqueueMessage(Message* msg) {
	if (mAbortRequest) {
		delete msg;
		return -1;
	}
	MessageNode *node = new MessageNode();
	if (!node)
		return -1;
	node->msg = msg;
	node->next = nullptr;
	int getLockCode = pthread_mutex_lock(&mLock);
	if (mLast == nullptr) {
		mFirst = node;
	} else {
		mLast->next = node;
	}
	mLast = node;
	mNbPackets++;
	pthread_cond_signal(&mCondition);
	pthread_mutex_unlock(&mLock);
	return 0;
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int MessageQueue::dequeueMessage(Message **msg, bool block) {
	MessageNode *node;
	int ret;
	int getLockCode = pthread_mutex_lock(&mLock);
	for (;;) {
		if (mAbortRequest) {
			ret = -1;
			break;
		}
		node = mFirst;
		if (node) {
			mFirst = node->next;
			if (!mFirst)
				mLast = nullptr;
			mNbPackets--;
			*msg = node->msg;
			delete node;
			node = nullptr;
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			pthread_cond_wait(&mCondition, &mLock);
		}
	}
	pthread_mutex_unlock(&mLock);
	return ret;
}

void MessageQueue::abort() {
	pthread_mutex_lock(&mLock);
	mAbortRequest = true;
	pthread_cond_signal(&mCondition);
	pthread_mutex_unlock(&mLock);
}


/******************* Message class *******************/
Message::Message() {
	handler = nullptr;
}

Message::Message(int what){
	handler = nullptr;
	this->what = what;
}
Message::Message(int what, int arg1, int arg2) {
	handler = nullptr;
	this->what = what;
	this->arg1 = arg1;
	this->arg2 = arg2;
}
Message::Message(int what, void* obj) {
	handler = nullptr;
	this->what = what;
	this->obj = obj;
}
Message::Message(int what, int arg1, int arg2, void* obj) {
	handler = nullptr;
	this->what = what;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->obj = obj;
}
Message::~Message() {
}

int Message::execute(){
	if (MESSAGE_QUEUE_LOOP_QUIT_FLAG == what) {
		return MESSAGE_QUEUE_LOOP_QUIT_FLAG;
	} else if (handler) {
		handler->handleMessage(this);
		return 1;
	}
	return 0;
};
