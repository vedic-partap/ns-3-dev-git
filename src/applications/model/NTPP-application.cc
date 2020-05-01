/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 Vedic Partap
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Vedic Partap <vedicpartap1999@gmail.com>
 */

#include "NTPP-application.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
// #include "ns3/random-variable.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/double.h"
#include "ns3/pointer.h"

NS_LOG_COMPONENT_DEFINE ("NTPPApplication");

using namespace std;

namespace ns3 {
	
	NS_OBJECT_ENSURE_REGISTERED (NTPPApplication);
	
	TypeId
	NTPPApplication::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::NTPPApplication")
		.SetParent<Application> ()
		.AddConstructor<NTPPApplication> ()
		.AddAttribute ("BurstIntensity", "The data rate of each burst.",
					   DataRateValue (DataRate ("1Mbps")),
					   MakeDataRateAccessor (&NTPPApplication::m_cbrRate),
					   MakeDataRateChecker ())
		.AddAttribute ("PacketSize", "The size of packets sent in on state",
					   UintegerValue (1470),
					   MakeUintegerAccessor (&NTPPApplication::m_pktSize),
					   MakeUintegerChecker<uint32_t> (1))
		.AddAttribute ("MeanBurstArrivals", "Mean Active Sources",
					   StringValue ("ns3::ConstantRandomVariable[Constant=20]"),
 					   MakePointerAccessor (&NTPPApplication::m_burstArrivals),
					   MakePointerChecker <RandomVariableStream> ())
		.AddAttribute ("MeanBurstTimeLength", "Pareto distributed burst durations",
					   StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"),
					   MakePointerAccessor (&NTPPApplication::m_burstLength),
					   MakePointerChecker <RandomVariableStream> ())
		.AddAttribute ("H", "Hurst parameter",
					   DoubleValue (0.7),
					   MakeDoubleAccessor (&NTPPApplication::m_h),
					   MakeDoubleChecker<double> ())
		.AddAttribute ("Remote", "The address of the destination",
					   AddressValue (),
					   MakeAddressAccessor (&NTPPApplication::m_peer),
					   MakeAddressChecker ())
		.AddAttribute ("Protocol", "The type of protocol to use.",
					   TypeIdValue (UdpSocketFactory::GetTypeId ()),
					   MakeTypeIdAccessor (&NTPPApplication::m_protocolTid),
					   MakeTypeIdChecker ())
		// .AddTraceSource ("Tx", "A new packet is created and is sent",
		// 				 MakeTraceSourceAccessor (&NTPPApplication::m_txTrace))
		;
		return tid;
	}
	
	NTPPApplication::NTPPApplication ()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_socket = 0;
		m_connected = false;
		m_lastStartTime = Seconds (0);
		m_totalBytes = 0;
		m_activebursts = 0;
		m_offPeriod = true;
	}
	
	NTPPApplication::~NTPPApplication()
	{
		NS_LOG_FUNCTION_NOARGS ();
	}
	
	uint32_t
	NTPPApplication::GetTotalBytes() const
	{
		return m_totalBytes;
	}
	
	void
	NTPPApplication::DoDispose (void)
	{
		NS_LOG_FUNCTION_NOARGS ();
		
		m_socket = 0;
		// chain up
		Application::DoDispose ();
	}
	
	// Application Methods
	void
	NTPPApplication::StartApplication() // Called at time specified by Start
	{
		NS_LOG_FUNCTION_NOARGS ();
		
		// Create the socket if not already
		if (!m_socket)
		{
			m_socket = Socket::CreateSocket (GetNode(), m_protocolTid);
			m_socket->Bind ();
			m_socket->Connect (m_peer);
		}
		// Insure no pending event
		CancelEvents ();
		ScheduleStartEvent();
	}
	
	void
	NTPPApplication::NTPP() // Poisson Pareto Burst 
	{
		NS_LOG_FUNCTION_NOARGS ();
		
		double inter_burst_intervals;
		inter_burst_intervals = 1/m_burstArrivals->GetValue();

		// ExponentialRandomVariable exp(inter_burst_intervals);
		Ptr<ExponentialRandomVariable> exp = CreateObject <ExponentialRandomVariable> ();
		exp->SetAttribute ("Mean", DoubleValue (inter_burst_intervals));

		Time t_poisson_arrival = Seconds (exp->GetValue());
		m_PoissonArrival = Simulator::Schedule(t_poisson_arrival,&NTPPApplication::PoissonArrival, this);
		
		// Pareto
		m_shape = 3 - 2 * m_h;
		m_timeSlot = Seconds((double) (m_shape - 1) * m_burstLength->GetValue() / m_shape);
		
		// ParetoRandomVariable pareto(m_burstLength->GetValue(), m_shape);
		Ptr<ParetoRandomVariable> pareto = CreateObject<ParetoRandomVariable> ();
		pareto->SetAttribute ("Scale", DoubleValue (m_burstLength->GetValue()));
		pareto->SetAttribute ("Shape", DoubleValue (m_shape));
		
		m_ParetoDeparture = Simulator::Schedule(t_poisson_arrival + Seconds (pareto->GetValue()),&NTPPApplication::ParetoDeparture, this);
		
		m_NTPP = Simulator::Schedule(t_poisson_arrival,&NTPPApplication::NTPP, this);
	}
	
	void NTPPApplication::PoissonArrival()
	{
		NS_LOG_FUNCTION_NOARGS ();
		++m_activebursts;
		if (m_offPeriod) ScheduleNextTx();
	}
	
	void
	NTPPApplication::ParetoDeparture()
	{
		NS_LOG_FUNCTION_NOARGS ();
		--m_activebursts;
	}
	
	void
	NTPPApplication::StopApplication() // Called at time specified by Stop
	{
		NS_LOG_FUNCTION_NOARGS ();
		
		CancelEvents ();
		if(m_socket != 0) m_socket->Close ();
		else NS_LOG_WARN("NTPPApplication found null socket to close in StopApplication");
	}
	
	void
	NTPPApplication::CancelEvents ()
	{
		NS_LOG_FUNCTION_NOARGS ();
		Simulator::Cancel(m_sendEvent);
		Simulator::Cancel(m_startStopEvent);
		
		Simulator::Cancel(m_NTPP);
		Simulator::Cancel(m_PoissonArrival);
		Simulator::Cancel(m_ParetoDeparture);
	}
	
	// Event handlers 
	void
	NTPPApplication::StartSending()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_lastStartTime = Simulator::Now();
		ScheduleNextTx();					// Schedule the send packet event
		ScheduleStopEvent();
	}

	void
	NTPPApplication::StopSending()
	{
		NS_LOG_FUNCTION_NOARGS ();
		CancelEvents();
		
		ScheduleStartEvent();
	}
	
	void
	NTPPApplication::ScheduleNextTx()
	{
		NS_LOG_FUNCTION_NOARGS ();
		uint32_t bits = (m_pktSize + 30) * 8;
		Time nextTime(Seconds (bits / 
							   static_cast<double>(m_cbrRate.GetBitRate())));
		
		if (m_activebursts != 0)
		{
			m_offPeriod = false;
			double data_rate = (double) nextTime.GetSeconds() / m_activebursts;
			m_sendEvent = Simulator::Schedule(Seconds(data_rate),&NTPPApplication::SendPacket, this);
		}
		else
		{
			m_offPeriod = true;
		}

	}
	
	void
	NTPPApplication::ScheduleStartEvent()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_NTPP = Simulator::Schedule(Seconds(0.0), &NTPPApplication::NTPP, this);
		m_startStopEvent = Simulator::Schedule(Seconds(0.0), &NTPPApplication::StartSending, this);
	}
	
	void
	NTPPApplication::ScheduleStopEvent()
	{
		NS_LOG_FUNCTION_NOARGS ();
	}
	
	void
	NTPPApplication::SendPacket()
	{
		NS_LOG_FUNCTION_NOARGS ();		
		Ptr<Packet> packet = Create<Packet> (m_pktSize);
		m_txTrace (packet);
		m_socket->Send (packet);
		m_totalBytes += packet->GetSize();
		m_lastStartTime = Simulator::Now();
		ScheduleNextTx();
	}
	
	void
	NTPPApplication::ConnectionSucceeded(Ptr<Socket>)
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_connected = true;
		ScheduleStartEvent();
	}
	
	void
	NTPPApplication::ConnectionFailed(Ptr<Socket>)
	{
		NS_LOG_FUNCTION_NOARGS ();
		cout << "NTPPApplication, Connection Failed" << endl;
	}
} // Namespace ns3