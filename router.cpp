#include "router.h"

std::ostream &operator<<(std::ostream &o, const packet &p)
{
	char buf[100];
	sprintf(buf, "(%d,%d)->(%d,%d)",
		p.src_x, p.src_y, p.dest_x, p.dest_y);
	return o << buf << ", " << p.token;
}

void sc_trace(sc_trace_file *tf, const packet &p, const std::string &name)
{
	sc_trace(tf, p.src_x, name+".src.x");
	sc_trace(tf, p.src_y, name+".src.y");
	sc_trace(tf, p.dest_x, name+".dest.x");
	sc_trace(tf, p.dest_y, name+".dest.y");
	sc_trace(tf, p.token, name+".token");
}

void router::main()
{
	internalClock++;
	assert((x_ != -1) && (y_ != -1)); // to identify PE

	for (int iport = 0; iport < PORTS; ++iport)
		read_packet(iport);

	queueSize2File();

	for (int iport = 0; iport < PORTS; ++iport)
		write_packet(iport);

	utilization2File();
}

void router::set_xy(int x, int y)
{
	assert((x_ == -1) && (y_ == -1)); // set once only
	assert((x != -1) && (y != -1)); // must use a legal location

	x_ = x;
	y_ = y;
}

void router::read_packet(int iport)
{
	assert(iport < PORTS);

	packet p = port_in[iport].read();

	if ((p.src_x == -1) && (p.src_y == -1))
		return; // empty packet

	route_packet_xy(p);
	//Every time the link is read not empty, the corresponding link counter increases by 1.
	linkNotEmpty[iport]++; 
}

void router::write_packet(int iport)
{
	assert(iport < PORTS);

	if (out_queue_[iport].empty())
	{
		port_out[iport].write(packet()); // write an empty packet
	}
	else
	{
		port_out[iport].write(out_queue_[iport].front());
		out_queue_[iport].pop_front();
	}
}

void router::route_packet_xy(packet p)
{
	if ((p.dest_x == -1) || (p.dest_y == -1))
	{
		printf("router (%d,%d): drop packet with invalid destination"
			" (%d,%d)->(%d,%d)\n",
			p.src_x, p.src_y, p.dest_x, p.dest_y);
		return;
	}

	// ignore dest_y for now
	if (p.dest_x == x_) // to PE
	{
		out_queue_[PE].push_back(p);
	}
	else if (p.dest_x < x_) // left to WEST
	{
		out_queue_[WEST].push_back(p);
	}
	else // (p.dest_x > x_) right to EAST
	{
		out_queue_[EAST].push_back(p);
	}
}

void router::init()
{
	linkNotEmpty[0] = 0;
	linkNotEmpty[1] = 0;
	linkNotEmpty[SOUTH] = 0;
	linkNotEmpty[EAST] = 0;
	linkNotEmpty[WEST] = 0;
	//init queue size file
	char filename[5][256];
	sprintf(filename[0], "router_%d_%d_OutQueue_PE.txt", x_, y_);
	sprintf(filename[1], "router_%d_%d_OutQueue_NORTH.txt", x_, y_);
	sprintf(filename[2], "router_%d_%d_OutQueue_SOUTH.txt", x_, y_);
	sprintf(filename[3], "router_%d_%d_OutQueue_EAST.txt", x_, y_);
	sprintf(filename[4], "router_%d_%d_OutQueue_WEST.txt", x_, y_);
	for(int i=0;i<5;i++)
	{
	ofstream initmyfile;
	initmyfile.open(filename[i], ios::trunc);
	initmyfile<<endl;
	initmyfile.close();
	}
	//init util file
	char filenameU[5][256];
	sprintf(filenameU[0], "router_%d_%d_UtilLinkIn_PE.txt", x_, y_);
	sprintf(filenameU[1], "router_%d_%d_UtilLinkIn_NORTH.txt", x_, y_);
	sprintf(filenameU[2], "router_%d_%d_UtilLinkIn_SOUTH.txt", x_, y_);
	sprintf(filenameU[3], "router_%d_%d_UtilLinkIn_EAST.txt", x_, y_);
	sprintf(filenameU[4], "router_%d_%d_UtilLinkIn_WEST.txt", x_, y_);
	for (int i=0; i<5;i++)
	{
		ofstream myfile;
		myfile.open(filenameU[i], ios::trunc);
		myfile<<endl;
		myfile.close();
	}

}
void router::getQueueSize(int * size)
{
		for(int i = 0;i<5;i++)
	{
		size[i] = (int)out_queue_[i].size();
	}
}
void router::queueSize2File()
{
	int size[5]={0};
	getQueueSize(size);

	char filename[5][256];
	sprintf(filename[0], "router_%d_%d_OutQueue_PE.txt", x_, y_);
	sprintf(filename[1], "router_%d_%d_OutQueue_NORTH.txt", x_, y_);
	sprintf(filename[2], "router_%d_%d_OutQueue_SOUTH.txt", x_, y_);
	sprintf(filename[3], "router_%d_%d_OutQueue_EAST.txt", x_, y_);
	sprintf(filename[4], "router_%d_%d_OutQueue_WEST.txt", x_, y_);

	
	for(int i=0;i<5;i++)
	{
	ofstream myfile(filename[i], ios::app);
	myfile<<size[i]<<endl;
	myfile.close();
	}
}
void router::utilization2File()
{
	float util[5];
	util[0] = (float)linkNotEmpty[0]/(float)internalClock;
	util[1] = (float)linkNotEmpty[1]/(float)internalClock;
	util[2] = (float)linkNotEmpty[2]/(float)internalClock;
	util[3] = (float)linkNotEmpty[3]/(float)internalClock;
	util[4] = (float)linkNotEmpty[4]/(float)internalClock;

	char filename[5][256];
	sprintf(filename[0], "router_%d_%d_UtilLinkIn_PE.txt", x_, y_);
	sprintf(filename[1], "router_%d_%d_UtilLinkIn_NORTH.txt", x_, y_);
	sprintf(filename[2], "router_%d_%d_UtilLinkIn_SOUTH.txt", x_, y_);
	sprintf(filename[3], "router_%d_%d_UtilLinkIn_EAST.txt", x_, y_);
	sprintf(filename[4], "router_%d_%d_UtilLinkIn_WEST.txt", x_, y_);

	for (int i=0; i<5;i++)
	{
		ofstream myfile(filename[i], ios::app);
		myfile<<util[i]<<endl;
		myfile.close();
	}
}