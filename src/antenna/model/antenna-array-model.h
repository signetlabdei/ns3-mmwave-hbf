/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*   Copyright (C) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
*   University of Padova
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
*/

#ifndef ANTENNA_ARRAY_MODEL_H_
#define ANTENNA_ARRAY_MODEL_H_
#include <ns3/antenna-model.h>
#include <complex>
#include <ns3/net-device.h>
#include <map>
#include "antenna-array-basic-model.h"
#include <ns3/nstime.h>

namespace ns3 {

/**
 * Class implementing an antenna array with isotropic radiation pattern
 */
class AntennaArrayModel : public AntennaArrayBasicModel
{
public:

  /**
   * Predefined antenna orientation options
   */
  enum AntennaOrientation{
    X0 = 0,//!< X0 Means that antenna's X axis is set to 0, hence the antenna is placed in Z-Y plane
    Z0 = 1,//!< Z0 Means that antenna's Z axis is set to 0, hence the antenna is placed in X-Y plane
    Y0 = 2//!< Y0 Means that antenna's Y axis is set to 0, hence the antenna is placed in X-Z plane
  };

  /**
   * Constructor
   */
  AntennaArrayModel ();

  /**
   * Destructor
   */
  virtual ~AntennaArrayModel ();

  /**
   * Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Must override the function to set 0 gain,
   * since the gain is already accounted for with GetRadiationPattern function
   * \param angles angles of gain
   * \return gain which is in this case always 0
   */
  virtual double GetGainDb (Angles a) override;

  /**
   * This function sets the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device
   * using the beam that is specified by the beamId
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
  virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                     Ptr<NetDevice> device);

  /**
   * Change the beamforming vector for a device
   * \param device Device to change the beamforming vector for
   */
  virtual void ChangeBeamformingVector (Ptr<NetDevice> device);

  /**
   * Change the antenna model to omnidirectional (ignoring the beams)
   */
  virtual void ChangeToOmniTx ();

  /**
   * This function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetCurrentBeamformingVector ();

  /**
   * This function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetBeamformingVector (Ptr<NetDevice> device);

  /**
   * Returns the time at which was the last time updated the beamforming vector for the given device.
   * \param device for which is used the beamforming vector whose last update time is needed
   * \return the time at which the beamforming vector was being updated
   */
  virtual Time GetBeamformingVectorUpdateTime (Ptr<NetDevice> device);

  /**
   * Returns a bool that says if the current transmission is configured to be
   * omni.
   * \return whether the transmission is set to omni
   */
  virtual bool IsOmniTx ();

  /**
   * This function returns the radiation pattern for the specified vertical
   * and the horizontal angle.
   * \param vangle vertical angle
   * \param hangle horizontal angle
   * \return returns the radiation pattern
   */
  virtual double GetRadiationPattern (double vangle, double hangle = 0);

  /**
   * This function returns the location of the antenna element inside of the
   * sector assuming the left bottom corner is (0,0,0).
   * \param index index of the antenna element
   * \return returns the 3D vector that represents the position of the antenna
   * by specifing x, y and z coordinate
   */
  virtual Vector GetAntennaLocation (uint8_t index);

  /**
   * Set the orientation of the antenna
   * \param orientation Orientation
   */
  void SetAntennaOrientation (AntennaArrayModel::AntennaOrientation orientation);

  /**
   * Returns the orientation of the antenna
   * \return orientation
   */
  AntennaArrayModel::AntennaOrientation GetAntennaOrientation () const;

  /**
   * Returns the number of antenna elements in the first dimension
   * \return number of elements
   */
  virtual uint8_t GetAntennaNumDim1 () const override;

  /**
   * Returns the number of antenna elements in the second dimension
   * \return number of elements
   */
  virtual uint8_t GetAntennaNumDim2 () const override;

  /**
   * Set the number of antenna elements in the first dimension
   * \param antennaNum the number of antenna elements in the first dimension
   */
  virtual void SetAntennaNumDim1 (uint8_t antennaNum) override;
   /**
    * Set the number of antenna elements in the second dimension
    * \param antennaNum the number of antenna elements in the second dimension
    */
  virtual void SetAntennaNumDim2 (uint8_t antennaNum) override;

private:

  typedef std::map<Ptr<NetDevice>, BeamformingVector> BeamformingStorage; /*!< A type represents a map where the key is a pointer
                                                                               to the device and the value is the BeamformingVector element */
  typedef std::map<Ptr<NetDevice>, Time> BeamformingStorageUpdateTimes;  /*!< A type that represents a map in which are pairs of the pointer to the
                                                                               the device and the time at which is updated the beamforming vector for that device*/
  bool m_omniTx; //!< true if the antenna is omni, false otherwise
  BeamformingVector m_currentBeamformingVector; //!< the current beamforming vector
  BeamformingStorage m_beamformingVectorMap; //!< the map containing the beamforming vectors
  BeamformingStorageUpdateTimes m_beamformingVectorUpdateTimes;

protected:
  double m_disV; //!< antenna spacing in the vertical direction in terms of wave length.
  double m_disH; //!< antenna spacing in the horizontal direction in terms of wave length.
  AntennaOrientation m_orientation; //!< antenna orientation, for example, when set to "X0" (x=0) it means that the antenna will be in y-z plane

  uint8_t m_antennaNumDim1; //!< the number of antenna elements in the first dimension.
  uint8_t m_antennaNumDim2; //!< the number of antenna elements in the second dimension.
};

} /* namespace ns3 */

#endif /* SRC_ANTENNA_MODEL_ANTENNA_ARRAY_MODEL_H_ */
