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
 */

#ifndef THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H
#define THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H

#include "ns3/spectrum-propagation-loss-model.h"
#include <complex.h>
#include "ns3/angles.h"
#include "ns3/three-gpp-channel.h"
#include "ns3/antenna-array-basic-model.h"

namespace ns3 {

class NetDevice;
class ChannelConditionModel;
class ChannelCondition;

/**
 * Data structure that stores the long term component for a tx-rx pair
 */
struct LongTerm : public SimpleRefCount<LongTerm>
{
  complexVector_t m_longTerm; //!< vector containing the long term component for each cluster
  Ptr<ThreeGppChannelMatrix> m_channel; //!< pointer to the channel matrix used to compute the long term
  complexVector_t m_txW; //!< the tx beamforming vector used to compute the long term
  complexVector_t m_rxW; //!< the rx beamforming vector used to compute the long term
};

/**
 * \ingroup spectrum
 *
 */
class ThreeGppSpectrumPropagationLossModel : public SpectrumPropagationLossModel
{
public:
  /**
   * Constructor
   */
  ThreeGppSpectrumPropagationLossModel ();

  /**
   * Destructor
   */
  ~ThreeGppSpectrumPropagationLossModel ();

  /**
   * Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Add a device-antenna pair
   * \param a pointer to the NetDevice
   * \param a pointer to the associated AntennaArrayBasicModel
   */
  void AddDevice (Ptr<NetDevice>, Ptr<AntennaArrayBasicModel>);

  /**
   * Set the channel condition model
   * \param a pointer to the ChannelConditionModel object
   */
  void SetChannelConditionModel (Ptr<ChannelConditionModel> model);

  /**
   * Get the associated channel condition model
   * \return a pointer to the ChannelConditionModel object
   */
  Ptr<ChannelConditionModel> GetChannelConditionModel () const;

  /**
   * Set the operating frequency
   * \param the operating frequency in Hz
   */
  void SetFrequency (double frequency);

  /**
   * Get the operating frequency
   * \return the operating frequency in Hz
   */
  double GetFrequency () const;

  /**
   * Set the 3GPP propagation scenario
   * \param the scenario
   */
  void SetScenario (std::string scenario);

  /**
   * Get the 3GPP propagation scenario
   * \return the propagation scenario
   */
  std::string GetScenario () const;

  /**
   * Computes the received PSD
   * \param tx PSD
   * \param tx mobility model
   * \param rx mobility model
   * \return the received PSD
   */
  virtual Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                           Ptr<const MobilityModel> a,
                                                           Ptr<const MobilityModel> b) const;

  Ptr<SpectrumValue> CalcRxPowerSpectralDensityMultiLayers (Ptr<const SpectrumValue> txPsd,
                                                           Ptr<const MobilityModel> a,
                                                           Ptr<const MobilityModel> b,
		                                           uint8_t txLayerInd,
							   uint8_t rxLayerInd) const;
  /**
   * Computes the received PSD
   * \param tx PSD
   * \param tx mobility model
   * \param rx mobility model
   * \param hbf layer to be used by antenna arrays
   * \return the received PSD
   */
  //TODO why does the single layer version above use virtual? after finding the motivation, apply here if it applies
  Ptr<SpectrumValue> DoCalcRxPowerSpectralDensityMultilayers (Ptr<const SpectrumValue> txPsd,
                                                           Ptr<const MobilityModel> a,
                                                           Ptr<const MobilityModel> b,
		                                           uint8_t txLayerInd,
							   uint8_t rxLayerInd) const;

protected:
private:
  /**
   * Looks for the long term component in m_longTermMap. If found, checks
   * whether it has to be updated. If not found or if it has to be updated,
   * calls the method CalLongTerm to compute it.
   * \param aMob the mobility model of the first device
   * \param bMob the mobility model of the second device
   * \param channelMatrix the channel matrix
   * \param aW the beamforming vector of the first device
   * \param bW the beamforming vector of the second device
   * \return vector containing the long term compoenent for each cluster
   */
  complexVector_t GetLongTerm (Ptr<const MobilityModel> aMob, Ptr<const MobilityModel> bMob, Ptr<ThreeGppChannelMatrix> channelMatrix, AntennaArrayBasicModel::BeamformingVector aBF, AntennaArrayBasicModel::BeamformingVector bBF) const;
  /**
   * Computes the long term component
   * \param the channel matrix H
   * \param the tx beamforming vector
   * \param the rx beamforming vector
   * \return the long term component
   */
  complexVector_t CalLongTerm (Ptr<ThreeGppChannelMatrix> channelMatrix, complexVector_t txW, complexVector_t rxW) const;

  /**
   * Computes the beamforming gain and applies it to the tx PSD
   * \param the tx PSD
   * \param the long term component
   * \return the rx PSD
   */
  Ptr<SpectrumValue> CalBeamformingGain (Ptr<SpectrumValue> txPsd, complexVector_t longTerm, Ptr<ThreeGppChannelMatrix> params, Vector txSpeed, Vector rxSpeed) const;

  std::map < Ptr<NetDevice>, Ptr<AntennaArrayBasicModel> > m_deviceAntennaMap; //!< map containig the <device, antenna> associations
  mutable std::map < uint32_t, Ptr<LongTerm> > m_longTermMap; //!< map containing the long term components
  Ptr<ChannelConditionModel> m_channelConditionModel; //!< the channel condition model
  Ptr<ThreeGppChannel> m_channelModel; //!< the model to generate the channel matrix
};
} // namespace ns3

#endif /* THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H */
