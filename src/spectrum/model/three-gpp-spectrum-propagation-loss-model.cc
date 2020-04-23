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
#include "ns3/antenna-array-model.h"
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

//TODO is this the best place to define this operator? it can be made "inline" in the .h file
bool operator<(Key3DLongTerm const & lhs, Key3DLongTerm const & rhs) {
    if (lhs.a < rhs.a) return true;
    if (rhs.a < lhs.a) return false;
    if (lhs.b < rhs.b) return true;
    if (rhs.b < lhs.b) return false;
    if (lhs.c < rhs.c) return true;
    return false;
};

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
    // computed the long term for the direct link. Consider to remove it (NOTE: Fgomez this point can be reached in the HBF extension when more than one bf vector is evaluated for the same channel matrix)

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
  uint16_t txChSize = params->m_channel.at (0).size ();
  uint16_t rxChSize = params->m_channel.size ();

  NS_LOG_DEBUG ("CalLongTerm with txAntenna " << (uint16_t)txAntenna << " rxAntenna " << (uint16_t)rxAntenna<<" txChSize " << (uint16_t)txChSize << " rxChSize " << (uint16_t)rxChSize);
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
              rxSum = rxSum + rxW.at (rxIndex) * params->m_channel.at (rxIndex).at (txIndex).at (cIndex);
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

  Ptr<SpectrumValue> tempPsd = Copy (txPsd);

  complexVector_t tempComplexCoef = CalBeamformingComplexCoef ( tempPsd, longTerm, params, txSpeed, rxSpeed);


//  //channel[rx][tx][cluster]
//  uint8_t numCluster = params->m_channel.at (0).at (0).size ();
//
//  //the update of Doppler is simplified by only taking the center angle of each cluster in to consideration.
  Values::iterator vit = tempPsd->ValuesBegin ();
//  Bands::const_iterator sbit = tempPsd->ConstBandsBegin(); // sub band iterator
//
  uint16_t iSubband = 0;
//  double slotTime = Simulator::Now ().GetSeconds ();
//  complexVector_t doppler;
//  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
//    {
//      //cluster angle angle[direction][n],where, direction = 0(aoa), 1(zoa).
//      // TODO should I include the "alfa" term for the Doppler of delayed paths?
//      double temp_doppler = 2 * M_PI * ((sin (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * cos (params->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.x
//                                        + sin (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * sin (params->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.y
//                                        + cos (params->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * rxSpeed.z)
//                                        + (sin (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * cos (params->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.x
//                                        + sin (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * sin (params->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.y
//                                        + cos (params->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * txSpeed.z))
//                                        * slotTime * GetFrequency () / 3e8;
//      doppler.push_back (exp (std::complex<double> (0, temp_doppler)));
//    }
//
  while (vit != tempPsd->ValuesEnd ())
    {
//      std::complex<double> subsbandGain (0.0,0.0);
//      if ((*vit) != 0.00)
//        {
//          double fsb = GetFrequency (); //TODO this is a temporary fix to test MMSE beamforming in a frequency-flat channel. We must restore the behavior on next line and implement frequency-selective MMSE beamforming in the future
////          double fsb = (*sbit).fc; //TODO it seems that the iterator (*sbit) is never changed, this may be a bug resulting in the fc of subcarrier 0 used for the doppler of all bands
////          NS_LOG_UNCOND("fsb: "<<fsb);
//          for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
//            {
//              double delay = -2 * M_PI * fsb * (params->m_delay.at (cIndex));
//              subsbandGain = subsbandGain + longTerm.at (cIndex) * doppler.at (cIndex) * exp (std::complex<double> (0, delay));
//            }
//          *vit = norm (subsbandGain);
//        }
      *vit = norm(tempComplexCoef.at(iSubband));
      vit++;
      iSubband++;//TODO it seems iSubband does nothing whereas the subband iterator *sbit has not been incremented in this loop
//      sbit++;
    }
  return tempPsd;
}

complexVector_t
ThreeGppSpectrumPropagationLossModel::CalBeamformingComplexCoef (Ptr<SpectrumValue> refPsd, complexVector_t longTerm, Ptr<ThreeGppChannelMatrix> params, Vector txSpeed, Vector rxSpeed) const
{
  NS_LOG_FUNCTION (this);

  //channel[rx][tx][cluster]
  uint8_t numCluster = params->m_channel.at (0).at (0).size ();
  complexVector_t tempComplexSpectrum;

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

  for (Bands::const_iterator sbit = refPsd->ConstBandsBegin(); sbit != refPsd->ConstBandsEnd (); sbit++)
    {
      std::complex<double> subsbandGain (0.0,0.0);

      double fsb = (*sbit).fc;
      for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
        {
          double delay = -2 * M_PI * fsb * (params->m_delay.at (cIndex));
          subsbandGain = subsbandGain + longTerm.at (cIndex) * doppler.at (cIndex) * exp (std::complex<double> (0, delay));
        }
      tempComplexSpectrum.push_back(subsbandGain);
    }
  return tempComplexSpectrum;
}


complexVector_t
ThreeGppSpectrumPropagationLossModel::GetLongTerm (Ptr<const MobilityModel> aMob, Ptr<const MobilityModel> bMob, Ptr<ThreeGppChannelMatrix> channelMatrix, AntennaArrayBasicModel::BeamformingVector aBF, AntennaArrayBasicModel::BeamformingVector bBF) const
{
  complexVector_t longTerm; // vector containing the long term component for each cluster

  complexVector_t aW = AntennaArrayBasicModel::GetVector (aBF);
  complexVector_t bW = AntennaArrayBasicModel::GetVector (bBF);

  //compute the long term key, the key is unique for each tx-rx pair
  uint32_t minId = std::min (aMob->GetObject<Node> ()->GetId (), bMob->GetObject<Node> ()->GetId ());
  uint32_t maxId = std::max (aMob->GetObject<Node> ()->GetId (), bMob->GetObject<Node> ()->GetId ());
  uint32_t longTermId = ThreeGppChannel::GetKey (minId, maxId);

  //TODO uncomment the following if we change our mind and decide to disable the interference caching implementation
//  bool interference = true;
//  if ( ( AntennaArrayBasicModel::GetBeamId(aBF) == longTermId ) && ( AntennaArrayBasicModel::GetBeamId(bBF) == longTermId ) )
//    {
//      interference = false;
//    }
  //TODO remove this variable and substitute it with longTermId hereafter if we change our mind and decide to disable the interference caching implementation
  Key3DLongTerm longTerm3Key = {
      AntennaArrayBasicModel::GetBeamId(aBF),
      longTermId,
      AntennaArrayBasicModel::GetBeamId(bBF)
      };

  bool update = false; // indicates whether the long term has to be updated
  bool notFound = false; // indicates if the long term has not been computed yet

  // look for the long term in the map and check if it is valid
  if (m_longTermMap.find (longTerm3Key) != m_longTermMap.end ())
  {
    NS_LOG_DEBUG ("found the long term component in the map");
    longTerm = m_longTermMap.at (longTerm3Key)->m_longTerm;

    // check if the channel matrix has been updated
    // or the tx beam has been changed
    // or the rx beam has been changed
    if (m_longTermMap.at (longTerm3Key)->m_channel->m_isReverse)
    {
      // the long term was computed considering device b as tx and device a as rx
      update = (m_longTermMap.at (longTerm3Key)->m_channel->m_generatedTime != channelMatrix->m_generatedTime
                || m_longTermMap.at (longTerm3Key)->m_txW != bW
                || m_longTermMap.at (longTerm3Key)->m_rxW != aW);
    }
    else
    {
      // the long term was computed considering device a as tx and device b as rx
      update = (m_longTermMap.at (longTerm3Key)->m_channel->m_generatedTime != channelMatrix->m_generatedTime
                || m_longTermMap.at (longTerm3Key)->m_txW != aW
                || m_longTermMap.at (longTerm3Key)->m_rxW != bW);
    }
  }
  else
  {
    NS_LOG_DEBUG ("long term component NOT found");
    notFound = true;
  }

  //TODO swap the following commented and uncommented lines if we change our mind and decide to disable the interference caching implementation
  if (update || notFound)
//  if (update || notFound || interference)
  {
    NS_LOG_DEBUG ("compute the long term for channel ID "<<longTermId<<" using tx bf Id "<<AntennaArrayBasicModel::GetBeamId(aBF)<<" and rx bf Id "<<AntennaArrayBasicModel::GetBeamId(bBF));
    // compute the long term component
    longTerm = CalLongTerm (channelMatrix, aW, bW);

    //TODO uncomment the following if we change our mind and decide to disable the interference caching implementation
//    if ( ! interference)
//      {
      // store the long term
      Ptr<LongTerm> longTermItem = Create<LongTerm> ();
      longTermItem->m_longTerm = longTerm;
      longTermItem->m_channel = channelMatrix;
      longTermItem->m_txW = aW;
      longTermItem->m_rxW = bW;

      m_longTermMap[longTerm3Key] = longTermItem;
//      }
  }

  return longTerm;
}

complex2DVector_t
ThreeGppSpectrumPropagationLossModel::GetFrequencyFlatChannelMatrixAtDeltaFrequency ( Ptr<const MobilityModel> a,
										      Ptr<const MobilityModel> b,
										      double deltaFc)
{
  //TODO this function shares a lot of code with calcLongTerm and calcBFGain, try to reuse more code
  //TODO we should be able to cache this too

  // retrieve the tx and rx devices
  Ptr<NetDevice> txDevice = a->GetObject<Node> ()->GetDevice (0);
  Ptr<NetDevice> rxDevice = b->GetObject<Node> ()->GetDevice (0);

  NS_ASSERT_MSG (a->GetDistanceFrom (b) != 0, "The position of tx and rx devices cannot be the same");

  // retrieve the channel condition
  Ptr<ChannelCondition> condition = m_channelConditionModel->GetChannelCondition (a, b);

  // compute the channel matrix between a and b
  bool los = (condition->GetLosCondition () == ChannelCondition::LosConditionValue::LOS);
  bool o2i = false; // TODO include the o2i condition in the channel condition model

  // retrieve the antenna of the tx device
  NS_ASSERT_MSG (m_deviceAntennaMap.find (txDevice) != m_deviceAntennaMap.end (), "Antenna not found for device " << txDevice);
  Ptr<AntennaArrayBasicModel> txAntennaArray = m_deviceAntennaMap.at (txDevice);
  NS_LOG_DEBUG ("tx dev " << txDevice << " antenna " << txAntennaArray);

  // retrieve the antenna of the rx device
  NS_ASSERT_MSG (m_deviceAntennaMap.find (rxDevice) != m_deviceAntennaMap.end (), "Antenna not found for device " << rxDevice);
  Ptr<AntennaArrayBasicModel> rxAntennaArray = m_deviceAntennaMap.at (rxDevice);
  NS_LOG_DEBUG ("rx dev " << rxDevice << " antenna " << rxAntennaArray);

  Ptr<ThreeGppChannelMatrix> channelMatrix = m_channelModel->GetChannel (a, b, txAntennaArray, rxAntennaArray, los, o2i);

  //channel[rx][tx][cluster]
  uint8_t numCluster = channelMatrix->m_channel.at (0).at (0).size ();

  uint16_t numTxAntenna = txAntennaArray->GetAntennaNumDim1 () * txAntennaArray->GetAntennaNumDim2 ();
  uint16_t numRxAntenna = rxAntennaArray->GetAntennaNumDim1 () * rxAntennaArray->GetAntennaNumDim2 ();
  if ( channelMatrix->m_isReverse )
    {
      NS_ASSERT_MSG (channelMatrix->m_channel.size() == numTxAntenna,"matrix dimensions mismatch tx array");
      NS_ASSERT_MSG (channelMatrix->m_channel.at(0).size() == numRxAntenna,"matrix dimensions mismatch rx array");
    }
  else
    {
      NS_ASSERT_MSG (channelMatrix->m_channel.size() == numRxAntenna,"matrix dimensions mismatch tx array");
      NS_ASSERT_MSG (channelMatrix->m_channel.at(0).size() == numTxAntenna,"matrix dimensions mismatch rx array");
    }

  //precompute delay and doppler to accelerate next loop
  complexVector_t doppler;
  complexVector_t delay_doppler;
  double slotTime = Simulator::Now ().GetSeconds ();
  double fsb = deltaFc + GetFrequency (); //we assume the reference signal is Deltafc distant from the center frequency of the channel model
  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      //cluster angle angle[direction][n],where, direction = 0(aoa), 1(zoa).
      // TODO should I include the "alfa" term for the Doppler of delayed paths?
      double temp_doppler = 2 * M_PI * ((sin (channelMatrix->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * cos (channelMatrix->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * b->GetVelocity ().x
					+ sin (channelMatrix->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * sin (channelMatrix->m_angle.at (ThreeGppChannel::AOA_INDEX).at (cIndex) * M_PI / 180) * b->GetVelocity ().y
					+ cos (channelMatrix->m_angle.at (ThreeGppChannel::ZOA_INDEX).at (cIndex) * M_PI / 180) * b->GetVelocity ().z)
					+ (sin (channelMatrix->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * cos (channelMatrix->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * a->GetVelocity ().x
					+ sin (channelMatrix->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * sin (channelMatrix->m_angle.at (ThreeGppChannel::AOD_INDEX).at (cIndex) * M_PI / 180) * a->GetVelocity ().y
					+ cos (channelMatrix->m_angle.at (ThreeGppChannel::ZOD_INDEX).at (cIndex) * M_PI / 180) * a->GetVelocity ().z))
					* slotTime * GetFrequency () / 3e8;

      doppler.push_back ( exp ( std::complex<double> (0, temp_doppler ) ) );

      double delay = -2 * M_PI * fsb * (channelMatrix->m_delay.at (cIndex));
      delay_doppler.push_back ( exp (std::complex<double> (0, delay ) ) );
    }

  complex2DVector_t resultMatrix; //this starts with empty matrix
  for (uint16_t rxIndex = 0; rxIndex < numRxAntenna; rxIndex++)
    {
      complexVector_t resultRow;//this starts adding an empty row to the matrix
      for (uint16_t txIndex = 0; txIndex < numTxAntenna; txIndex++)
	{
	  std::complex<double> resultElement (0,0);//this adds a zero element to the new row of the matrix
	  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
	    {
	      if ( channelMatrix->m_isReverse )
		{//we read from the ChannelMatrix in reverse but store in the current order, effectively conjugating the result

		  resultElement +=   channelMatrix->m_channel.at (txIndex).at (rxIndex).at (cIndex) * doppler.at (cIndex) * delay_doppler.at(cIndex);
//		  NS_LOG_DEBUG ("REVERSE Computing step ["<< rxIndex <<","<< txIndex <<","<< (int )cIndex <<
//				"] Array size = ["<<  channelMatrix->m_channel.at(0).size() <<","<< channelMatrix->m_channel.size() <<","<< channelMatrix->m_channel.at(0).at(0).size() <<
//				"] loop limits ["<< numRxAntenna <<","<< numTxAntenna<<","<<(int )numCluster<<
//				"] gain "<< channelMatrix->m_channel.at (txIndex).at (rxIndex).at (cIndex) <<
//				" doppler "<< doppler.at (cIndex)<<
//				" delay doppler "<< delay_doppler.at(cIndex));
		}
	      else
		{
		   resultElement +=  channelMatrix->m_channel.at (rxIndex).at (txIndex).at (cIndex) * doppler.at (cIndex) * delay_doppler.at(cIndex);
//		   NS_LOG_DEBUG ("Computing reverse step ["<< rxIndex <<","<< txIndex <<","<< (int )cIndex <<
//				 "] Array size = ["<<  channelMatrix->m_channel.size() <<","<< channelMatrix->m_channel.at(0).size() <<","<< channelMatrix->m_channel.at(0).at(0).size() <<
//				 "] loop limits "<< numRxAntenna <<","<< numTxAntenna<<","<<(int )numCluster<<
//				 "]  gain "<< channelMatrix->m_channel.at (rxIndex).at (txIndex).at (cIndex) <<
//				" doppler "<< doppler.at (cIndex)<<
//				" delay doppler "<< delay_doppler.at(cIndex));
		}
	    }
	  resultRow.push_back(resultElement);
	}
      resultMatrix.push_back (resultRow);
    }
  return (resultMatrix);
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity ( Ptr<const SpectrumValue> txPsd,
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
		                                            uint8_t txLayerInd,
							    uint8_t rxLayerInd) const
{
  return DoCalcRxPowerSpectralDensityMultilayers (txPsd, a, b, txLayerInd, rxLayerInd);
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensityMultilayers (Ptr<const SpectrumValue> txPsd,
                                                                    Ptr<const MobilityModel> a,
                                                                    Ptr<const MobilityModel> b,
			                                            uint8_t txLayerInd,
								    uint8_t rxLayerInd) const
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
  Ptr<AntennaArrayModel> castTxArray=DynamicCast<AntennaArrayModel>(txAntennaArray);
  Ptr<AntennaArrayModel> castRxArray=DynamicCast<AntennaArrayModel>(rxAntennaArray);

  AntennaArrayBasicModel::BeamformingVector txW = castTxArray->GetCurrentBeamformingVectorMultilayers (txLayerInd);
  AntennaArrayBasicModel::BeamformingVector rxW = castRxArray->GetCurrentBeamformingVectorMultilayers (rxLayerInd);

  Ptr<SpectrumValue>  bfGainPsd = Create<SpectrumValue>( rxPsd->GetSpectrumModel () );

  if (castTxArray->isDigitalCombiningOn())
    {
      NS_ASSERT_MSG ( ! castRxArray->isDigitalCombiningOn() , "Digital combining at both transmitter and receiver is impossible if one is a UE");
      AntennaArrayModel::complex3DVector_t mmseWDCmatrix = castTxArray->GetDigitalCombining();
      if ( mmseWDCmatrix.at(0).size()<=txLayerInd )
        {//when we are called with a value of txLayerInd greater than the number of active layers
         //for example if a mmwave-phy calls startTx for all its layers indiscriminately, including those that are not allocated
         //by convention we assume unallocated layers transmit 0 power and thus do not send out unnecessary interference
          (*bfGainPsd)=0;
        }
      else
        {
          complexVector_t bfComplexSpectrum ( mmseWDCmatrix.size() , 0.0) ;
          for (uint8_t txLayerCtr = 0; txLayerCtr< mmseWDCmatrix.at(0).size(); txLayerCtr++ )
            {
              AntennaArrayBasicModel::BeamformingVector txWaux = castTxArray->GetCurrentBeamformingVectorMultilayers ( txLayerCtr );
              complexVector_t longTerm = GetLongTerm (a, b, channelMatrix, txWaux, rxW);
              complexVector_t bfComplexNewComponent = CalBeamformingComplexCoef (rxPsd, longTerm, channelMatrix, a->GetVelocity (), b->GetVelocity ());          ;
              for ( size_t sBandCtr = 0; sBandCtr < mmseWDCmatrix.size(); sBandCtr++)
                {//if there are no bugs dimensions always match
                  bfComplexSpectrum.at( sBandCtr ) += mmseWDCmatrix.at( sBandCtr ).at( txLayerInd ).at( txLayerCtr ) * bfComplexNewComponent.at( sBandCtr );
                }
            }
          for ( size_t sBandCtr = 0; sBandCtr < mmseWDCmatrix.size(); sBandCtr++)
            {
              (*bfGainPsd)[sBandCtr] = std::norm( bfComplexSpectrum.at( sBandCtr ) );
            }
        }
    }
  else if(castRxArray->isDigitalCombiningOn())
    {
      AntennaArrayModel::complex3DVector_t mmseWDCmatrix = castRxArray->GetDigitalCombining();
      if ( mmseWDCmatrix.at(0).size()<=rxLayerInd )
        {//when we are called with a value of rxLayerInd greater than the number of active layers
         //for example, for example if a mmwave-phy  is always trying to receive in all its layers, entering this segment of code for the layers it has not allocated
          //by convention we assume unallocated layers receive 0 power and thus do not capture unnecessary noise and interference
          (*bfGainPsd)=0;
        }
      else
        {
          complexVector_t bfComplexSpectrum ( mmseWDCmatrix.size() , 0.0 ) ;
          for (uint8_t rxLayerCtr = 0; rxLayerCtr< mmseWDCmatrix.at(0).size(); rxLayerCtr++ )
            {
              AntennaArrayBasicModel::BeamformingVector rxWaux = castRxArray->GetCurrentBeamformingVectorMultilayers ( rxLayerCtr );
              complexVector_t longTerm = GetLongTerm (a, b, channelMatrix, txW, rxWaux);
              complexVector_t bfComplexNewComponent = CalBeamformingComplexCoef (rxPsd, longTerm, channelMatrix, a->GetVelocity (), b->GetVelocity ());
              for ( size_t sBandCtr = 0; sBandCtr < mmseWDCmatrix.size(); sBandCtr++)
                {//if there are no bugs dimensions always match
                  bfComplexSpectrum.at( sBandCtr ) += mmseWDCmatrix.at( sBandCtr ).at( rxLayerInd ).at( rxLayerCtr ) * bfComplexNewComponent.at( sBandCtr );
                }
            }
          for ( size_t sBandCtr = 0; sBandCtr < mmseWDCmatrix.size(); sBandCtr++)
            {
              (*bfGainPsd)[sBandCtr] = std::norm( bfComplexSpectrum.at( sBandCtr ) );
            }
        }
    }
  else
    {
      //  NS_LOG_DEBUG ("In this calcPSDmultilayer call layerInd: "<<(int ) txLayerInd<<"->"<< (int ) rxLayerInd<< " tx vect size " << txW.first.size() << " rx vect size " << rxW.first.size());
      //  NS_LOG_DEBUG ("Tx MAC: "<< txDevice->GetAddress() <<" -> Rx MAC:"<< rxDevice->GetAddress() << " at distance " << a->GetDistanceFrom (b));
      // retrieve the long term component
      complexVector_t longTerm = GetLongTerm (a, b, channelMatrix, txW, rxW);
      //  for (uint16_t cIndex = 0; cIndex < txW.first.size(); cIndex++)
      //    {
      //      NS_LOG_DEBUG ("    txbfcoef " << (int) cIndex << " = " << txW.first.at(cIndex));
      //    }
      // apply the beamforming gain
      bfGainPsd = CalBeamformingGain (rxPsd, longTerm, channelMatrix, a->GetVelocity (), b->GetVelocity ());
    }
  (*rxPsd) *= (*bfGainPsd);
  NS_LOG_UNCOND("BF Gain TxId " << a->GetObject<Node>()->GetId () << " RxId " << b->GetObject<Node>()->GetId () << " TxBeam " << AntennaArrayBasicModel::GetBeamId(txW) << " RxBeam " << AntennaArrayBasicModel::GetBeamId(rxW) << " g= " << Sum(*bfGainPsd) / bfGainPsd->GetSpectrumModel()->GetNumBands() );
  return rxPsd;
}

complexVector_t
ThreeGppSpectrumPropagationLossModel::DoCalcRxComplexSpectrum (Ptr<SpectrumValue> refPsd,
                                                                    Ptr<const MobilityModel> a,
                                                                    Ptr<const MobilityModel> b,
                                                                    AntennaArrayBasicModel::BeamformingVector txW,
                                                                    AntennaArrayBasicModel::BeamformingVector rxW
                                                                ) const
{
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

    NS_ASSERT_MSG ( !(txAntennaArray->IsOmniTx () || rxAntennaArray->IsOmniTx () ), "The arrays must not be in omni mode in this function");

    NS_ASSERT_MSG (a->GetDistanceFrom (b) != 0, "The position of tx and rx devices cannot be the same");

    // retrieve the channel condition
    Ptr<ChannelCondition> condition = m_channelConditionModel->GetChannelCondition (a, b);

    // compute the channel matrix between a and b
    bool los = (condition->GetLosCondition () == ChannelCondition::LosConditionValue::LOS);
    bool o2i = false; // TODO include the o2i condition in the channel condition model
    Ptr<ThreeGppChannelMatrix> channelMatrix = m_channelModel->GetChannel (a, b, txAntennaArray, rxAntennaArray, los, o2i);

    // retrieve the long term component
    complexVector_t longTerm = GetLongTerm (a, b, channelMatrix, txW, rxW);

    return CalBeamformingComplexCoef (refPsd, longTerm, channelMatrix, a->GetVelocity (), b->GetVelocity ());
}


}  // namespace ns3
