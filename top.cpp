#include <time.h>
#include <systemc.h>

#include "router.h"
#include "pe.h"

SC_MODULE(top)
{
public:
	enum {N = 2};

	router *routers[N];
	PE_base *pes[N];

	sc_signal<packet> router_to_pe[N], pe_to_router[N];
	sc_signal<packet> router_to_router_east[N-1], router_to_router_west[N-1];
	sc_signal<packet> terminal_loop_north[N], terminal_loop_south[N];
	sc_signal<packet> terminal_loop_east, terminal_loop_west;
	sc_signal<bool> clock;

	SC_CTOR(top)
	{
		create_pes();
		create_network();
	}

protected:
	void create_pes()
	{
		pes[0] = new PE_IO("PI/PO");
		pes[0]->clock(clock);
		pes[0]->set_xy(0, 0);

		pes[1] = new PE_inc("P1");
		pes[1]->clock(clock);
		pes[1]->set_xy(1, 0);
	}

	void create_network()
	{
		for (int i = 0; i < N; ++i)
		{
			char name[100];
			sprintf(name, "router%d", i);

			// create router
			routers[i] = new router(name);
			routers[i]->set_xy(i,0);
			routers[i]->clock(clock);

			// loop unused ports
			routers[i]->port_in[router::NORTH](
				terminal_loop_north[i]);
			routers[i]->port_out[router::NORTH](
				terminal_loop_north[i]);
			routers[i]->port_in[router::SOUTH](
				terminal_loop_south[i]);
			routers[i]->port_out[router::SOUTH](
				terminal_loop_south[i]);

			// connect router to west routers
			if (i != 0)
			{
				routers[i]->port_out[router::WEST](
					router_to_router_west[i-1]);
				routers[i]->port_in[router::WEST](
					router_to_router_east[i-1]);
			}
			else // or make a loop
			{
				routers[i]->port_out[router::WEST](
					terminal_loop_west);
				routers[i]->port_in[router::WEST](
					terminal_loop_west);
			}

			if (i != N-1) // connect router to east routers
			{
				routers[i]->port_out[router::EAST](
					router_to_router_east[i]);
				routers[i]->port_in[router::EAST](
					router_to_router_west[i]);
			}
			else // or make a loop
			{
				routers[i]->port_out[router::EAST](
					terminal_loop_east);
				routers[i]->port_in[router::EAST](
					terminal_loop_east);
			}

			// connect router to PE
			routers[i]->port_out[router::PE](router_to_pe[i]);
			routers[i]->port_in[router::PE](pe_to_router[i]);
			pes[i]->data_in(router_to_pe[i]);
			pes[i]->data_out(pe_to_router[i]);
		}
	}

}; // top

int sc_main(int argc , char *argv[])
{
	srand(0);

	top top_module("top");

	printf("cycle  0 ================================\n");
	sc_start(0, SC_NS);

	for(int i = 1; i < 20; i++){
		
		printf("cycle %2d ================================\n", i);

		top_module.clock.write(1);
		sc_start(10, SC_NS);
		top_module.clock.write(0);
		sc_start(10, SC_NS);
	}

	return 0;
}
