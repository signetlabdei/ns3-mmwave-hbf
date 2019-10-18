/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 University of Padova, Dep. of Information Engineering, SIGNET lab.
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
*/

#ifndef SRC_MMWAVE_BEAMFORMING_MODEL_H_
#define SRC_MMWAVE_BEAMFORMING_MODEL_H_

#include "ns3/object.h"
#include "ns3/antenna-array-basic-model.h"
#include "ns3/mobility-model.h"
#include <map>

namespace ns3 {

class MobilityModel;
class AntennaArrayBasicModel;
class NetDevice;

namespace mmwave {


/**
 * This class handles the beamforming operations.
 * Extend this class to implement a specific beamforming algorithm.
 */
class MmWaveBeamformingModel : public Object
{
public:
  /**
   * Constructor
   */
  MmWaveBeamformingModel ();

  /**
   * Destructor
   */
  virtual ~MmWaveBeamformingModel ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * Returns the antenna object
   * \return the antenna object
   */
  Ptr<AntennaArrayBasicModel> GetAntenna (void) const;

  /**
   * Set the antenna array object.
   * \param antenna the antenna object
   */
  void SetAntenna (Ptr<AntennaArrayBasicModel> antenna);

  /**
   * Computes the beamforming vector to communicate with the target device
   * and sets the antenna.
   * \param the target device
   */
  virtual void SetBeamformingVectorForDevice (Ptr<NetDevice> device, uint8_t layerInd = 0) = 0;

protected:
  Ptr<AntennaArrayBasicModel> m_antenna; // pointer to the antenna array instance
};


struct BFVectorCacheEntry : public SimpleRefCount<BFVectorCacheEntry>
{
  Vector m_myPos; //the semantic here is my position and other device position instead of tx and rx because we assume reversible channels
  Vector m_otherPos;
  AntennaArrayBasicModel::BeamId m_beamId;
  AntennaArrayBasicModel::complexVector_t m_antennaWeights;
};

/**
 * This class extends the MmWaveBeamformingModel interface.
 * It implements a DFT-based beamforming algorithm.
 */
class MmWaveDftBeamforming : public MmWaveBeamformingModel
{
public:
  /**
   * Constructor
   */
  MmWaveDftBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveDftBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * Computes the beamforming vector to communicate with the target device
   * and sets the antenna.
   * The beamforming vector is computed using a DFT-based beamforming
   * algorithm.
   * \param the target device
   */
  void SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice, uint8_t layerInd = 0) override;

private:

  /**
    * Calculate the channel key using the Cantor function
    * \param x1 first value
    * \param x2 second value
    * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$
    */
   static constexpr uint32_t GetKey (uint32_t x1, uint32_t x2)
   {//TODO this is a replica of ThreeGppChannel id generation, it is not required to import modification to that function, but it should be considered
     return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
   }
   Ptr<MobilityModel> m_mobility; // pointer to the MobilityModel installed in this device
   std::map< uint32_t, Ptr<BFVectorCacheEntry> > m_vectorCache; // a memory to remember previous bf vectors and reuse them without recomputing
};

} // namespace mmwave
} // namespace ns3

#endif /* SRC_MMWAVE_BEAMFORMING_MODEL_H_ */
