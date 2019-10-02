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

#ifndef SRC_MMWAVE_BEAMFORMING_H_
#define SRC_MMWAVE_BEAMFORMING_H_

#include "ns3/object.h"
#include "ns3/antenna-array-basic-model.h"

namespace ns3 {

class MobilityModel;

namespace mmwave {


/**
 * This class handles the beamforming operations.
 * Extend this class to implement a specific beamforming algorithm.
 */
class MmWaveBeamforming : public Object
{
public:
  /**
   * Constructor
   */
  MmWaveBeamforming ();

  /**
   * Destructor
   */
  virtual ~MmWaveBeamforming ();

  /**
   * Returns the object type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * Returns the beamforming vector to be used to communicate with a target
   * device
   * \param the target device
   */
  virtual AntennaArrayBasicModel::complexVector_t SetBeamformingVectorForDevice (Ptr<NetDevice> device) = 0;

protected:
  Ptr<AntennaArrayBasicModel> m_antenna; // pointer to the antenna array instance
};


/**
 * This class extends the MmWaveBeamforming interface.
 * It implements a DFT-based beamforming algorithm.
 */
class MmWaveDftBeamforming : public MmWaveBeamforming
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
   * Returns the beamforming vector to be used to communicate with a target
   * device. The beamforming vector is computed using a DFT-based beamforming
   * algorithm.
   * \param the target device
   */
  AntennaArrayBasicModel::complexVector_t SetBeamformingVectorForDevice (Ptr<NetDevice> otherDevice) override;

private:
  Ptr<MobilityModel> m_mobility; // pointer to the MobilityModel installed in this device
};

} // namespace mmwave
} // namespace ns3

#endif /* SRC_MMWAVE_BEAMFORMING_H_ */
