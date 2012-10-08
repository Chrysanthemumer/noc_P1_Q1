#ifndef PE_H
#define PE_H

#include "router.h"

SC_MODULE(PE_base)
{
	sc_in<bool> clock; // port for the global clock

	sc_in<packet> data_in; // from router
	sc_out<packet> data_out; // to router

	void main() // specify the functionality of PE per clock cycle
	{
		internalClock++;
		read_input();
		execute();
		queueSize2File();//the queue size shall be calculated after read before write
		write_output();
		utilization2File();
	}

	SC_CTOR(PE_base)
		: x_(-1), y_(-1), internalClock(0)
	{
		SC_METHOD(main);
		sensitive << clock.pos();
	}

	// use this function to set the coordinates of the PE
	void set_xy(int x, int y);

	virtual ~PE_base() {}

	//functions for P1_Q1

	//use this function to initialize the PE
	//Implemented:
	//(1)linkNotEmpty = 0 ;
	//(2)firedTimesPI = 0;	
	//(3)firedTimesPO = 0;
	virtual void init() = 0;
	//counters for P1_Q1
	int firedTimes[2];
	int linkNotEmpty;

	//functions for P1_Q2
	void getQueueSize(int * size);
	void queueSize2File();
	

protected:
	std::list<packet> out_queue_; // output queue
	packet packet_in_; // incoming packet from the router

	int x_, y_; // location of the PE
	int internalClock;
	virtual void read_input(); // read a packet from the the router
	virtual void execute() = 0; // abstraction of computations
	virtual void write_output(); // // send a packet to the router
	//functions for P1_Q1
	virtual void utilization2File() = 0;
}; // PE_base

// for PI and PO
class PE_IO : public PE_base
{
public:
	PE_IO(const char *name) : PE_base(name) {}
		
	void init();
protected:
	void execute();

	void fire_PI();
	void fire_PO();
	void utilization2File();

}; // class PE_IO

// for P1
class PE_inc : public PE_base
{
public:
	PE_inc(const char *name) : PE_base(name) {}
	void init();
	
protected:

	void execute();

	void fire();
	void utilization2File();

};

#endif // PE_H
