/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*   Copyright (c) 2016, 2018, University of Padova, Dep. of Information Engineering, SIGNET lab.
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*
* Modified by: Michele Polese <michele.polese@gmail.com>
*                 Dual Connectivity and Handover functionalities
*
* Modified by: Tommaso Zugno <tommasozugno@gmail.com>
*								 Integration of Carrier Aggregation
*
*	Modified by: Junseok Kim <jskim14@mwnl.snu.ac.kr> Hybrid beamforming
*/


#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/log.h>
#include "mmwave-phy.h"
#include "mmwave-phy-sap.h"
#include "mmwave-mac-pdu-tag.h"
#include "mmwave-mac-pdu-header.h"
#include <sstream>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhy");

namespace mmwave {

NS_OBJECT_ENSURE_REGISTERED ( MmWavePhy);

/*   SAP   */
class MmWaveMemberPhySapProvider : public MmWavePhySapProvider
{
public:
  MmWaveMemberPhySapProvider (MmWavePhy* phy);

  virtual void SendMacPdu (Ptr<Packet> p );

  virtual void SendControlMessage (Ptr<MmWaveControlMessage> msg);

  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti);

  virtual void SetDlSfAllocInfo (SfAllocInfo sfAllocInfo);

  virtual void SetUlSfAllocInfo (SfAllocInfo sfAllocInfo);

private:
  MmWavePhy* m_phy;
};

MmWaveMemberPhySapProvider::MmWaveMemberPhySapProvider (MmWavePhy* phy)
  : m_phy (phy)
{
//	 Nothing more to do
}

void
MmWaveMemberPhySapProvider::SendMacPdu (Ptr<Packet> p)
{
  m_phy->SetMacPdu (p);
}

void
MmWaveMemberPhySapProvider::SendControlMessage (Ptr<MmWaveControlMessage> msg)
{
  m_phy->SetControlMessage (msg);      //May need to change
}

void
MmWaveMemberPhySapProvider::SendRachPreamble (uint8_t PreambleId, uint8_t Rnti)
{
  m_phy->SendRachPreamble (PreambleId, Rnti);
}

void
MmWaveMemberPhySapProvider::SetDlSfAllocInfo (SfAllocInfo sfAllocInfo)
{
  m_phy->SetDlSfAllocInfo (sfAllocInfo);
}

void
MmWaveMemberPhySapProvider::SetUlSfAllocInfo (SfAllocInfo sfAllocInfo)
{
  m_phy->SetUlSfAllocInfo (sfAllocInfo);
}

/* ======= */

TypeId
MmWavePhy::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWavePhy")
    .SetParent<Object> ()
  ;

  return tid;
}

MmWavePhy::MmWavePhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

MmWavePhy::MmWavePhy (Ptr<MmWaveSpectrumPhy> dlChannelPhy, Ptr<MmWaveSpectrumPhy> ulChannelPhy)
  : m_downlinkSpectrumPhy (dlChannelPhy),
    m_uplinkSpectrumPhy (ulChannelPhy),
    m_cellId (0),
    m_frameNum (0),
    m_sfNum (0),
    m_slotNum (0),
    m_sfAllocInfoUpdated (false),
    m_componentCarrierId (0)
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider = new MmWaveMemberPhySapProvider (this);
}

MmWavePhy::MmWavePhy (std::vector<Ptr<MmWaveSpectrumPhy> > dlChannelPhy, std::vector<Ptr<MmWaveSpectrumPhy> > ulChannelPhy)
  : //m_downlinkSpectrumPhy (dlChannelPhy.at(0)),
    //m_uplinkSpectrumPhy (ulChannelPhy.at(0)),
    m_downlinkSpectrumPhyList (dlChannelPhy),
    m_uplinkSpectrumPhyList (ulChannelPhy),
    m_cellId (0),
    m_frameNum (0),
    m_sfNum (0),
    m_slotNum (0),
    m_sfAllocInfoUpdated (false),
    m_componentCarrierId (0)
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider = new MmWaveMemberPhySapProvider (this);
}


MmWavePhy::~MmWavePhy ()
{

}

void
MmWavePhy::DoInitialize ()
{
}

void
MmWavePhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_controlMessageQueue.clear ();

  Object::DoDispose ();
}

void
MmWavePhy::SetDevice (Ptr<NetDevice> d)
{
  m_netDevice = d;
}

Ptr<NetDevice>
MmWavePhy::GetDevice ()
{
  return m_netDevice;
}

void
MmWavePhy::SetChannel (Ptr<SpectrumChannel> c)
{

}

double
MmWavePhy::GetTti (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyMacConfig->GetTti ();
}

void
MmWavePhy::DoSetCellId (uint16_t cellId)
{

  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
  if (m_downlinkSpectrumPhy != 0)
    {
      m_downlinkSpectrumPhy->SetCellId (cellId);
      m_uplinkSpectrumPhy->SetCellId (cellId);
    }
  else
    {
      uint8_t numEnbLayers = m_phyMacConfig->GetNumEnbLayers ();
      for (uint8_t layerInd = 0; layerInd < numEnbLayers; layerInd++)
        {
          m_downlinkSpectrumPhyList.at (layerInd)->SetCellId (cellId);
          m_uplinkSpectrumPhyList.at (layerInd)->SetCellId (cellId);
        }
    }
}


void
MmWavePhy::SetNoiseFigure (double nf)
{
  m_noiseFigure = nf;
}

double
MmWavePhy::GetNoiseFigure (void) const
{
  return m_noiseFigure;
}

void
MmWavePhy::SendRachPreamble (uint32_t PreambleId, uint32_t Rnti)
{
  m_raPreambleId = PreambleId;
  Ptr<MmWaveRachPreambleMessage> msg = Create<MmWaveRachPreambleMessage> ();
  msg->SetRapId (PreambleId);
  SetControlMessage (msg);
}

void
MmWavePhy::SetMacPdu (Ptr<Packet> p)
{
  MmWaveMacPduTag tag;
  if (p->PeekPacketTag (tag))
    {
      //uint8_t numLayers = m_phyMacConfig->GetNumEnbLayers();
      NS_ASSERT ((tag.GetSfn ().m_sfNum >= 0) && (tag.GetSfn ().m_sfNum < m_phyMacConfig->GetSymbolsPerSubframe ()));
      uint8_t sfNum = tag.GetSfn ().m_sfNum;
      uint8_t slotNum = tag.GetSfn ().m_slotNum;
      uint8_t numAllocLayers = tag.GetNumAllocLayers();
      uint8_t layerInd = tag.GetLayerInd();
      NS_LOG_UNCOND ("Sf num: " << (int)sfNum << ", slot num: " << (int)slotNum);
      NS_LOG_UNCOND ("No. of allocated layers: " << (int)numAllocLayers << ", Layer number: " << (int)layerInd);
      if (numAllocLayers == 1)
        {
          std::map<uint32_t, Ptr<PacketBurst>>::iterator it = m_packetBurstMap.find (tag.GetSfn ().Encode ());
          if (it == m_packetBurstMap.end ())
            {
              it = m_packetBurstMap.insert (std::pair<uint32_t, Ptr<PacketBurst> > (tag.GetSfn ().Encode (), CreateObject<PacketBurst> ())).first;
            }
          else
            {
              NS_FATAL_ERROR ("Packet burst map entry already exists");
            }
          it->second->AddPacket (p);
        }
      else
        {
          std::map<uint64_t, Ptr<PacketBurst>>::iterator it = m_packetBurstLayerMap.find (tag.Encode());
          if (it == m_packetBurstLayerMap.end ())
            {
              it = m_packetBurstLayerMap.insert (std::pair<uint64_t, Ptr<PacketBurst> > (tag.Encode(), CreateObject<PacketBurst> ())).first;
            }
          else
            {
              NS_FATAL_ERROR ("Packet burst map entry already exists");
            }
          it->second->AddPacket (p);
        }
    }
  else
    {
      NS_FATAL_ERROR ("No MAC packet PDU header available");
    }
}

Ptr<PacketBurst>
MmWavePhy::GetPacketBurst (SfnSf sfn)
{
  Ptr<PacketBurst> pburst;
  std::map<uint32_t, Ptr<PacketBurst> >::iterator it = m_packetBurstMap.find (sfn.Encode ());
  if (it == m_packetBurstMap.end ())
    {
      NS_LOG_ERROR ("GetPacketBurst(): Packet burst not found for subframe " << (unsigned)sfn.m_sfNum << " slot/sym start "  << (unsigned)sfn.m_slotNum);
      return pburst;
    }
  else
    {
      pburst = it->second;
      m_packetBurstMap.erase (it);
    }
  return pburst;
}

Ptr<PacketBurst>
MmWavePhy::GetPacketBurst (SfnSf sfn, uint8_t layerInd)
{
  Ptr<PacketBurst> pburst;
  MmWaveMacPduTag pduTag (sfn, 0, 0, 0, layerInd); // 0 values equal null
  NS_LOG_INFO ("Frame:" << (unsigned)sfn.m_frameNum << ", subframe:" << (unsigned)sfn.m_sfNum << ", symStart:" << (unsigned)sfn.m_slotNum << ", layerInd: " << (int)layerInd);
  std::map<uint64_t, Ptr<PacketBurst> >::iterator it = m_packetBurstLayerMap.find (pduTag.Encode ());
  if (it == m_packetBurstLayerMap.end ())
    {
      NS_LOG_ERROR ("GetPacketBurst(): Packet burst not found for subframe " << (unsigned)sfn.m_sfNum << " slot/sym start "  << (unsigned)sfn.m_slotNum << " layer index " << (int)layerInd);
      return pburst;
    }
  else
    {
      pburst = it->second;
      m_packetBurstLayerMap.erase (it);
    }
  return pburst;
}

void
MmWavePhy::SetControlMessage (Ptr<MmWaveControlMessage> m)
{
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<MmWaveControlMessage> > l;
      l.push_back (m);
      m_controlMessageQueue.push_back (l);
    }
  else
    {
      m_controlMessageQueue.at (m_controlMessageQueue.size () - 1).push_back (m);
    }
}

std::list<Ptr<MmWaveControlMessage> >
MmWavePhy::GetControlMessages (void)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }

  if (m_controlMessageQueue.at (0).size () > 0)
    {
      std::list<Ptr<MmWaveControlMessage> > ret = m_controlMessageQueue.front ();
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      return (ret);
    }
  else
    {
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }
}

void
MmWavePhy::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
  m_phyMacConfig = ptrConfig;
}

Ptr<MmWavePhyMacCommon>
MmWavePhy::GetConfigurationParameters (void) const
{
  return m_phyMacConfig;
}


MmWavePhySapProvider*
MmWavePhy::GetPhySapProvider ()
{
  return m_phySapProvider;
}


SfAllocInfo
MmWavePhy::GetSfAllocInfo (uint8_t subframeNum)
{
  return m_sfAllocInfo[subframeNum];
}

void
MmWavePhy::SetDlSfAllocInfo (SfAllocInfo sfAllocInfo)
{
  // get previously enqueued SfAllocInfo and set DL slot allocations
  //SfAllocInfo &sf = m_sfAllocInfo[sfAllocInfo.m_sfnSf.m_sfNum];
  // merge slot lists
  //sf.m_dlSlotAllocInfo = sfAllocInfo.m_dlSlotAllocInfo;
  m_sfAllocInfo[sfAllocInfo.m_sfnSf.m_sfNum] = sfAllocInfo;
  //m_sfAllocInfoUpdated = true;
}

void
MmWavePhy::SetUlSfAllocInfo (SfAllocInfo sfAllocInfo)
{
  // add new SfAllocInfo with UL slot allocation
  //m_sfAllocInfo[sfAllocInfo.m_sfnSf.m_sfNum] = sfAllocInfo;
}

void
MmWavePhy::AddPropagationLossModel (Ptr<PropagationLossModel> model)
{
  m_propagationLoss = model;
}

void
MmWavePhy::AddLosTracker (Ptr<MmWaveLosTracker> losTracker)
{
  m_losTracker = losTracker;
}

void
MmWavePhy::AddSpectrumPropagationLossModel (Ptr<SpectrumPropagationLossModel> model)
{
  m_spectrumPropagationLossModel = model;
}

void
MmWavePhy::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
  if (m_downlinkSpectrumPhy != 0) //For UE
    {
      m_downlinkSpectrumPhy->SetComponentCarrierId (index);
      m_uplinkSpectrumPhy->SetComponentCarrierId (index);
    }
  else //For gNB
    {
      uint8_t numEnbLayers = m_phyMacConfig->GetNumEnbLayers ();
      for (uint8_t layerInd = 0; layerInd < numEnbLayers; layerInd++)
        {
          m_downlinkSpectrumPhyList.at (layerInd)->SetComponentCarrierId (index);
          m_uplinkSpectrumPhyList.at (layerInd)->SetComponentCarrierId (index);
        }
    }
}

uint8_t
MmWavePhy::GetComponentCarrierId ()
{
  return m_componentCarrierId;
}

}

}
