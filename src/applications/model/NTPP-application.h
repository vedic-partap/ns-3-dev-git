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

#ifndef __NTPP_application_h__
#define __NTPP_application_h__

#include "ns3/core-module.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"
#include "ns3/data-rate.h"

namespace ns3 {
	
	class Socket;
	
	/**
	 * \ingroup applications 
	 * \defgroup NTPP NTPP-application
	 *
	 * The traffic generator follows a Network Transmission Point Process (NTPP).
	 * This a process based on the overlapping of multiple bursts with
	 * heavy-tailed distributed lengths.
	 * Events in this process represent points of time at which one of an
	 * infinite population of users begins or stops transmitting a traffic burst.
	 * The NTPP is closely related to the M/G/Infinity queue model.
	 */
	
	/**
	 * \ingroup NTPP
	 *
	 * \brief Generate traffic to a single destination according to a 
	 *        Point Process Point Process (NTPP)
	 */
	
	class NTPPApplication : public Application 
	{
	public:
		static TypeId GetTypeId (void);
		
		NTPPApplication ();
		
		virtual ~NTPPApplication();
		
		/**
		 * \brief Return total bytes sent by this object.
		 */
		uint32_t      GetTotalBytes() const;
		
	protected:
		virtual void DoDispose ();
		
	private:
		// Inherited from Application base class.
		virtual void StartApplication ();				// Called at time specified by Start
		virtual void StopApplication ();				// Called at time specified by Stop
		
		// Helpers
		void CancelEvents ();
		
		// Event handlers
		void StartSending();
		void StopSending();
		void SendPacket();
		
		Ptr<Socket>     m_socket;						// Associated socket
		TypeId          m_protocolTid;					// protocol type id
		Address         m_peer;							// Peer address
		bool            m_connected;					// True if connected
		
		uint32_t        m_totalBytes;					// Total bytes sent so far
				
		Time            m_lastStartTime;				// Time last packet sent		
		EventId         m_startStopEvent;				// Event id for next start or stop event
		EventId         m_sendEvent;					// Event id of pending "send packet" event
		EventId			m_getUtilization;				// Event id to get the utilization factor
		EventId			m_PoissonArrival;				// Event id for next burst arrival
		EventId			m_NTPP;							// Event id for the length of a pending burst arrival
		EventId			m_ParetoDeparture;				// Event id of burst departure
		
		uint32_t		m_pktSize;						// Size of packets
		
		TracedCallback< Ptr<const Packet> > m_txTrace;	// Trace callback for each sent packet
		
		Ptr<ConstantRandomVariable>	m_burstArrivals;				// Mean rate of burst arrivals
		Ptr<ConstantRandomVariable>	m_burstLength;					// Mean burst time length
		DataRate        m_cbrRate;						// Burst intensity (constant bit-rate)

		double			m_h;							// Hurst parameter	(distribution)
		double			m_shape;						// Shape			(distribution)
		Time			m_timeSlot;						// The time slot
		int				m_activebursts;					// Number of active bursts at time t
		bool			m_offPeriod;

		
	private:
		void ScheduleStartEvent();
		void ScheduleStopEvent();
		void ConnectionSucceeded(Ptr<Socket>);
		void ConnectionFailed(Ptr<Socket>);
		
		/**
		 * \ Functions that allows to keep track of the current number of active bursts at time t, nt,
		 * taking into account that their arrival process follows a Poisson process and that their
		 * length is determined by a distribution.
		 */
		void NTPP();
		void PoissonArrival();
		void ParetoDeparture();
		
		/**
		 * \ Function thet generates the packets departure at a constant bit-rate nt x r.
		 */
		void ScheduleNextTx();
	};
	
} // namespace ns3
#endif