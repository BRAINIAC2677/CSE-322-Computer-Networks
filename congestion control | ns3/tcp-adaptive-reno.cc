/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

    NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

    TypeId
    TcpAdaptiveReno::GetTypeId()
    {
        static TypeId tid =
            TypeId("ns3::TcpAdaptiveReno")
                .SetParent<TcpWestwoodPlus>()
                .SetGroupName("Internet")
                .AddConstructor<TcpAdaptiveReno>()
                .AddAttribute(
                    "FilterType",
                    "Use this to choose no filter or Tustin's approximation filter",
                    EnumValue(TcpAdaptiveReno::TUSTIN),
                    MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                    MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
                .AddTraceSource("EstimatedBW",
                                "The estimated bandwidth",
                                MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                                "ns3::TracedValueCallback::DataRate");
        return tid;
    }

    TcpAdaptiveReno::TcpAdaptiveReno()
        : TcpWestwoodPlus(),
          m_minRtt(Time(0)),
          m_currentRtt(Time(0)),
          m_congRtt(Time(0)),
          m_prevCongRtt(Time(0)),
          m_baseWnd(0),
          m_probeWnd(0)
    {
    }

    TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno &sock)
        : TcpWestwoodPlus(sock),
          m_minRtt(Time(0)),
          m_currentRtt(Time(0)),
          m_congRtt(Time(0)),
          m_prevCongRtt(Time(0)),
          m_baseWnd(0),
          m_probeWnd(0)
    {
    }

    TcpAdaptiveReno::~TcpAdaptiveReno()
    {
    }

    void
    TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time &rtt)
    {
        if (rtt.IsZero())
        {
            return;
        }

        m_ackedSegments += packetsAcked;

        if (m_minRtt.IsZero() || rtt < m_minRtt)
        {
            m_minRtt = rtt;
        }

        m_currentRtt = rtt;
        TcpWestwoodPlus::EstimateBW(rtt, tcb);
    }

    double
    TcpAdaptiveReno::EstimateCongestionLevel()
    {
        double a = 0.85;
        // todo: think about this
        if (m_prevCongRtt < m_minRtt)
        {
            a = 0;
        }
        double currentCongRtt = a * m_prevCongRtt.GetSeconds() + (1 - a) * m_congRtt.GetSeconds();
        m_congRtt = Seconds(currentCongRtt);
        double c = std::min(1.0,
                            (m_currentRtt.GetSeconds() - m_minRtt.GetSeconds()) /
                                (m_congRtt.GetSeconds() - m_minRtt.GetSeconds()));
        return c;
    }

    uint32_t
    TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
    {
        int m = 1000;
        double alpha = 10;
        double c = EstimateCongestionLevel();
        double mss = tcb->m_segmentSize * tcb->m_segmentSize;

        double maxIncWnd = m_currentBW.Get().GetBitRate() / m * mss;
        double beta = 2 * maxIncWnd * ((1 / alpha) - ((1 / alpha + 1) / (std::exp(alpha))));
        double gamma = 1 - (2 * maxIncWnd * ((1 / alpha) - ((1 / alpha + 0.5) / (std::exp(alpha)))));
        uint32_t incWnd = (int)((maxIncWnd / std::exp(alpha * c)) + (beta * c) + gamma);
        return incWnd;
    }

    void
    TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
    {
        if (segmentsAcked > 0)
        {
            uint32_t incWnd = EstimateIncWnd(tcb);
            double mss = tcb->m_segmentSize * tcb->m_segmentSize;
            double increment = std::max(1.0, mss / tcb->m_cWnd.Get());
            m_baseWnd += static_cast<uint32_t>(increment);
            m_probeWnd = std::max(double(m_probeWnd + incWnd / (int)tcb->m_cWnd.Get()), (double)0);
            tcb->m_cWnd = m_baseWnd + m_probeWnd;
        }
    }

    uint32_t
    TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight [[maybe_unused]])
    {
        m_prevCongRtt = m_congRtt;
        double c = EstimateCongestionLevel();
        uint32_t threshold = (uint32_t)(tcb->m_cWnd.Get() / (1 + c));
        threshold = std::max(2 * tcb->m_segmentSize, threshold);
        m_baseWnd = threshold;
        m_probeWnd = 0;
        return threshold;
    }

}
