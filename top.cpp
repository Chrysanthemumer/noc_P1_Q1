#include <time.h>
#include <systemc.h>

#include "router.h"
#include "pe.h"

struct utilizationData
{
	enum {N =2};
	//for router id links
	int rtr2rtrW[N-1];
	int rtr2rtrE[N-1];
	float P_rtr2rtrW[N-1];
	float P_rtr2rtrE[N-1];
	//for pe id links
	int pe2rtr[N];
	int rtr2pe[N];
	float P_pe2rtr[N];
	float P_rtr2pe[N];
	//for pe
	int PEinc;
	float P_PEinc;
	int PEIO[2];
	float P_PEIO[2];

	utilizationData()
	{
		for (int i=0; i<N-1; i++)
		{
			rtr2rtrW[i]=0;
			rtr2rtrE[i]=0;
			P_rtr2rtrW[i]=0;
			P_rtr2rtrE[i]=0;
		}
		for (int i=0; i<N; i++)
		{
			pe2rtr[i]=0;
			P_pe2rtr[i]=0;
			rtr2pe[i]=0;
			P_rtr2pe[i]=0;
		}
		PEinc = 0;
		PEIO[0]=0;
		PEIO[1]=0;
		P_PEinc = 0;
		P_PEIO[0]=0;
		P_PEIO[1]=0;

	}	
};

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

	//top level public functions for project 1 questions
	void getUtilizationData(float totalClock, utilizationData * returnStruct)
	{
		//get values
		returnStruct->pe2rtr[0] = routers[0]->linkNotEmpty[0];
		returnStruct->pe2rtr[1] = routers[1]->linkNotEmpty[0];

		returnStruct->rtr2pe[0] = pes[0]->linkNotEmpty;
		returnStruct->rtr2pe[1] = pes[1]->linkNotEmpty;

		returnStruct->PEIO[0] = pes[0]->firedTimes[0];
		returnStruct->PEIO[1] = pes[0]->firedTimes[1];
		returnStruct->PEinc = pes[1]->firedTimes[0];
		//calculate rates
		returnStruct->P_pe2rtr[0] = (float)returnStruct->pe2rtr[0]/totalClock;
		returnStruct->P_pe2rtr[1] = (float)returnStruct->rtr2pe[0]/totalClock;

		returnStruct->P_rtr2pe[0] = (float)returnStruct->rtr2pe[0]/totalClock;
		returnStruct->P_rtr2pe[1] = (float)returnStruct->rtr2pe[1]/totalClock;

		returnStruct->P_PEIO[0] = (float)returnStruct->PEIO[0]/totalClock;
		returnStruct->P_PEIO[1] = (float)returnStruct->PEIO[1]/totalClock;
		returnStruct->P_PEinc = (float)returnStruct->PEinc/totalClock;

		return;
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

		//init pe counters for P1_Q1
		pes[0]->init();
		pes[1]->init();

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
			
			
			//init router counters for P1_Q1
			routers[0]->init();
			routers[1]->init();

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

	//Data for Project 1
	utilizationData * P1_Q1 = new utilizationData();

	printf("cycle  0 ================================\n");
	sc_start(0, SC_NS);

	for(int i = 1; i < 20; i++){
		
		printf("cycle %2d ================================\n", i);

		top_module.clock.write(1);
		sc_start(10, SC_NS);
		top_module.clock.write(0);
		sc_start(10, SC_NS);
	}

	//Methods for Project 1
	//Question 1
	top_module.getUtilizationData(20, P1_Q1);
	printf("Utilization Rate for P1: \%%4.2f", P1_Q1->P_PEinc);

	return 0;
}
