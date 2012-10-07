#include "pe.h"

void PE_base::set_xy(int x, int y)
{
	assert((x_ == -1) && (y_ == -1)); // set once only
	assert((x != -1) && (y != -1)); // must use a legal location

	x_ = x;
	y_ = y;
}

void PE_base::read_input()
{
	packet_in_ = data_in.read();
}

void PE_base::write_output()
{
	if (out_queue_.empty())
	{
		data_out.write(packet());
	}
	else
	{
		data_out.write(out_queue_.front());
		out_queue_.pop_front();
	}
}

void PE_IO::execute()
{
	// decide if we are going to fire PI
	int r = rand()%100;
	if (r < 80)
		fire_PI();

	// fire PO if the incoming packet is valid
	if ((packet_in_.src_x != -1)
		&& (packet_in_.src_y != -1))
		fire_PO();
}

void PE_IO::fire_PI()
{
	int P1_x = 1, P1_y = 0;

	packet p(x_, y_, P1_x, P1_y, rand()%100);

	printf("PI: send %d to (%d,%d)\n",
		p.token, p.dest_x, p.dest_y);

	out_queue_.push_back(p);
}

void PE_IO::fire_PO()
{
	assert((packet_in_.src_x != -1)
		&& (packet_in_.src_y != -1));

	printf("PO: receive %d from (%d,%d)\n",
		packet_in_.token, packet_in_.src_x,	packet_in_.src_y);
}

void PE_IO::init()
{
	linkNotEmpty = 0;
	firedTimesPI = 0;
	firedTimesPO = 0;
}

void PE_inc::execute()
{
	// fire the actor if the incoming packet is valid
	if ((packet_in_.src_x != -1)
		&& (packet_in_.src_y != -1))
		fire();
}

void PE_inc::fire()
{
	assert((packet_in_.src_x != -1)
		&& (packet_in_.src_y != -1));

	int PO_x = 0, PO_y = 0;
	packet p(x_, y_, PO_x, PO_y, packet_in_.token+1);

	printf("inc(%d,%d): receive %d from (%d,%d), send %d to (%d,%d)\n",
		x_, y_,
		packet_in_.token, packet_in_.src_x,	packet_in_.src_y,
		p.token, p.dest_x, p.dest_y);

	out_queue_.push_back(p);
}
void PE_inc::init()
{
	linkNotEmpty = 0;
	firedTimes = 0;
}