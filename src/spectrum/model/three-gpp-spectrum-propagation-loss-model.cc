/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering,
 * New York University
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
 * University of Padova
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
 */

#include "ns3/log.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/net-device.h"
#include "ns3/antenna-array-basic-model.h"
#include "ns3/node.h"
#include "ns3/channel-condition-model.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ThreeGppSpectrumPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (ThreeGppSpectrumPropagationLossModel);

ThreeGppSpectrumPropagationLossModel::ThreeGppSpectrumPropagationLossModel ()
{
  NS_LOG_FUNCTION (this);
  m_channelModel = CreateObject<ThreeGppChannel> ();
}

ThreeGppSpectrumPropagationLossModel::~ThreeGppSpectrumPropagationLossModel ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ThreeGppSpectrumPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ThreeGppSpectrumPropagationLossModel")
    .SetParent<SpectrumPropagationLossModel> ()
    .SetGroupName ("Spectrum")
    .AddConstructor<ThreeGppSpectrumPropagationLossModel> ()
    .AddAttribute ("Frequency",
                   "The operating Frequency in Hz",
                   DoubleValue (0),
                   MakeDoubleAccessor (&ThreeGppSpectrumPropagationLossModel::SetFrequency,
                                       &ThreeGppSpectrumPropagationLossModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Scenario",
                   "The 3GPP scenario (RMa, UMa, UMi-StreetCanyon, InH-OfficeOpen, InH-OfficeMixed)",
                   StringValue (""),
                   MakeStringAccessor (&ThreeGppSpectrumPropagationLossModel::SetScenario,
                                       &ThreeGppSpectrumPropagationLossModel::GetScenario),
                   MakeStringChecker ())
    .AddAttribute ("ChannelConditionModel",
                   "Pointer to the channel condition model",
                   PointerValue (),
                   MakePointerAccessor (&ThreeGppSpectrumPropagationLossModel::SetChannelConditionModel,
                                        &ThreeGppSpectrumPropagationLossModel::GetChannelConditionModel),
                   MakePointerChecker<ChannelConditionModel> ())
    ;
  return tid;
}

void
ThreeGppSpectrumPropagationLossModel::AddDevice (Ptr<NetDevice> dev, Ptr<AntennaArrayBasicModel> antenna)
{
  NS_ASSERT_MSG (m_deviceAntennaMap.find (dev) == m_deviceAntennaMap.end (), "Device is already present in the map");
  m_deviceAntennaMap.insert (std::pair <Ptr<NetDevice>, Ptr<AntennaArrayBasicModel>> (dev, antenna));
}

void
ThreeGppSpectrumPropagationLossModel::SetChannelConditionModel (Ptr<ChannelConditionModel> model)
{
  m_channelConditionModel = model;
}

Ptr<ChannelConditionModel>
ThreeGppSpectrumPropagationLossModel::GetChannelConditionModel () const
{
  return m_channelConditionModel;
}

void
ThreeGppSpectrumPropagationLossModel::SetFrequency (double frequency)
{
  m_channelModel->SetAttribute ("Frequency", DoubleValue (frequency));
}

double
ThreeGppSpectrumPropagationLossModel::GetFrequency () const
{
  DoubleValue freq;
  m_channelModel->GetAttribute ("Frequency", freq);
  return freq.Get ();
}

void
ThreeGppSpectrumPropagationLossModel::SetScenario (std::string scenario)
{
  m_channelModel->SetAttribute ("Scenario", StringValue (scenario));
}

std::string
ThreeGppSpectrumPropagationLossModel::GetScenario () const
{
  StringValue scenario;
  m_channelModel->GetAttribute ("Scenario", scenario);
  return scenario.Get ();
}

complexVector_t
ThreeGppSpectrumPropagationLossModel::CalLongTerm (Ptr<ThreeGppChannelMatrix> params, complexVector_t aW, complexVector_t bW) const
{
  // NOTE We assume channel reciprocity between each tx-rx pair, hence the
  // channel matrix H is generated once for each pair.
  // If the channel matrix H has been generated considering device a as tx and
  // device b as rx, but we need to compute the long term for the reverse link
  // we need to transpose the matrix or we can just invert the BF vectors,
  // i.e., rxW^T H^T txW = (rxW^T H^T txW)^T = txW^T H rxW

  complexVector_t txW, rxW;
  if (params->m_isReverse)
  {
    // TODO we should never enter in this branch because we should have already
    // computed the long term for the direct link. Consider to remove it

    // the channel matrix was generated considering device b as tx and device
    // a as rx
    txW = bW;
    rxW = aW;
  }
  else
  {
    // the channel matrix was generated considering device a as tx and device
    // b as rx
    txW = aW;
    rxW = bW;
  }

  uint16_t txAntenna = txW.size ();
  uint16_t rxAntenna = rxW.size ();

  NS_LOG_DEBUG ("CalLongTerm with txAntenna " << (uint16_t)txAntenna << " rxAntenna " << (uint16_t)rxAntenna);
  //store the long term part to reduce computation load
  //only the small scale fading needs to be updated if the large scale parameters and antenna weights remain unchanged.
  complexVector_t longTerm;
  uint8_t numCluster = params->m_channel.at (0).at (0).size ();

  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      std::complex<double> txSum (0,0);
      for (uint16_t txIndex = 0; txIndex < txAntenna; txIndex++)
        {
          std::complex<double> rxSum (0,0);
          for (uint16_t rxIndex = 0; rxIndex < rxAntenna; rxIndex++)
            {
              rxSum = rxSum + std::conj (rxW.at (rxIndex)) * params->m_channel.at (rxIndex).at (txIndex).at (cIndex);
            }
          txSum = txSum + txW.at (txIndex) * rxSum;
        }
      longTerm.push_back (txSum);
    }
  return longTerm;
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::CalBeamformingGain (Ptr<SpectrumValue> txPsd, complexVector_t longTerm, Ptr<ThreeGppChannelMatrix> params, Vector txSpeed, Vector rxSpeed) const
{
  NS_LOG_FUNCTION (this);

  Ptr<SpectrumValue> tempPsd = Copy<SpectrumValue> (txPsd);

  //channel[rx][tx][cluster]
  uint8_t numCluster = params->m_channel.at (0).at (0).size ();

  //the update of Doppler is simplified by only taking the center angle of each cluster in to consideration.
  Values::iterator vit = tempPsd->ValuesBegin ();
  Bands::const_iterator sbit = tempPsd->ConstBandsBegin(); // sub band iterator

  uint16_t iSubband = 0;
  double slotTime = Simulator::Now ().GetSeconds ();
  complexVector_t doppler;
  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      //cluster angle angle[direction][n],where, direction = 0(aoa), 1(zoa).
      // TODO should I include the "alfa" term for the Doppler of delayed paths?
      double temp_doppler = 2 * M_PI * ((sin (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * cos (params->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.x
                                        + sin (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * sin (params->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.y
                                        + cos (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.z)
                                        + (sin (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * cos (params->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.x
                                        + sin (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * sin (params->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.y
                                        + cos (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.z))
                                        * slotTime * GetFrequency () / 3e8;
      doppler.push_back (exp (std::complex<double> (0, temp_doppler)));
    }

  while (vit != tempPsd->ValuesEnd ())
    {
      std::complex<double> subsbandGain (0.0,0.0);
      if ((*vit) != 0.00)
        {
          double fsb = (*sbit).fc;
          for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
            {
              double delay = -2 * M_PI * fsb * (params->m_delay.at (cIndex));
              subsbandGain = subsbandGain + longTerm.at (cIndex) * doppler.at (cIndex) * exp (std::complex<double> (0, delay));
            }
          *vit = (*vit) * (norm (subsbandGain));
        }
      vit++;
      iSubband++;
    }
  return tempPsd;
}

complexVector_t
ThreeGppSpectrumPropagationLossModel::GetLongTerm (Ptr<const MobilityModel> aMob, Ptr<const MobilityModel> bMob, Ptr<ThreeGppChannelMatrix> channelMatrix, complexVector_t aW, complexVector_t bW) const
{
  complexVector_t longTerm; // vector containing the long term component for each cluster

  // compute the long term key, the key is unique for each tx-rx pair
  uint32_t minId = std::min (aMob->GetObject<Node> ()->GetId (), bMob->GetObject<Node> ()->GetId ());
  uint32_t maxId = std::max (aMob->GetObject<Node> ()->GetId (), bMob->GetObject<Node> ()->GetId ());
  uint32_t longTermId = ThreeGppChannel::GetKey (minId, maxId);

  bool update = false; // indicates whether the long term has to be updated
  bool notFound = false; // indicates if the long term has not been computed yet

  // look for the long term in the map and check if it is valid
  if (m_longTermMap.find (longTermId) != m_longTermMap.end ())
  {
    NS_LOG_DEBUG ("found the long term component in the map");
    longTerm = m_longTermMap.at (longTermId)->m_longTerm;

    // check if the channel matrix has been updated
    // or the tx beam has been changed
    // or the rx beam has been changed
    if (m_longTermMap.at (longTermId)->m_channel->m_isReverse)
    {
      // the long term was computed considering device b as tx and device a as rx
      update = (m_longTermMap.at (longTermId)->m_channel->m_generatedTime != channelMatrix->m_generatedTime
                || m_longTermMap.at (longTermId)->m_txW != bW
                || m_longTermMap.at (longTermId)->m_rxW != aW);
    }
    else
    {
      // the long term was computed considering device a as tx and device b as rx
      update = (m_longTermMap.at (longTermId)->m_channel->m_generatedTime != channelMatrix->m_generatedTime
                || m_longTermMap.at (longTermId)->m_txW != aW
                || m_longTermMap.at (longTermId)->m_rxW != bW);
    }
  }
  else
  {
    NS_LOG_DEBUG ("long term component NOT found");
    notFound = true;
  }

  if (update || notFound)
  {
    NS_LOG_DEBUG ("compute the long term");
    // compute the long term component
    longTerm = CalLongTerm (channelMatrix, aW, bW);

    // store the long term
    Ptr<LongTerm> longTermItem = Create<LongTerm> ();
    longTermItem->m_longTerm = longTerm;
    longTermItem->m_channel = channelMatrix;
    longTermItem->m_txW = aW;
    longTermItem->m_rxW = bW;

    m_longTermMap[longTermId] = longTermItem;
  }

  return longTerm;
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                 Ptr<const MobilityModel> a,
                                                 Ptr<const MobilityModel> b) const
{

  // we made this function do nothing in order to relocate the actual BF vector calculation
  // to MmwaveSpectrumPhy::StartRx() instead of MultiModelSpectrumChannel::StartTx()
  // uncomment the following to restore legacy behavior (not compatible with HBF)
  //TODO implement a more suitable integration of HBF to the main branch of ns3 in the future
  //return DoCalcRxPowerSpectralDensityMultiLayers (txPsd, a, b, 0, true);

  Ptr<SpectrumValue> retPsd = Copy<SpectrumValue> (txPsd);// we have to copy it because there is a const type missmatch
  return retPsd;
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::CalcRxPowerSpectralDensityMultiLayers (Ptr<const SpectrumValue> txPsd,
                                                         Ptr<const MobilityModel> a,
                                                         Ptr<const MobilityModel> b,
                                                         uint8_t layerInd) const
{
  return DoCalcRxPowerSpectralDensityMultilayers (txPsd, a, b, layerInd);
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensityMultilayers (Ptr<const SpectrumValue> txPsd,
                                                                    Ptr<const MobilityModel> a,
                                                                    Ptr<const MobilityModel> b,
			                                            uint8_t layerInd) const
{
  NS_LOG_FUNCTION (this);

  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue> (txPsd);

  // retrieve the tx and rx devices
  Ptr<NetDevice> txDevice = a->GetObject<Node> ()->GetDevice (0);
  Ptr<NetDevice> rxDevice = b->GetObject<Node> ()->GetDevice (0);

  // retrieve the antenna of the tx device
  NS_ASSERT_MSG (m_deviceAntennaMap.find (txDevice) != m_deviceAntennaMap.end (), "Antenna not found for device " << txDevice);
  Ptr<AntennaArrayBasicModel> txAntennaArray = m_deviceAntennaMap.at (txDevice);
  NS_LOG_DEBUG ("tx dev " << txDevice << " antenna " << txAntennaArray);

  // retrieve the antenna of the rx device
  NS_ASSERT_MSG (m_deviceAntennaMap.find (txDevice) != m_deviceAntennaMap.end (), "Antenna not found for device " << rxDevice);
  Ptr<AntennaArrayBasicModel> rxAntennaArray = m_deviceAntennaMap.at (rxDevice);
  NS_LOG_DEBUG ("rx dev " << rxDevice << " antenna " << rxAntennaArray);

  if (txAntennaArray->IsOmniTx () || rxAntennaArray->IsOmniTx () )
    {
      NS_LOG_LOGIC ("Omni transmission, do nothing.");
      return rxPsd;
    }

  NS_ASSERT_MSG (a->GetDistanceFrom (b) != 0, "The position of tx and rx devices cannot be the same");

  // retrieve the channel condition
  Ptr<ChannelCondition> condition = m_channelConditionModel->GetChannelCondition (a, b);

  // compute the channel matrix between a and b
  bool los = (condition->GetLosCondition () == ChannelCondition::LosConditionValue::LOS);
  bool o2i = false; // TODO include the o2i condition in the channel condition model
  Ptr<ThreeGppChannelMatrix> channelMatrix = m_channelModel->GetChannel (a, b, txAntennaArray, rxAntennaArray, los, o2i);

  // get the precoding and combining vectors
  AntennaArrayBasicModel::BeamformingVector txW = txAntennaArray->GetCurrentBeamformingVector ();
  AntennaArrayBasicModel::BeamformingVector rxW = rxAntennaArray->GetCurrentBeamformingVector ();

  // retrieve the long term component
  complexVector_t longTerm = GetLongTerm (a, b, channelMatrix, AntennaArrayBasicModel::GetVector (txW), AntennaArrayBasicModel::GetVector (rxW));

  // apply the beamforming gain
  rxPsd = CalBeamformingGain (rxPsd, longTerm, channelMatrix, a->GetVelocity (), b->GetVelocity ());

  return rxPsd;
}


}  // namespace ns3
